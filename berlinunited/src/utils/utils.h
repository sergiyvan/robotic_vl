#ifndef UTILS_H_
#define UTILS_H_

#include <inttypes.h>
#include <sstream>
#include <string>
#include <vector>

#include <inttypes.h>

/*------------------------------------------------------------------------------------------------*/

bool getFilesInDir(std::string dir, std::vector<std::string> &files);
bool fileExists(const char* fileName);
bool fileExists(const std::string& fileName);
void hexDump(const uint8_t *data, uint32_t dataLength);
uint64_t filesize(const char* filename);
uint64_t filesize(const std::string& filename);

#endif /* UTILS_H_ */
