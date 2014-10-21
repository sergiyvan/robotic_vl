/*
 * Picaso4DSSerial.cpp
 *
 *  Created on: Jan 30, 2014
 *      Author: dseifert
 */

#include "display_picaso4DSSerial.h"
#include "platform/system/transport/transport_rs232.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

DisplayPicaso4DSSerial::DisplayPicaso4DSSerial()
	: transport(NULL)
	, sdCardInitialized(false)
	, fatInitialized(false)
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

DisplayPicaso4DSSerial::~DisplayPicaso4DSSerial() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool DisplayPicaso4DSSerial::open(const std::string &portDeviceName, uint32_t baudrate) {
	close();

	sdCardInitialized = false;
	fatInitialized = false;

	transport = new TransportSerial232(portDeviceName, baudrate);
	if (NULL == transport)
		return false;

	if (false == transport->open())
		return false;

	delay(2000*milliseconds);
	putString("Initializing ...\n");

	initializeFAT();
//
//	// initialize system
//	if (false == sendCommand(0xFF89, 0, 0, Microsecond(5*seconds))) {
//		ERROR("Display: SD card media initialization failure");
//	} else {
//		int32_t response = read16(Microsecond(2*seconds));
//
//		if (response == -1) {
//			ERROR("Display: SD card initialized failed (no response)");
//		} else {
//			sdCardInitialized = (response != 0);
//
//			if (sdCardInitialized) {
//				INFO("Display: SD card initialized.");
//			} else {
//				WARNING("Display: No SD card or not readable.");
//			}
//		}
//	}


	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void DisplayPicaso4DSSerial::close() {
	if (transport) {
		if (transport->isConnected())
			transport->close();

		delete transport;
	}

	transport = NULL;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool DisplayPicaso4DSSerial::sendCommand(uint16_t commandID, const void *data, uint16_t dataLength, Microsecond readTimeOut) {
//	INFO("Sending command %04x", commandID);

	drain();

	// send command
	commandID = htons(commandID);
	transport->write(&commandID, sizeof commandID);
	if (dataLength > 0) {
		ASSERT(data != NULL);
		transport->write(data, dataLength);
	}

	return getAck(readTimeOut);
}

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool DisplayPicaso4DSSerial::getAck(Microsecond timeout) {
	if (transport->waitForData(1, timeout)) {
		uint8_t ack;
		if (transport->read(&ack, 1) == 1) {
			if (0x06 == ack) {
//				INFO("Ack'ed");
			} else {
				WARNING("No ack, got %d", ack);
			}

			return 0x06 == ack;
		}
	}

	ERROR("no data");
	return false;
}
