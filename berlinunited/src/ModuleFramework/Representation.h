/**
 * @file Representation.h
 *
 * @author <a href="mailto:mellmann@informatik.hu-berlin.de">Heinrich Mellmann</a>
 * Declaration of class Representation
 */

#ifndef __Representation_h_
#define __Representation_h_

#include <string>
#include <iostream>
#include <list>

#include "Serializer.h"

class Module;

/**
 ** @ingroup ModuleFramework
 */

class Representation {
	friend class boost::serialization::access;

private:
	std::string name;

protected:
	// pointers to the providing and requiring modules
	std::list<const Module*> provided;
	std::list<const Module*> used;
	std::list<const Module*> required;
	std::list<const Module*> recycled;

	Representation(const std::string name)
		: name(name)
		, provided()
		, used()
		, required()
		, recycled()
	{
		//std::cout << "Load " << getModuleName() << endl;
	}

public:
	virtual ~Representation() {
	}

	const std::string& getName() const {
		return name;
	}

	void registerProvidingModule(const Module& module) {
		provided.push_back(&module);
	}

	void unregisterProvidingModule(const Module& module) {
		provided.remove(&module);
	}

	void registerRequiringModule(const Module& module) {
		required.push_back(&module);
	}

	void unregisterRequiringModule(const Module& module) {
		required.remove(&module);
	}

	void registerRecyclingModule(const Module& module) {
		recycled.push_back(&module);
	}

	void unregisterRecyclingModule(const Module& module) {
		recycled.remove(&module);
	}

	virtual bool serialize(SERIALIZER &archive) = 0;
	virtual bool deserialize(DESERIALIZER &archive) = 0;
};

#endif //__Representation_h_
