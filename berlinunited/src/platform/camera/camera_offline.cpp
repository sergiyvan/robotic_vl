/*
 * camera_simulator.cpp
 *
 */

#include "camera_offline.h"

#include "platform/image/image.h"
#include "msg_image.pb.h"
#include "debug.h"
#include "utils/utils.h"
#include "services.h"

#include "management/config/config.h"
#include "management/config/configRegistry.h"

#include "platform/image/camera_imageYUV422.h"

#include <png/image.hpp>

#include <algorithm>
#include <memory>

/*------------------------------------------------------------------------------------------------*/

REGISTER_CAMERA("offline", CameraOffline, "Offline camera, using image files or folders");


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

CameraOffline::CameraOffline()
	: cameraImageFileName("")
	, imageIdx(0)
	, lastImageCaptured(0)
{}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

CameraOffline::~CameraOffline() {
	if (image) {
		image->freeImageData();
		delete image;
		image = NULL;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

bool CameraOffline::openCamera(const char* deviceName, uint16_t requestedImageWidth, uint16_t requestedImageHeight, Hertz requestedFps) {
	if (requestedFps != 0*hertz)
		fps = requestedFps;
	else
		fps = 1*hertz;

	image = new IMAGETYPE(requestedImageWidth, requestedImageHeight);
	return image != NULL;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Reads image from given command line argument --camera.device
 **
 */

bool CameraOffline::capture() {
	// keep the requested interval
	robottime_t currentTime   = getCurrentTime();
	robottime_t nextImageTime =  lastImageCaptured + Millisecond(1./fps);
	if (nextImageTime > currentTime)
		delay(nextImageTime - currentTime);

	// get name of image file or folder
	std::string cameraImageFile = services.getConfig().get<std::string>("camera.device", "camera.png");

	std::string extension = "";
	std::vector<std::string> files;
	int dotPos = cameraImageFile.find_last_of(".");

	// if dot exists and is not the first character
	if (dotPos != (int)std::string::npos && dotPos != 0)
		extension = cameraImageFile.substr(dotPos + 1);

	// test if path is a directory
	if (extension == "") {
		bool hasFiles = getFilesInDir(cameraImageFile, files);
		if (!hasFiles) {
			ERROR("Empty folder");
			return false;
		}
		std::string file = "";
		uint invalidFiles = 0;

		// sort alphabetically
		std::sort(files.begin(), files.end());

		// find valid file name
		while (true) {
			if (invalidFiles == files.size()){
				ERROR("No valid files in folder");
				return false;
			}

			// ensure that imageIdx is in range
			if (imageIdx >= (int)files.size())
				imageIdx = 0;
			else if (imageIdx < 0)
				imageIdx = files.size()-1;

			file = files[imageIdx];

			// ignore other folders and files that start or end with a dot
			dotPos = file.find_last_of(".");
			if (dotPos == (int)std::string::npos
					|| dotPos == 0
					|| dotPos == (int)file.length()-1) {
				imageIdx++;
				invalidFiles++;
			}
			else break;
		}

		// store current image file name
		cameraImageFileName = file;

		// update extension
		extension = file.substr(dotPos + 1);

		// combine folder and file name
		if (cameraImageFile[cameraImageFile.length()-1] != '/')
			cameraImageFile += '/';
		cameraImageFile += file;
	} else {
		// store current image file name
		cameraImageFileName = cameraImageFile;
	}

	// try reading the image file
	if (extension == "pbi") {
		if (false == readImageFromPBI(cameraImageFile)) {
			ERROR("Failed to load PBI image %s", cameraImageFile.c_str());
			return false;
		}
	} else {
		if (false == readImage(cameraImageFile)) {
			ERROR("Failed to load PNG image %s", cameraImageFile.c_str());
			return false;
		}
	}

	INFO("Image %d loaded: %s", imageIdx, cameraImageFile.c_str());

	totalFrames++;
	lastImageCaptured = getCurrentTime();

	// proceed to next image
	imageIdx++;

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Reads an image from a protobuf file.
 **
 ** @param cameraImageFile the string name of the image file to read
 ** @return true if image could be read, false otherwise
 */

bool CameraOffline::readImageFromPBI(std::string cameraImageFile) {
	de::fumanoids::message::Image pbImage;

	std::ifstream file(cameraImageFile.c_str(), std::ios::in | std::ios::binary);
	if (file.fail())
		return false;

	pbImage.ParseFromIstream(&file);
	if (pbImage.IsInitialized() == false) {
		ERROR("Offline image not properly constructed");
		return false;
	}

	// sets the image
	bool foundRawData = false;
	for (int i=0; i < pbImage.imagedata_size(); i++) {
		const de::fumanoids::message::ImageData &pbImageData = pbImage.imagedata(i);
#if defined IMAGEFORMAT_YUV422
		if (pbImageData.format() == de::fumanoids::message::YUV422_IMAGE) {
#else
		if (pbImageData.format() == de::fumanoids::message::BAYER_IMAGE) {
#endif
			imageWidth  = pbImageData.width();
			imageHeight = pbImageData.height();

			// sets the pitch / roll and head angle data
			image->setImagePosition( pbImage.eyeheight(), pbImage.pitch(), pbImage.roll(), pbImage.yaw() );
			// FIXME
			// robot.getHead().setGyroManual(pbImage.pitch(), pbImage.roll());
			// robot.getHead().setAnglesManual(pbImage.yaw());

			std::shared_ptr<void> newData(malloc(pbImageData.data().size()), [](void *ptr) { free(ptr); });
			memcpy(newData.get(), pbImageData.data().c_str(), pbImageData.data().size());
			image->freeImageData();
			image->setImage(0, newData, pbImageData.data().size(), imageWidth, imageHeight);
			//printf("set new pbi image (%d bytes, %dx%d)\n", (int)pbImageData.data().size(), pbImageData.width(), pbImageData.height());
			foundRawData = true;
		}
	}

	if (foundRawData == false) {
#if defined IMAGEFORMAT_YUV422
		ERROR("The pbi file '%s' does not contain raw image data for YUV422, try another format/build or use a PNG image", cameraImageFile.c_str());
#else
		ERROR("The pbi file '%s' does not contain raw image data for Bayer, try another format/build or use a PNG image", cameraImageFile.c_str());
#endif
		return false;
	}
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Reads the image using libPNG
 **
 ** @param cameraImageFile the string name of the image file to read
 ** @return true if image could be read, false otherwise
 */

bool CameraOffline::readImage(std::string cameraImageFile) {
	png::image< png::rgb_pixel > png;

	try {
		png.read(cameraImageFile);
	} catch (...) {
		return false;
	}

	// TODO: check that file was loaded

	imageWidth  = png.get_width();
	imageHeight = png.get_height();

#if defined IMAGEFORMAT_YUV422
	// allocate an uint8_t array, initialize it to zero and provide a proper deleter
	std::shared_ptr<uint8_t> yuv422(new uint8_t[imageWidth*imageHeight*2](), std::default_delete<uint8_t[]>());
	if (!yuv422) {
		ERROR("Could not allocate memory for image conversion.");
		return false;
	}

	// convert image to yuv422
	for (int x=0; x < imageWidth; x += 2) {
		for (int y=0; y < imageHeight; y++) {
			uint8_t y1, y2, u1, u2, v1, v2;

			ColorConverter::rgb2yuv(png[y][x  ].red, png[y][x  ].green, png[y][x  ].blue, &y1, &u1, &v1);
			ColorConverter::rgb2yuv(png[y][x+1].red, png[y][x+1].green, png[y][x+1].blue, &y2, &u2, &v2);

			yuv422.get()[2*y*imageWidth + 2*x + 0] = y1;
			yuv422.get()[2*y*imageWidth + 2*x + 1] = (v1 + v2)/2;
			yuv422.get()[2*y*imageWidth + 2*x + 2] = y2;
			yuv422.get()[2*y*imageWidth + 2*x + 3] = (u1 + u2)/2;
		}
	}

	image->setImage(0, std::static_pointer_cast<void>(yuv422), imageWidth*imageHeight*2, imageWidth, imageHeight);
#else
	uint8_t *bayer = (uint8_t*)malloc(imageWidth*imageHeight);

	for (int x=0; x<imageWidth-1; x++) {
		for (int y=0; y<imageHeight-1; y++) {
			uchar* pixel = (bayer + y*imageWidth + x);

			png::rgb_pixel p1 = png[(y & ~1)  ][(x & ~1)  ];
			png::rgb_pixel p2 = png[(y & ~1)  ][(x & ~1)+1];
			png::rgb_pixel p3 = png[(y & ~1)+1][(x & ~1)  ];
			png::rgb_pixel p4 = png[(y & ~1)+1][(x & ~1)+1];

			// TODO: the green1 / green2 calculation is only approximate
			if (x & 1) {
				if (y & 1) { // x odd, y odd -> blue
					*pixel = (uint8_t)((p1.blue + p2.blue + p3.blue + p4.blue) / 4);
				} else { // x odd, y even -> green1
					*pixel = (uint8_t)((p1.green + p2.green + p3.green + p4.green) / 4);
				}
			} else {
				if (y & 1) { // x even, y odd -> green2
					*pixel = (uint8_t)((p1.green + p2.green + p3.green + p4.green) / 4);
				} else { // x even, y even -> red
					*pixel = (uint8_t)((p1.red + p2.red + p3.red + p4.red) / 4);
				}
			}
		}
	}

	image->freeImageData();
	image->setImage(0, bayer, imageWidth*imageHeight, imageWidth, imageHeight);
#endif

	// try to determine pitch and roll from the filename
	{
		// get current pitch/roll values
		int16_t pitch = image->getImagePositionPitch();
		int16_t roll  = image->getImagePositionRoll();

		// get the manually set values
		int preconfigured_pitch = services.getConfig().get<int>("Pitch", -999); // FIXME: strange config option name
		int preconfigured_roll  = services.getConfig().get<int>("Roll",  -999);

		// determine where in the filename string the pitch/roll values would be
		size_t pitchStrOffset = cameraImageFile.find("-pitch-");
		size_t rollStrOffset  = cameraImageFile.find("-roll-");

		// extract the pitch/roll values from the filename
		int image_pitch_parameter = -999;
		int image_roll_parameter  = -999;
		if (pitchStrOffset != std::string::npos)
			image_pitch_parameter = atoi(cameraImageFile.c_str() + pitchStrOffset + 7);
		if (rollStrOffset != std::string::npos)
			image_roll_parameter = atoi(cameraImageFile.c_str() + rollStrOffset + 6);

		// if the pitch or roll was not manually set, use the image pitch or roll
		// value (if any)
		if (preconfigured_pitch == -999 && image_pitch_parameter != -999)
			pitch = image_pitch_parameter;
		if (preconfigured_roll == -999 && image_roll_parameter != -999)
			roll  = image_roll_parameter;

		// TODO: determine head turn angle from filename
		image->setImagePosition(robotEyeHeight, pitch, roll, 0);
		// FIXME
		// robot.getHead().setGyroManual(pitch, roll);
	}

	return true;
}
