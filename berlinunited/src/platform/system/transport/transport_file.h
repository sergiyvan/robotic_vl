#ifndef __FILETRANSPORT_H__
#define __FILETRANSPORT_H__

#include "transport.h"

#include <string>


/*------------------------------------------------------------------------------------------------*/

/**
 ** The file transport implements saving to or reading from a file.
 **
 ** @ingroup transport
 **
 */

class TransportFile : public Transport {
protected:
	/// file descriptor
	int fd;

	/// filename
	std::string filename;

public:
	TransportFile(std::string _filename);
	virtual ~TransportFile();

	/// open file
	virtual bool open();
	virtual bool openReadOnly();

	/// returns whether file is open
	virtual bool isConnected();

	/// close file
	virtual void close();

	/// write
	virtual int write(const void *data, uint32_t count);

	/// read
	virtual int read(void *data, uint32_t count);

	/// wait for data
	virtual bool waitForData(uint32_t, Microsecond timeout);
};

#endif
