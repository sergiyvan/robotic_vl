/** @file
 **
 ** Base class for generic v4l2 camera support. Can be used
 ** standalone or refined/subclassed for individual cameras.
 */

#ifndef CAMERA_V4L2_H_
#define CAMERA_V4L2_H_

#include "camera.h"

#include <linux/videodev2.h>

#include <inttypes.h>

#include <fstream>
#include <vector>
#include <string>


/*------------------------------------------------------------------------------------------------*/

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define CLIP(color) (unsigned char)(((color)>0xFF)?0xff:(((color)<0)?0:(color)))


/*------------------------------------------------------------------------------------------------*/

struct Buffer {
	Buffer()
		: start()
		, length(0)
	{}

	std::shared_ptr<void> start;
	size_t length;
};


/*------------------------------------------------------------------------------------------------*/

class CameraV4L2 : public Camera {
public:
	CameraV4L2();
	virtual ~CameraV4L2();

	virtual std::string getCameraName() {
		return "v4l2 camera";
	}

	virtual bool openCamera(const char* deviceName, uint16_t requestedImageWidth, uint16_t requestedImageHeight, Hertz requestedFps=0*hertz) override;
	virtual void closeCamera();
	virtual bool isOpen();

	virtual bool capture();

	// set a camera setting
	virtual void setSetting(CAMERA_SETTING setting, int32_t value);

	// get a camera setting
//	virtual int32_t getSetting(CAMERA_SETTING setting);

protected:
	virtual bool openDevice(const char* deviceName);
	virtual void closeDevice();

	virtual bool initDevice(uint16_t requestedImageWidth, uint16_t requestedImageHeight, Hertz requestedFps);
	virtual void uninitDevice();

	virtual uint32_t getPixelFormat();
	virtual CameraImage* createImage();

	virtual bool init_mmap();
	virtual bool startCapturing();
	virtual void stopCapturing();

	int xioctl(unsigned long int request, void* arg);

	void determineControls();
	void determineControls(uint32_t minID, uint32_t maxID);

private:
	int fd;
	std::vector<Buffer> buffers;
	unsigned int n_buffers;

	struct v4l2_buffer dequeuedBuffer;
	bool bufferDequeued;
	__u32 lastSequenceNo;

	bool readFrame();
};

#endif /* CAMERA_V4L2_H_ */
