#include "camera_sensor.h"
#include "utils/utils.h"
#include "services.h"
#include "debug.h"

#include "platform/system/transport/transport_file.h"
#include "platform/image/image.h"
#include "platform/system/events.h"

#include "communication/comm.h"
#include "management/config/config.h"
#include "management/config/configRegistry.h"

#include "platform/image/image.h"

#include <png/image.hpp>

#include <msg_calibration.pb.h>

#include <algorithm>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <zlib.h>


/*------------------------------------------------------------------------------------------------*/

// The default camera device and type to use. If you want a different default,
// specify it via a #define (-D) to g++, or define it in the configuration file.

#if not defined DEFAULTCAMERADEVICE
	#ifdef __linux__
		#define DEFAULTCAMERADEVICE /dev/video0
	#else
		#define DEFAULTCAMERADEVICE
	#endif
#endif

#if not defined DEFAULTCAMERA
	#define DEFAULTCAMERA c910
#endif

#if not defined DEFAULTCAMERAWIDTH
	#define DEFAULTCAMERAWIDTH 640
#endif

#if not defined DEFAULTCAMERAHEIGHT
	#define DEFAULTCAMERAHEIGHT 480
#endif


/*------------------------------------------------------------------------------------------------*/

namespace {
	auto cfgDevice       = ConfigRegistry::registerOption<std::string>("camera.device",        STRINGIFY(DEFAULTCAMERADEVICE),  "Set the camera device to use. It also can point to a *.pbi file.");
	auto cfgType         = ConfigRegistry::registerOption<std::string>("camera.type",          STRINGIFY(DEFAULTCAMERA),       "Camera class to use: [generic, philips, simulator, offline, disabled, ...]. Take a look at the code to find out more.");

	auto cfgWidth        = ConfigRegistry::registerOption<uint32_t>   ("camera.width",         DEFAULTCAMERAWIDTH,        "Define the width of the image");
	auto cfgHeight       = ConfigRegistry::registerOption<uint32_t>   ("camera.height",        DEFAULTCAMERAHEIGHT,       "Define the height of the image");
	auto cfgFPS          = ConfigRegistry::registerOption<Hertz>      ("camera.fps",           0,                         "Frame rate (in hertz) of the image capture (0 == max speed)");

	auto cfgSave         = ConfigRegistry::registerOption<bool>       ("camera.save.enabled",  false,                     "Save images");
	auto cfgSavePath     = ConfigRegistry::registerOption<std::string>("camera.save.path",     ".",                       "Path (relative or absolute) for image storage");
	auto cfgSaveInterval = ConfigRegistry::registerOption<Millisecond>("camera.save.interval", 100*milliseconds,          "Interval in which to save images");

	auto cfgSaveFormat   = ConfigRegistry::registerOption<std::string>("camera.save.format",   "pbi",                     "Image format (three letter, e.g. pbi, png)");

	auto cfgCalibration  = ConfigRegistry::registerOption<std::string>("camera.calibration",   "config/calibration.dat",  "Camera calibration file (camera settings)");
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
**/

CameraSensor::CameraSensor()
	: cam(nullptr)
	, image(nullptr)
	, cameraType("")
{
	cs.setName("CameraSensor");
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Destructor
**/

CameraSensor::~CameraSensor() {
	services.getMessageRegistry().unregisterMessageCallback(this, "calibrationRequest");
	services.getMessageRegistry().unregisterMessageCallback(this, "cameraSettingsRequest");

	if (isRunning())
		cancel(true);

	if (cam)
		delete cam;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Initializes the camera.
 **
 ** @return true iff camera was initialized successfully
 */

bool CameraSensor::initCamera() {
	// if camera is already open, close it now
	if (cam != 0) {
		cam->closeCamera();
		delete cam;
		cam = 0;
	}

	cameraType = cfgType->get();
	std::transform(cameraType.begin(), cameraType.end(), cameraType.begin(), ::tolower);

	if (cameraType == "disabled") {
		INFO("Loading no camera");
		return true;
	}

	if (false == Factory<Camera>::has(cameraType)) {
		ERROR("Unknown camera %s", cameraType.c_str());
		return false;
	}

	INFO("Loading camera %s (%s)", cameraType.c_str(), Factory<Camera>::getDescription(cameraType).c_str());
	cam = Factory<Camera>::getNew(cameraType);

	std::string cameraDevice  = cfgDevice->get();
	int configuredResolutionX = cfgWidth->get();
	int configuredResolutionY = cfgHeight->get();
	Hertz configuredFps       = cfgFPS->get();

	std::transform(cameraType.begin(), cameraType.end(), cameraType.begin(), ::tolower);

	// open camera
	if (false == cam->openCamera(cameraDevice.c_str(), configuredResolutionX, configuredResolutionY, configuredFps))
		return false;

	// apply camera settings
	{
		CriticalSectionLock lock(cs);
		cam->configure(calibration.getProtobufCalibration().camerasettings());

		// skip the first images, as it takes some time for the images to
		// reflect the new camera settings
		int count = 3;
		while (count > 0) {
			if (cam->capture())
				count--;
		}
	}

	// retrieve image - because of performance issues, we need to know the exact
	// type (see comment in image.h for more details)
	image = cam->getImage();
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Initializes the camera and starts the sensor thread.
 **
 ** @return true iff thread was started (this implies initialization succeeded)
 */

bool CameraSensor::init() {
	// init calibration file
	calibration.load( cfgCalibration->get().c_str() );

	// register for calibration requests
	services.getMessageRegistry().registerMessageCallback(this, "calibrationRequest");
	services.getMessageRegistry().registerMessageCallback(this, "cameraSettingsRequest");

	// init camera
	initCamera();

	// start thread
	run();

	// we were successful
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Thread init.
 **
 */
void CameraSensor::threadInit() {
	// let's try to take a bit more CPU time
	setNiceness(-2);
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Thread main function.
 **
 */

void CameraSensor::threadMain() {
	if (cam == 0)
		return;

	threadInit();

	robottime_t lastFrameTime = 0;

	// loop until we quit
	while (isRunning()) {
		// make sure we are still connected to a camera
		if (false == cam->isOpen()) {
			WARNING("Camera not active, trying to re-initiate");
			if (false == initCamera()) {
				ERROR("No camera ...");
				delay(250*milliseconds);
				continue;
			}
		}

		// update frame
		cs.enter();
		bool imageCaptured = cam->capture();
		cs.leave();

		if (imageCaptured) {
			// notify any other objects that want to know about this new image for processing
			services.getEvents().trigger(EVT_IMAGE_CAPTURED, image);

			lastFrameTime = getCurrentTime();

			// save image if required
			handleImageSaving();
		} else if (lastFrameTime + 3000*milliseconds < getCurrentTime()) {
			ERROR("Did not receive image data for several seconds!");
			// TODO: trigger watchdog in competition mode
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Pass-through function for setting camera parameters.
 **
 ** @param settings  A reference to a CameraSettings protobuf message.
 */

void CameraSensor::setSettings(const de::fumanoids::message::CameraSettings &settings) {
	if (cam) {
		cam->configure(settings);
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Pass-through function for getting camera parameters.
 **
 ** @param settings  A reference to a CameraSettings protobuf message.
 */

void CameraSensor::getSettings(de::fumanoids::message::CameraSettings &settings) {
	if (cam)
		cam->getConfiguration(settings);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void CameraSensor::handleImageSaving() {
	if (! cfgSave->get() )
		return;

	static robottime_t lastSave = getCurrentTime();
	if (lastSave + cfgSaveInterval->get() > getCurrentTime())
		return;

	std::string fileExtension = cfgSaveFormat->get();
	lastSave = getCurrentTime();

	std::stringstream fileName;
	int count = 0;
	do {
		count++;
		fileName.seekp(0, std::ios::beg);
		fileName << cfgSavePath->get() << "/image-" << services.getName() << "-";

		time_t now = time(NULL);
		struct tm dt;
		localtime_r(&now, &dt);

		fileName << std::setfill('0')
		   << std::setw(4) << dt.tm_year + 1900
		   << std::setw(2) << dt.tm_mon + 1
		   << std::setw(2) << dt.tm_mday
		   << "-"
		   << std::setw(2) << dt.tm_hour
		   << "_"
		   << std::setw(2) << dt.tm_min
		   << "_"
		   << std::setw(2) << dt.tm_sec;

		fileName << "-" << std::setfill('0') << std::setw(2) << count;

		// add extension
		fileName << "." << fileExtension;
	}  while (fileExists(fileName.str().c_str()));

	saveImage(fileName.str(), fileExtension);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void CameraSensor::saveImage(std::string fileName, std::string fileExtension) {
	INFO("Saving image %s", fileName.c_str());

	if (fileExtension == "pbi") {
		TransportFile *transport = new TransportFile(fileName.c_str());
		if (transport->open()) {
			//sendImageData(transport, false, false);

			de::fumanoids::message::Image pbImage;
			de::fumanoids::message::ImageData *pbRawImage = pbImage.add_imagedata();
			image->getImageData(pbRawImage);

			std::string data;
			pbImage.SerializeToString(&data);
			uint32_t  dataSize       = data.size();

			transport->write((const uint8_t*)data.c_str(), dataSize);
			transport->close();
		}
		delete transport;
	} else {
		png::image< png::rgb_pixel > png(image->getImageWidth(), image->getImageHeight());
		for (size_t x = 0; x < png.get_width(); ++x) {
			for (size_t y = 0; y < png.get_height(); ++y) {
				uint8_t r, g, b;
				((IMAGETYPE*)image)->getPixelAsRGB(x, y, &r, &g, &b);
				png[y][x] = png::rgb_pixel(r, g, b);
			}
		}
		try {
			png.write(fileName);
		} catch (...) {

		}
	}
}

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool CameraSensor::messageCallback(
        const std::string               &messageName,
        const google::protobuf::Message &msg,
        int32_t                          senderID,
        RemoteConnectionPtr             &remote)
{
	if (messageName == "de.fumanoids.message.calibrationRequest") {
		CriticalSectionLock lock(cs);

		bool send = false;

		const de::fumanoids::message::CalibrationRequest &request =
				(const de::fumanoids::message::CalibrationRequest&)msg;

		de::fumanoids::message::Message response;
		de::fumanoids::message::CalibrationRequest *calibrationResponse =
			response.MutableExtension(de::fumanoids::message::calibrationRequest);

		if (request.setcalibration()) {
			pendingCalibrationRequest.CopyFrom(request);
			requestRemote = RemoteConnectionPtr( std::move(remote) );

			calibration.load(pendingCalibrationRequest.calibration());

			de::fumanoids::message::Message response;
			de::fumanoids::message::CalibrationRequest *calibrationResponse = response.MutableExtension(de::fumanoids::message::calibrationRequest);

			calibrationResponse->set_calibrationwasset(true);
			services.getComm().sendMessage(response, requestRemote.get());
		}

		if (request.getcalibration()) {
			calibrationResponse->mutable_calibration()->CopyFrom(calibration.getProtobufCalibration());
			send = true;
		}

		if (request.savecalibration()) {
			std::string calibrationFileName = cfgCalibration->get();
			bool success = calibration.save(calibrationFileName.c_str());
			calibrationResponse->set_calibrationwassaved(success);
			send = true;
		}

		if (send) {
			services.getComm().sendMessage(response, remote.get());
		}

	} else if (messageName == "de.fumanoids.message.cameraSettingsRequest") {
		CriticalSectionLock lock(cs);

		bool send = false;

		const de::fumanoids::message::CameraSettingsRequest &request =
				(const de::fumanoids::message::CameraSettingsRequest&)msg;

		de::fumanoids::message::Message response;
		de::fumanoids::message::CameraSettingsRequest *settingsResponse =
			response.MutableExtension(de::fumanoids::message::cameraSettingsRequest);

		if (request.setsettings()) {
			setSettings(request.settings());
			settingsResponse->set_settingswereset(true);
			send = true;
		}

		if (request.getsettings()) {
			getSettings(*settingsResponse->mutable_settings());
			send = true;
		}

		if (send)
			services.getComm().sendMessage(response, remote.get());
	}

	return false;
}
