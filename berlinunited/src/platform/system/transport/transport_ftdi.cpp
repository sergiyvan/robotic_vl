#include "transport.h"
#include "platform/system/timer.h"
#include "communication/comm.h"

#include <stdio.h>         // Standard input/output definitions
#include <string.h>        // String function definitions
#include <unistd.h>        // UNIX standard function definitions
#include <fcntl.h>         // File control definitions
#include <errno.h>         // Error number definitions
#include <inttypes.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <linux/types.h>
#include <linux/serial.h>

#ifdef USE_LIBFTDI

#include "platform/system/transport_ftdi.h"

TransportUSBFTDI::TransportUSBFTDI(const std::string port, int baudrate) {
	this->baudrate = baudrate;
	this->port     = port;
}



/*
 * Implementation of libftdi usage.
 *
 * For reference use of libftdi for Dynamixel control see:
 *  - http://github.com/damg/arnie-robot-controller/tree/serial_interface/src/serial_interface/connection.c
 *  - https://www.ni.uos.de/pub/HumanoideRoboter/wiki/doku.php?id=ftdiexample
 */


/**
 ** opens connection to FTDI chip via libftdi
 **
 ** @return FALSE if connection could not be established or initialized
 */

bool TransportUSBFTDI::open() {
	int ret;

	connected = false;

	if (ftdi_init(&ftdic) != 0) {
		ERROR("unable to init ftdi device: %s", ftdi_get_error_string(&ftdic));
		return false;
	}

	if ((ret = ftdi_usb_open(&ftdic, 0x0403, 0x6001)) < 0) {
		ERROR("unable to open ftdi device: %d (%s)", ret, ftdi_get_error_string(&ftdic));
		return false;
	}

	if (ftdi_usb_reset(&ftdic) != 0) {
		ERROR("unable to reset ftdi device: %s", ftdi_get_error_string(&ftdic));
		close();
		return false;
	}

	// set baudrate
	ret = ftdi_set_baudrate(&ftdic, baudrate);
	if (ret != 0) {
		ERROR("baudrate return value: %d (%s)", ret, ftdi_get_error_string(&ftdic));
		close();
		return false;
	}

	// set latency timer to smallest value
	ret = ftdi_set_latency_timer(&ftdic, 1);
	if (ret != 0) {
		ERROR("ftdi_set_latency_timer return value: %d (%s)", ret, ftdi_get_error_string(&ftdic));
		close();
		return false;
	}

	// minimize USB buffer size
	ftdi_write_data_set_chunksize(&ftdic, 8);

	// bitbang should be off by default, but let's make sure
	ftdi_disable_bitbang(&ftdic);

	// empty the buffers
	ftdi_usb_purge_buffers(&ftdic);

	// yay
	INFO("libftdi transport opened successfully with %d baud", baudrate);

	connected = true;
	return true;
}

/// return whether connection was established
bool TransportUSBFTDI::isConnected() {
	return connected;
}

/// close
void TransportUSBFTDI::close() {
	ftdi_usb_close(&ftdic);
	ftdi_deinit(&ftdic);
}

/// write
int TransportUSBFTDI::write(const uint8_t *data, uint32_t count) {
	int w = ftdi_write_data(&ftdic, const_cast<uint8_t*>(data), count);
	if (w < 0)
		ERROR("error writing");
	return w;
}

/// read
int TransportUSBFTDI::read(uint8_t *data, uint32_t count) {
	int r = ftdi_read_data(&ftdic, data, count);
	if (r < 0)
		ERROR("error reading: %s", ftdi_get_error_string(&ftdic));
	return r;
}

#endif // #ifdef USE_LIBFTDI

