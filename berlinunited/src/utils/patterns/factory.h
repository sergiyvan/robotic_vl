/**
 ** Generic Factory implementation with included registration capabilities.
 */
 
#ifndef __OBJECTREGISTRY_H__
#define __OBJECTREGISTRY_H__

#include "debug.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <map>
#include <string>
#include <functional>


/*------------------------------------------------------------------------------------------------*/

#define REGISTER_OBJECT(name, ObjectType, BaseType, description) \
	static bool registered_##ObjectType = Factory<BaseType>::registerObject(name, #ObjectType, []()->ObjectType*{ return new ObjectType; }, description)


/*------------------------------------------------------------------------------------------------*/

template <class T>
class Factory {
public:
	typedef std::function<T*()> CreatorFuncPtr;

	typedef struct ObjectInfo {
		ObjectInfo()
			: key()
			, className()
			, creatorFunction(nullptr)
			, description()
		{}

		ObjectInfo(const std::string &key, const std::string &className, CreatorFuncPtr creatorFunction, const std::string &description)
			: key(key)
			, className(className)
			, creatorFunction(creatorFunction)
			, description(description)
		{}

		std::string    key;
		std::string    className;

		CreatorFuncPtr creatorFunction;
		std::string    description;
	} ObjectInfo;

protected:
	// protected de- and constructor as we do not really want to instantiate the factory
	Factory();
	~Factory();

	typedef std::map<std::string, ObjectInfo> Registry;
	static Registry& getRegistry() {
		static Registry objectInfos;
		return objectInfos;
	}

public:

	/** Register a new class (derived from T)
	 **
	 ** @param key             Key identifying the class
	 ** @param className       Name of the class (for documentation purposes)
	 ** @param creatorFunction Function to create an instance of the class
	 ** @param description     Description of the class
	 **
	 ** @return
	 */
	static bool registerObject(
			const std::string &key,
			const std::string &className,
			CreatorFuncPtr creatorFunction,
			const std::string &description)
	{
		if (getRegistry().end() == getRegistry().find(key)) {
			ObjectInfo info = {
					/*.name            = */ key,
					/*.className       = */ className,
					/*.creatorFunction = */ creatorFunction,
					/*.description     = */ description
			};
			getRegistry()[key] = info;
			return true;
		} else {
			ASSERT(true);
			return false;
		}
	}

	/** Check whether an object/class identified by key is registered.
	 **
	 ** @param key  Key to check
	 **
	 ** @return true iff object/class is already registered
	 */
	static bool has(const std::string key) {
		return (getRegistry().end() != getRegistry().find(key));
	}

	/**
	 **
	 ** @param key
	 **
	 ** @return
	 */
	static T* getNew(const std::string key) {
		if (getRegistry().end() == getRegistry().find(key)) {
			return NULL;
		} else {
			return getRegistry()[key].creatorFunction();
		}
	}

	/**
	 **
	 ** @param key
	 **
	 ** @return
	 */
	static const std::string& getDescription(const std::string key) {
		if (getRegistry().end() == getRegistry().find(key)) {
			static std::string unknown = "(N/A)";
			return unknown;
		} else {
			return getRegistry()[key].description;
		}
	}

	/**
	 **
	 ** @param key
	 **
	 ** @return
	 */
	static std::vector<std::string> getAvailableObjectNames() {
		std::vector<std::string> ret;
		for (const auto& e : getRegistry()) {
			ret.push_back(e.first);
		}
		return ret;
	}

};

#endif
