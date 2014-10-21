#include "debug.h"
#include "tools/position.h"
#include "representations/imageDimensions.h"
#include "representations/camera/cameraMatrix.h"


/**
 * @brief Draw the Fiel of View of the robot for the given PositionRobot.
 *
 * @param debugName the name of the debug symbol
 * @param red red 0--255
 * @param green green 0--255
 * @param blue blue 0--255
 * @param robotPos a robotposition as origin for the drawing
 * @param camera The CameraMatrix is needed for some calculation
 * @param imageDimensions The ImageDimensions are needed for some calculation.
 * @param imageStream The ImageStream to draw on.
 */
inline void drawFieldOfView(const std::string& debugName,
                            uint8_t red,
                            uint8_t green,
                            uint8_t blue,
                            const PositionRobot& robotPos,
                            const CameraMatrix& camera,
                            const ImageDimensions& imageDimensions)
{
	DebuggingOption* _debug_option = ::Debugging::getInstance().getDebugOption(debugName);
	if (_debug_option->enabled) {
		ImageDebugger &imageDebugger = ::Debugging::getInstance().getImageDebugger();

		// some tmp variables
		int16_t horizon = camera.getHorizon();

		PositionAbsolute topLeft = camera.translateToRelative(
								  0, horizon).translateToAbsolute(robotPos);

		PositionAbsolute topRight = camera.translateToRelative(
								  imageDimensions.imageWidth , horizon).translateToAbsolute(robotPos);

		PositionAbsolute bottomLeft = camera.translateToRelative(
								  0, imageDimensions.imageHeight).translateToAbsolute(robotPos);

		PositionAbsolute bottomRight = camera.translateToRelative(
								  imageDimensions.imageWidth, imageDimensions.imageHeight).translateToAbsolute(robotPos);


		// draw the FOV
		imageDebugger.setColor(_debug_option, red, green, blue);
		imageDebugger.addTetragon(_debug_option,
		                        topLeft.getX(), topLeft.getY(),
		                        topRight.getX(), topRight.getY(),
		                        bottomRight.getX(), bottomRight.getY(),
		                        bottomLeft.getX(), bottomLeft.getY());
	}

}
