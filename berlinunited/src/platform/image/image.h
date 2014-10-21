#ifndef IMAGE_H_
#define IMAGE_H_

#include "platform/system/macroMagic.h"


// In order to avoid performance loss we do some macro tricks to
// be able to acess the image class directly even though it is one of the
// following:
//   CameraImageYUV422;
//   CameraImageBayer;
//   CameraImageRGB;
//   CameraImageRGB;
#ifndef IMAGETYPE
	#if defined IMAGEFORMAT_YUV422
		#define IMAGETYPE      CameraImageYUV422
		#define IMAGETYPEFULL  CameraImageYUV422 // TODO: should be YUV
		class CameraImageYUV422;
		#include "platform/image/camera_imageYUV422.h"
	#elif defined IMAGEFORMAT_BAYER
		#define IMAGETYPE     CameraImageBayer
		#define IMAGETYPEFULL CameraImageRGB
		class CameraImageBayer;
		class CameraImageRGB;
		#include "platform/image/camera_imageBayer.h"
		#include "platform/image/camera_imageRGB.h"
	#elif defined IMAGEFORMAT_RGB
		#define IMAGETYPE     CameraImageRGB
		#define IMAGETYPEFULL CameraImageRGB
		class CameraImageRGB;
		#include "platform/image/camera_imageRGB.h"
	#endif

	#define IMAGETYPENAME STRINGIFY(IMAGETYPE)
#endif


/*------------------------------------------------------------------------------------------------*/

// treat each CameraImage* as Image
typedef class IMAGETYPE Image;

#endif
