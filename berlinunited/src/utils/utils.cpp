#include <stdio.h>
#include <string.h>
#include <errno.h> // Error number definitions
#include <inttypes.h>
#include <dirent.h>

#include <string>
#include <vector>
#include <fstream>
#include <istream>

#include "debug.h"


/*------------------------------------------------------------------------------------------------*/
/**
 * Returns the file an directory names contained in directory dir.
 * '.' and '..' are excluded.
 * @param dir directory name
 * @param files file names in dir
 * @return true if directory contains at least one file, false otherwise
 */
bool getFilesInDir(std::string dir, std::vector<std::string> &files) {
	DIR *dp;
	struct dirent *dirp;

	if ((dp = opendir(dir.c_str())) == NULL) {
		ERROR("Error opening directory %s", dir.c_str());
		return false;
	}

	while ((dirp = readdir(dp)) != NULL) {
		std::string filename = std::string(dirp->d_name);
		if (filename != "." && filename != "..")
			files.push_back(filename);
	}

	closedir(dp);
	return files.empty() == false;
}


/*------------------------------------------------------------------------------------------------*/

/** Check whether a file already exists
 **
 ** @param fileName   File name to check
 **
 ** @return true iff file does not exist
 */

bool fileExists(const char* fileName) {
	FILE *file = fopen(fileName, "r");
	if (file == 0)
		return (errno != ENOENT);

	fclose(file);
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/** Check whether a file already exists
 **
 ** @param fileName   File name to check
 **
 ** @return true iff file does not exist
 */

bool fileExists(const std::string &fileName) {
	return fileExists(fileName.c_str());
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param filename   Filename to check
 ** @return file size in bytes.
 */

uint64_t filesize(const char* filename) {
	if (false == fileExists(filename))
		return 0;

	std::ifstream in(filename, std::ifstream::in | std::ifstream::binary | std::ifstream::ate);
	return in.tellg();
}



/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param filename   Filename to check
 ** @return file size in bytes.
 */

uint64_t filesize(const std::string& filename) {
	return filesize(filename.c_str());
};


/*------------------------------------------------------------------------------------------------*/

/** Dumps a range of memory as hex and ascii
 **
 **
 */

void hexDump(const uint8_t *data, uint32_t dataLength) {
	const int16_t bytesPerLine = 16;

	const uint64_t ptr = reinterpret_cast<uint64_t>(data);
	const uint8_t *startAddress = reinterpret_cast<uint8_t*>(( ptr / bytesPerLine) * bytesPerLine);
	const uint8_t *endAddress   = data + dataLength-1;
	while (startAddress < endAddress) {
		printf("%08lx  ", reinterpret_cast<long unsigned int>(startAddress) );

		// print hex
		for (int i = 0; i < bytesPerLine; i++) {
			if (startAddress + i < data || startAddress + i > endAddress)
				printf("   ");
			else
				printf("%02hhx ", startAddress[i]);
		}

		// print ascii
		printf(" |");
		for (int i = 0; i < bytesPerLine; i++) {
			if (startAddress + i < data || startAddress + i > endAddress)
				printf(" ");
			else {
				printf("%c", isprint(startAddress[i]) ? startAddress[i] : '.');
			}
		}

		printf("|\n");

		startAddress += bytesPerLine;
	}
}
