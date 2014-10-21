/*
 * Picaso4DSSerial.h
 *
 *  Created on: Jan 30, 2014
 *      Author: dseifert
 */

#ifndef PICASO4DSSERIAL_H_
#define PICASO4DSSERIAL_H_

#include "platform/system/transport/transport.h"

#include <arpa/inet.h>
#include "debug.h"

class DisplayPicaso4DSSerial {
public:
	DisplayPicaso4DSSerial();
	virtual ~DisplayPicaso4DSSerial();

	bool open(const std::string &portDeviceName, uint32_t baudrate);
	bool isOpen() const {
		return transport != NULL && transport->isConnected();
	}
	void close();


	typedef enum {
		SLIDER_INTENDED = 0,
		SLIDER_RAISED   = 1,
		SLIDER_HIDDEN   = 2
	} SLIDER_MODE;

	typedef enum {
		  BLACK = 0x0000
		, GREEN = 0x0004
		, NAVY  = 0x1000
		, RED   = 0x00F8
	} COLOR;

	/** Set the cursor to a line/column position, based on current font size.
	 **
	 ** @param line     Line (starts with 0)
	 ** @param row      Row/Column (starts with 0)
	 ** @return true if command was acknowledged as successful
	 */
	bool moveCursor(uint16_t line, uint16_t row) {
		uint16_t params[2] = { htons(line), htons(row) };
		return sendCommand(0xFFE9, params, sizeof params);
	}

	/** Prints a single character to the current cursor location.
	 **
	 ** @param character    Character to put on screen.
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool putCharacter(char character) {
		return sendCommand(0xFFFE, &character, 1);
	}

	/** Prints a string to the current cursor location.
	 **
	 ** @param string       String to output
	 **
	 ** @return number of bytes printed to screen (-1 on error)
	 */
	int16_t putString(const std::string &text) {
		ASSERT(text.size() < 512);
		if (sendCommand(0x0018, text.c_str(), text.size() + 1)) {
			return read16();
		} else {
			return -1;
		}
	}

	/** Retrieve the width of a character in pixel.
	 **
	 ** @param character    Character (ASCII) to calculate the width for
	 **
	 ** @return number of pixels this character is wide (-1 on error)
	 */
	int16_t getCharacterWidth(char character) {
		if (sendCommand(0x001E, &character, 1)) {
			return read16();
		} else
			return -1;
	}

	/** Retrieve the height of a character in pixel.
	 **
	 ** @param character    Character (ASCII) to calculate the height for
	 **
	 ** @return number of pixels this character is high (-1 on error)
	 */
	int16_t getCharacterHeight(char character) {
		if (sendCommand(0x001D, &character, 1)) {
			return read16();
		} else
			return -1;
	}

	/** Sets the text foreground color
	 **
	 ** @param color        the foreground color to set
	 **
	 ** @return previous text foreground color (-1 on error)
	 */
	int32_t setTextForeground(uint16_t color) {
		return send16AndGet16(0xFFE7, color);
	}

	/** Sets the text background color
	 **
	 ** @param color        the background color to set
	 **
	 ** @return previous text background color (-1 on error)
	 */
	int32_t setTextBackground(uint16_t color) {
		return send16AndGet16(0xFFE6, color);
	}

	/** Set font
	 **
	 ** @param font         the font to set
	 **
	 ** @return previous font (-1 on error)
	 */
	int32_t setTextFont(uint16_t fontID) {
		return send16AndGet16(0xFFE5, fontID);
	}

	/** Set text width multiplier
	 **
	 ** @param multiplier   multiplier
	 **
	 ** @return previous multiplier (-1 on error)
	 */
	int16_t setTextWidthMultiplier(uint8_t multiplier) {
		ASSERT(multiplier <= 16);
		return send16AndGet16(0xFFE4, multiplier);
	}

	/** Set text height multiplier
	 **
	 ** @param multiplier   multiplier
	 **
	 ** @return previous multiplier (-1 on error)
	 */
	int16_t setTextHeightMultiplier(uint8_t multiplier) {
		ASSERT(multiplier <= 16);
		return send16AndGet16(0xFFE3, multiplier);
	}

	/** Set text X gap
	 **
	 ** @param gap          the gap in pixel (X direction)
	 **
	 ** @return previous gap (-1 on error)
	 */
	int16_t setTextXGap(uint8_t gap) {
		ASSERT(gap <= 32);
		return send16AndGet16(0xFFE2, gap);
	}

	/** Set text Y gap
	 **
	 ** @param gap          the gap in pixel (Y direction)
	 **
	 ** @return previous gap (-1 on error)
	 */
	int16_t setTextYGap(uint8_t gap) {
		ASSERT(gap <= 32);
		return send16AndGet16(0xFFE1, gap);
	}

	/** Set text to be bold (or not)
	 **
	 ** @param on           whether to turn bold on (1) or off (0)
	 **
	 ** @return previous bold status (0 = off, 1 = on, -1 = error)
	 */
	int16_t setBold(bool on=true) {
		return send16AndGet16(0xFFDE, (on ? 1 : 0));
	}

	/** Set text to be inverse (or not)
	 **
	 ** @param inverse      whether to turn "inverse" on (1) or off (0)
	 **
	 ** @return previous "inverse" status (0 = off, 1 = on, -1 = error)
	 */
	int16_t setInverse(bool inverse=true) {
		return send16AndGet16(0xFFDC, (inverse ? 1 : 0));
	}

	/** Set text to be italic (or not)
	 **
	 ** @param italic       whether to turn italics on (1) or off (0)
	 **
	 ** @return previous italics status (0 = off, 1 = on, -1 = error)
	 */
	int16_t setItalic(bool italic=true) {
		return send16AndGet16(0xFFDD, (italic ? 1 : 0));
	}

	/** Set text to be opaque (or not)
	 **
	 ** @param opaque       whether to turn opacity on (1) or off (0, transparent)
	 **
	 ** @return previous opacitystatus (0 = off, 1 = on, -1 = error)
	 */
	int16_t setTextOpacity(bool opacity=true) {
		return send16AndGet16(0xFFDF, (opacity ? 1 : 0));
	}

	/** Set text to be underlined (or not). Note that Text Y Gap needs to be
	 ** set to at least 2 (via setTextYGap(2)) if underline is enabled.
	 **
	 ** @param underlined   whether to print text underlined (1) or not (0)
	 **
	 ** @return previous underline status (0 = off, 1 = on, -1 = error)
	 */
	int16_t setTextUnderlined(bool underlined=true) {
		return send16AndGet16(0xFFDB, (underlined ? 1 : 0));
	}

	/** Clear screen and reset settings.
	 **
	 ** @return true if command was acknowledged as successful
	 */

	bool clearAndResetScreen() {
		return sendCommand(0xFFCD, 0, 0);
	}

	/** Change all occurences of oldColor in the clipping region to newColor
	 **
	 ** @param oldColor     Color to be changed
	 ** @param newColor     Color to change to
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool changeColor(uint16_t oldColor, uint16_t newColor) {
		uint16_t params[2] = { htons(oldColor), htons(newColor) };
		return sendCommand(0xFFB4, params, sizeof params);
	}

	/** Draw circle
	 **
	 ** @param x            x coordinate of circle center
	 ** @param y            y coordinate of circle center
	 ** @param radius       radius of circle
	 ** @param color        color to use for drawing
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color) {
		uint16_t params[4] = { htons(x), htons(y), htons(radius), htons(color) };
		return sendCommand(0xFFC3, params, sizeof params);
	}

	/** Draw a filled circle
	 **
	 ** @param x            x coordinate of circle center
	 ** @param y            y coordinate of circle center
	 ** @param radius       radius of circle
	 ** @param color        color to use for drawing
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawCircleFilled(uint16_t x, uint16_t y, uint16_t radius, uint16_t color) {
		uint16_t params[4] = { htons(x), htons(y), htons(radius), htons(color) };
		return sendCommand(0xFFC2, params, sizeof params);
	}

	/** Draw a line
	 **
	 ** @param x1           x coordinate of start point
	 ** @param y1           y coordinate of start point
	 ** @param x2           x coordinate of end point
	 ** @param y2           y coordinate of end point
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
		uint16_t params[5] = { htons(x1), htons(y1), htons(x2), htons(y2), htons(color) };
		return sendCommand(0xFFC8, params, sizeof params);
	}

	/** Draw a rectangle
	 **
	 ** @param x1           x coordinate of top-left point
	 ** @param y1           y coordinate of top-left point
	 ** @param x2           x coordinate of bottom-right point
	 ** @param y2           y coordinate of bottom-right point
	 ** @param color        color to use for drawing
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
		uint16_t params[5] = { htons(x1), htons(y1), htons(x2), htons(y2), htons(color) };
		return sendCommand(0xFFC5, params, sizeof params);
	}

	/** Draw a filled rectangle
	 **
	 ** @param x1           x coordinate of top-left point
	 ** @param y1           y coordinate of top-left point
	 ** @param x2           x coordinate of bottom-right point
	 ** @param y2           y coordinate of bottom-right point
	 ** @param color        color to use for drawing
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawFilledRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
		uint16_t params[5] = { htons(x1), htons(y1), htons(x2), htons(y2), htons(color) };
		return sendCommand(0xFFC4, params, sizeof params);
	}

	/** Draw a polyline
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawPolyline(uint16_t verticesCount, uint16_t *verticesCoordinates, uint16_t color) {
		return false; // TODO: implement
	}

	/** Draw a polygon
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawPolygon(uint16_t verticesCount, uint16_t *verticesCoordinates, uint16_t color) {
		return false; // TODO: implement
	}

	/** Draw a filled polygon
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawFilledPolygon(uint16_t verticesCount, uint16_t *verticesCoordinates, uint16_t color) {
		return false; // TODO: implement
	}

	/** Draw a triangle
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color) {
		uint16_t params[7] = { htons(x1), htons(y1), htons(x2), htons(y2), htons(x3), htons(y3), htons(color) };
		return sendCommand(0xFFBF, params, sizeof params);
	}

	/** Draw a filled triangle
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color) {
		uint16_t params[7] = { htons(x1), htons(y1), htons(x2), htons(y2), htons(x3), htons(y3), htons(color) };
		return sendCommand(0xFFA9, params, sizeof params);
	}

	/** Calculate position of a polar point (distance, angle) from the current origin
	 **
	 ** @param angle        angle in degrees
	 ** @param distance     distance in pixel
	 ** @param xPos         reference to variable, will contain x position of point on successful return
	 ** @param yPos         reference to variable, will contain y position of point on successful return
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool getPosition(uint16_t angle, uint16_t distance, uint16_t &xPos, uint16_t &yPos) {
		uint16_t params[2] = { htons(angle), htons(distance) };
		if (sendCommand(0x0012, params, sizeof params)) {
			xPos = read16();
			yPos = read16();
		}

		return false;
	}

	/** Draw a pixel.
	 **
	 ** @param x            x coordinate
	 ** @param y            y coordinate
	 ** @param color        color
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawPixel(uint16_t x, uint16_t y, uint16_t color) {
		uint16_t params[3] = { htons(x), htons(y), htons(color) };
		return sendCommand(0xFFC1, params, sizeof params);
	}

	/** Retrieve color value of a pixel.
	 **
	 ** @param x            x coordinate
	 ** @param y            y coordinate
	 **
	 ** @return color value of pixel (-1 on error)
	 */
	int32_t getPixel(uint16_t x, uint16_t y) {
		uint16_t params[2] = { htons(x), htons(y) };
		if (sendCommand(0xFFC0, params, sizeof params)) {
			return read16();
		}
		return -1;
	}

	/** Set origin (can be used both for graphics and text)
	 **
	 ** @param x            x coordinate
	 ** @param y            y coordinate
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool setOrigin(uint16_t x, uint16_t y) {
		uint16_t params[2] = { htons(x), htons(y) };
		return sendCommand(0xFFCC, params, sizeof params);
	}

	/** Draw a line to (x, y) and set it as the new origin
	 **
	 ** @param x            x coordinate
	 ** @param y            y coordinate
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool drawLineAndSetOrigin(uint16_t x, uint16_t y) {
		uint16_t params[2] = { htons(x), htons(y) };
		return sendCommand(0xFFCA, params, sizeof params);
	}

	/** Enable or disable clipping.
	 **
	 ** @param clipping     enable clipping (1) or disable clipping (0)
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool enableClipping(bool clipping) {
		uint16_t value = (clipping ? 1 : 0);
		return sendCommand(0xFFA2, &value, 2);
	}

	/** Set clipping region.
	 **
	 ** @param x1           top-left corner of clipping area
	 ** @param y1           top-left corner of clipping area
	 ** @param x2           bottom-right corner of clipping area
	 ** @param y2           bottom-right corner of clipping area
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool setClippingWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
		uint16_t params[4] = { htons(x1), htons(y1), htons(x2), htons(y2) };
		return sendCommand(0xFFB5, params, sizeof params);
	}

	/** Extend clipping window to last printed text or last show image
	 **
	 ** @return true if command was acknowledged as successful
	 */
	bool extendClippingWindow(uint16_t ) {
		return sendCommand(0xFFB3, 0, 0);
	}

	bool drawEllipse(uint16_t x, uint16_t y, uint16_t xRadius, uint16_t yRadius, uint16_t color) {
		uint16_t params[5] = { htons(x), htons(y), htons(xRadius), htons(yRadius), htons(color) };
		return sendCommand(0xFFB2, params, sizeof params);
	}

	bool drawFilledEllipse(uint16_t x, uint16_t y, uint16_t xRadius, uint16_t yRadius, uint16_t color) {
		uint16_t params[5] = { htons(x), htons(y), htons(xRadius), htons(yRadius), htons(color) };
		return sendCommand(0xFFB1, params, sizeof params);
	}

	bool drawButton(const std::string &text, bool buttonPressed, uint16_t x, uint16_t y, COLOR buttonColor, COLOR textColor, uint16_t font, uint16_t textWidthMultiplier=1, uint16_t textHeightMultiplier=1) {
		uint16_t buttonState = buttonPressed ? 1 : 0;
		uint16_t params[8] = { htons(buttonState), htons(x), htons(y), htons(buttonColor), htons(textColor), htons(font), htons(textWidthMultiplier), htons(textHeightMultiplier) };

		uint16_t commandID = htons(0x0011);
		transport->write(&commandID, sizeof commandID);
		transport->write(params, sizeof params);
		transport->write(text.c_str(), text.size() + 1);

		return getAck();
	}

	bool drawPanel(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
		uint16_t params[5] = { htons(x), htons(y), htons(width), htons(height), htons(color) };
		return sendCommand(0xFFAF, params, sizeof params);
	}

	bool drawSlider(SLIDER_MODE mode, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint16_t scale, uint16_t value) {
		uint16_t params[8] = { htons(mode), htons(x1), htons(y1), htons(x2), htons(y2), htons(color), htons(scale), htons(value) };
		return sendCommand(0xFFAE, params, sizeof params);
	}

	// TODO: add commands from page 57 to 69

	bool soundPlayWAV(const std::string &filename) {
		if (false == fatInitialized) {
			if (false == initializeFAT()) {
				return false;
			}
		}

		if (false == sendCommand(0x000B, filename.c_str(), filename.size() + 1, Microsecond(1*seconds)))
			return false;

		if (transport->waitForData(2, Microsecond(250*milliseconds))) {
			int32_t status = read16();
			switch (status) {
			case -1:
				ERROR("Insufficient read");
			case 6:
				ERROR("Display: Can not play this rate");
				return false;
			case 5:
				ERROR("No data chunk found in first sector");
				return false;
			case 4:
				ERROR("No format data");
				return false;
			case 3:
				ERROR("No wave chunk signature");
				return false;
			case 2:
				ERROR("bad wave file format");
				return false;
			case 1:
				ERROR("file not found");
				return false;

			default:
				return true;
			}
		}

		return false;
	}

	bool setSoundVolume(uint16_t soundLevel) {
		ASSERT(soundLevel < 128);
		uint16_t params[1] = { htons(soundLevel) };
		return sendCommand(0xFF00, params, sizeof params);
	}

	int32_t setSoundPitch(uint16_t soundPitch) {
		ASSERT(soundPitch == 0 || soundPitch >= 4000);
		return send16AndGet16(0xFEFF, soundPitch);
	}

	bool setSoundBufferSize(uint16_t bufferSize) {
		ASSERT(bufferSize <= 2);
		uint16_t params[1] = { htons(bufferSize) };
		return sendCommand(0xFEFE, params, sizeof params);
	}

	bool soundStop() {
		return sendCommand(0xFEFD, 0, 0);
	}

	bool soundPause() {
		return sendCommand(0xFEFC, 0, 0);
	}

	bool soundContinue() {
		return sendCommand(0xFEFB, 0, 0);
	}

	int32_t soundRemainingBuffers() {
		// TODO
		return 0;
	}

	int32_t getSPEVersion() {
		if (false == sendCommand(0x001B, 0, 0))
			return -1;

		return read16();
	}

	int32_t getPmmcVersion() {
		if (false == sendCommand(0x001C, 0, 0))
			return -1;

		return read16();
	}

	std::string getDisplayModel() {
		if (false == sendCommand(0x001A, 0, 0))
			return "n/a";

		return readString();
	}

	std::string readString() {
		// read number of characters to expect
		int32_t count = read16();
		if (count <= 0)
			return "";

		char *str = new char[count+1];
		int readBytes = 0;
		while (readBytes < count) {
			if (false == transport->waitForData(1, 50000*microseconds))
				break;

			int16_t status = transport->read(str + readBytes, count - readBytes);
			if (status < 0)
				break;

			readBytes += status;
		}

		std::string output = str;
		delete [] str;
		return output;
	}

	bool initializeFAT() {
		if (false == sendCommand(0xFF03, 0, 0, Microsecond(5*seconds))) {
			ERROR("initialize fat, no ack received");
			return false;
		}

		int32_t status = read16();
		if (status == -1) {
			ERROR("FAT initialization failed, did not get status");
			return false;
		} else if (status == 0) {
			ERROR("FAT initialization failed, will not be able to open files.");
			return false;
		} else {
			fatInitialized = true;
			INFO("FAT file system successfully mounted.");
			return true;
		}
	}

protected:
	Transport *transport;
	bool       sdCardInitialized;
	bool       fatInitialized;

	bool sendCommand(uint16_t commandID, const void *data, uint16_t dataLength, Microsecond readTimeOut=500000*microseconds);
	bool getAck(Microsecond timeout=500000*microseconds);

	int32_t send16AndGet16(uint16_t command, uint16_t valueToSend) {
		uint16_t params[1] = { htons(valueToSend) };
		if (sendCommand(command, params, sizeof params))
			return read16();
		else
			return -1;
	}

	/** Read a 16 bit value.
	 **
	 ** @return 16 bit value (-1 on error)
	 */
	int32_t read16(Microsecond timeout=50000*microseconds) {
		uint16_t value;

		int readBytes = 0;
		while (readBytes < 2) {
			if (false == transport->waitForData(1, timeout)) {
				ERROR("timeout waiting for data");
				return -1;
			}

			int16_t status = transport->read((uint8_t*)&value + readBytes, 2 - readBytes);
			if (status < 0) {
				ERROR("Error reading from serial");
				return -1;
			}
			readBytes += status;
		}

		return ntohs(value);
	}

	void drain(Microsecond timeout=10*microseconds) {
		if (NULL == transport || false == transport->isConnected())
			return;

		uint16_t drainCount = 0;
		while (transport->waitForData(1, timeout)) {
			uint8_t dummy;
			transport->read(&dummy, 1);
			drainCount++;
		}

		if (drainCount > 0) {
			INFO("drained %d bytes", drainCount);
		}
	}
};

//typedef struct {
//	uint16_t command;
//	uint16_t minParameterBytes;
//	uint16_t maxParameterBytes;
//} Picaso4DCommand;
//
//Picaso4DCommand picaso4DCommands[] = {
//	PICASO_CMD(MOVE_CURSOR, 0xFFE9, 0, 0)
//};
//
//#define PICASO_CMD(name, code, minBytes, maxBytes)
//
//PICASO_CMD(MOVE_CURSOR, 0xFFE9, 0, 0)
//
//#define PICASO4D_CMD_MOVE_CURSOR  0xFFE9
#define PICASO4D_CMD_MOVE_CURSOR

#define PICASO4D_CMD_CLEAR_SCREEN                  0xFFCD
#define PICASO4D_CMD_PUT_TEXT                      0x0018

#endif /* PICASO4DSSERIAL_H_ */
