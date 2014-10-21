#ifndef __CONFIGEDITOR_H_
#define __CONFIGEDITOR_H_

// forward declaration
class Config;

#include <string>

/**
 ** Terminal-based configuration editor
 **
 ** @ingroup config
 */
class ConfigEditor {
protected:
	virtual void printUsage() const;
	virtual void printUsageShort() const;
	virtual void printConfiguration(const std::string &section) const;

	virtual std::string getSubsectionPath(const std::string &parentSection, const std::string &subsection) const;
	virtual std::string parentSection(const std::string &section) const;
	virtual std::string queryUserSelection(const std::string &currentSection) const;
	virtual bool isValidSubSection(std::string currentSection, std::string potentialSubSection) const;

	Config& config;

	// we are using pointers, it does not make sense to copy this handler so prevent it
	ConfigEditor(const ConfigEditor &) = delete;
	ConfigEditor& operator=(const ConfigEditor &) = delete;

public:
	ConfigEditor(Config &config);
	virtual ~ConfigEditor();

	virtual void start();
};

#endif
