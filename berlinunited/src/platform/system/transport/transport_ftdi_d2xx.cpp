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


/*------------------------------------------------------------------------------------------------*/

#ifdef USE_D2XX

/*
 * Implementation of D2XX usage.
 *
 *
 */


/**
 ** opens connection to FTDI chip via closed-source D2XX driver
 **
 ** @return FALSE if connection could not be established or initialized
 */

bool MotorBusTransportFTDID2XX::open(int baudrate) {
	FT_STATUS ftStatus = FT_Open(0, &ftHandle);
	if (ftStatus != FT_OK) {
		ERROR("Error FT_Open(%ld)", ftStatus);
		return false;
	}

	if ((ftStatus = FT_SetBaudRate(ftHandle, baudrate)) != FT_OK) {
		ERROR("Error FT_SetBaudRate(%ld)", ftStatus);
		return false;
	}

	if ((ftStatus = FT_SetLatencyTimer(ftHandle, 0)) != FT_OK) {
		ERROR("Error FT_SetLatencyTimer(%ld)", ftStatus);
	}

	FT_Purge(ftHandle, FT_PURGE_RX);
	return true;
}

bool MotorBusTransportFTDID2XX::isConnected() {
	return (ftHandle != 0);
}

void MotorBusTransportFTDID2XX::close() {
	FT_Close(ftHandle);
	ftHandle = 0;
}

/// write
int MotorBusTransportFTDID2XX::write(const uint8_t *data, uint8_t count) {
	DWORD dwBytesWritten = 0;
	FT_STATUS ftStatus = FT_Write(ftHandle, (uint8_t*)data, count, &dwBytesWritten);
	if (ftStatus != FT_OK)
		ERROR("Error FT_Write(%ld)", ftStatus);

	return dwBytesWritten;
}

/// read
int MotorBusTransportFTDID2XX::read(uint8_t *data, uint8_t count) {
	DWORD dwBytesRead = 0;
	DWORD dwNumToRead = 0;

	FT_STATUS ft_status = FT_GetQueueStatus(ftHandle, &dwNumToRead );
	if( ft_status != FT_OK )
		return 0;

	if( dwNumToRead > 0) {
		if (dwNumToRead > count)
			dwNumToRead = count;

		ft_status = FT_Read( ftHandle, data, dwNumToRead, &dwBytesRead);
		if( ft_status == FT_IO_ERROR )
			return 0;
	}

	return dwBytesRead;
}

#endif
