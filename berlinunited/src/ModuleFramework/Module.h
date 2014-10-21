/**
 * @file Module.h
 *
 * @author <a href="mailto:mellmann@informatik.hu-berlin.de">Heinrich Mellmann</a>
 * Declaration of class Module (base class for modules)
 */

#ifndef __Module_h_
#define __Module_h_

#include "BlackBoardInterface.h"

#include "DataHolder.h"

#include "debug.h"
#include "platform/system/macroMagic.h"

#include <string.h>
#include <map>
#include <list>

#include <stdio.h>
class Representation;



/*------------------------------------------------------------------------------------------------*/

#define MODULETEMPVAR(lineno,name) CONCATENATE(ModuleTemp##name, lineno)
#define REGISTER_MODULE(manager, module, enabled, description) \
	class module; \
	namespace { \
		auto cfgModule##module##Enabled = ConfigRegistry::registerOption<bool>("modules."#manager"."#module, enabled, description); \
		auto MODULETEMPVAR(__LINE__, module) = ModuleManagers::getInstance().get<manager>()->registerModule<module>(std::string(#module)); \
		auto MODULETEMPVAR(__LINE__, debug)  = \
			::Debugging::getInstance().registerDebugOption(module::getModuleDebugSymbol(), TEXT, NONET | CMDLOUT, __FILE__, "Text debug output (console only) for module " # module); \
	}


/*------------------------------------------------------------------------------------------------*/

/**
 *
 */

class RegistrationInterface {
public:
	virtual ~RegistrationInterface() {}

	virtual Representation& registerAtBlackBoard(BlackBoard& blackBoard) = 0;
};

/** type for a named map of representations */
typedef std::map<std::string, RegistrationInterface*> RepresentationMap;

/**
 *
 */
template<class T>
class TypedRegistrationInterface: public RegistrationInterface {
private:
	const std::string getName() const {
		return name;
	}
	std::string name;

// HACK: should not be public, but inline access from StaticProvidingRegistrator
// needs
public:
	//HACK: remove it, here schouldn't be any object data
	T* data;

public:

	TypedRegistrationInterface(const std::string& name)
		: name(name), data(NULL)
	{}

	inline T& getData() const {
		assert(data != NULL);
		return *data;
	}

	virtual Representation& registerAtBlackBoard(BlackBoard& blackBoard) {
		DataHolder<T>& rep = get(blackBoard);
		data = &(*rep);
		return rep;
	}

	virtual DataHolder<T>& get(BlackBoard& blackBoard) {
		return blackBoard.template getRepresentation<DataHolder<T> >(getName());
	}

	virtual const DataHolder<T>& get(const BlackBoard& blackBoard) const {
		return blackBoard.template getRepresentation<DataHolder<T> >(getName());
	}
};

/**
 *
 */
class Module: virtual public BlackBoardInterface {
private:
	std::string moduleName;

protected:

	// pointers to the provided and required representations
	std::list<Representation*> provided;
	std::list<Representation*> required;
	std::list<Representation*> recycled;

	Module(std::string name)
		: moduleName(name)
		, provided()
		, required()
		, recycled()
	{}

	Module()
		: moduleName("invalid module")
		, provided()
		, required()
		, recycled()
	{
		assert(false);
		// should never be here
	}

	void registerProviding(const RepresentationMap& list) {
		RepresentationMap::const_iterator iter = list.begin();
		for (; iter != list.end(); iter++) {
			// init the actual dependency to the black board
			Representation& representation = (*iter).second->registerAtBlackBoard(getBlackBoard());
			provided.push_back(&representation);
			representation.registerProvidingModule(*this);
		}
	}

	void registerRequiring(const RepresentationMap& list) {
		RepresentationMap::const_iterator iter = list.begin();
		for (; iter != list.end(); iter++) {
			// init the actual dependency to the black board
			Representation& representation = (*iter).second->registerAtBlackBoard(getBlackBoard());
			required.push_back(&representation);
			representation.registerRequiringModule(*this);
		}
	}

	void registerRecycling(const RepresentationMap& list) {
		RepresentationMap::const_iterator iter = list.begin();
		for (; iter != list.end(); iter++) {
			// init the actual dependency to the black board
			Representation& representation = (*iter).second->registerAtBlackBoard(getBlackBoard());
			recycled.push_back(&representation);
			representation.registerRecyclingModule(*this);
		}
	}

	void unregisterProviding(const RepresentationMap& list) {
		RepresentationMap::const_iterator iter = list.begin();
		for (; iter != list.end(); iter++) {
			// init the actual dependency to te black board
			Representation& representation = (*iter).second->registerAtBlackBoard(getBlackBoard());
			provided.remove(&representation);
			representation.unregisterProvidingModule(*this);
		}
	}

	void unregisterRequiring(const RepresentationMap& list) {
		RepresentationMap::const_iterator iter = list.begin();
		for (; iter != list.end(); iter++) {
			// init the actual dependency to te black board
			Representation& representation = (*iter).second->registerAtBlackBoard(getBlackBoard());

			required.remove(&representation);
			representation.unregisterRequiringModule(*this);
		}
	}

	void unregisterRecycling(const RepresentationMap& list) {
		RepresentationMap::const_iterator iter = list.begin();
		for (; iter != list.end(); iter++) {
			// init the actual dependency to te black board
			Representation& representation = (*iter).second->registerAtBlackBoard(getBlackBoard());

			recycled.remove(&representation);
			representation.unregisterRecyclingModule(*this);
		}
	}

public:

	const std::list<Representation*>& getRequiredRepresentations() {
		return required;
	}

	const std::list<Representation*>& getRecycledRepresentations() {
		return recycled;
	}

	const std::list<Representation*>& getProvidedRepresentations() {
		return provided;
	}

	/** initializes the module */
	virtual void init() = 0;

	/** executes the module */
	virtual void execute() = 0;

	virtual ~Module() {}

	const std::string& getModuleName() const {
		return (*this).moduleName;
	}

	// what is it used for?
	Module* operator->() {
		return this;
	}
};

template<class T> RepresentationMap* getProvidingRegistry() {
	static RepresentationMap* rep = new RepresentationMap();
	return rep;
}
template<class T> RepresentationMap* getRequiringRegistry() {
	static RepresentationMap* rep = new RepresentationMap();
	return rep;
}
template<class T> RepresentationMap* getRecyclingRegistry() {
	static RepresentationMap* rep = new RepresentationMap();
	return rep;
}

/**
 *
 */
template<class T>
class StaticRegistry {
protected:
	template<class TYPE_WHAT>
	class StaticRequiringRegistrator {
	private:
		TypedRegistrationInterface<TYPE_WHAT>* data;

	public:
		StaticRequiringRegistrator()
			: data(NULL)
		{
		}

		StaticRequiringRegistrator(const std::string& name) :
				data(NULL) {
			// TODO: check the type
			if (getRequiringRegistry<T>()->find(name) == getRequiringRegistry<T>()->end()) {
				(*getRequiringRegistry<T>())[name] = new TypedRegistrationInterface<TYPE_WHAT>(name);
			}
			data = dynamic_cast<TypedRegistrationInterface<TYPE_WHAT>*> ((*getRequiringRegistry<T>())[name]);
		}

		const TYPE_WHAT& get(const BlackBoard& blackBoard) const {
			return *(data->get(blackBoard));
		}
	};

	template<class TYPE_WHAT>
	class StaticRecyclingRegistrator {
	private:
		TypedRegistrationInterface<TYPE_WHAT>* data;

	public:
		StaticRecyclingRegistrator() {
		}

		StaticRecyclingRegistrator(const std::string& name) {
			// TODO: check the type
			if (getRecyclingRegistry<T>()->find(name) == getRecyclingRegistry<T>()->end()) {
				(*getRecyclingRegistry<T>())[name] = new TypedRegistrationInterface<TYPE_WHAT>(name);
			}
			data = dynamic_cast<TypedRegistrationInterface<TYPE_WHAT>*> ((*getRecyclingRegistry<T>())[name]);
		}

		const TYPE_WHAT& get(const BlackBoard& blackBoard) const {
			return *(data->get(blackBoard));
		}
	};

	template<class TYPE_WHAT>
	class StaticProvidingRegistrator {
	private:
		TypedRegistrationInterface<TYPE_WHAT>* data;

	public:
		StaticProvidingRegistrator()
			: data(NULL)
		{}

		StaticProvidingRegistrator(const std::string& name)
			: data(NULL)
		{
			// TODO: check the type
			if (getProvidingRegistry<T>()->find(name) == getProvidingRegistry<T>()->end()) {
				(*getProvidingRegistry<T>())[name] = new TypedRegistrationInterface<TYPE_WHAT>(name);
			}
			data = dynamic_cast<TypedRegistrationInterface<TYPE_WHAT>*> ((*getProvidingRegistry<T>())[name]);
		}

		inline TYPE_WHAT& get(BlackBoard& blackBoard) const {
			return *(data->get(blackBoard));
		}

		inline TYPE_WHAT& getData() const {
			assert(data->data != NULL);
			return *(data->data);
		}
	};
};


/***************************************************************
 macros for creating a module
 usage:

 #include "RepresentationA.h"
 #include "RepresentationB.h"

 BEGIN_DECLARE_MODULE(ModuleA)
 REQUIRE(RepresentationA)
 PROVIDE(RepresentationB)
 END_DECLARE_MODULE(ModuleA)


 class ModuleA: private ModuleABase
 {

 public:

 // a default constructor is required,
 // it is used to create an instance of the module
 ModuleA()
 {
 }

 // the execute method is called to run the module
 // put your functionality here
 virtual void execute()
 {
 // do something with RepresentationA and RepresentationB:
 // int x = getRepresentationA().x;
 // getRepresentationB().y = x + 1;
 }

 };//end class ModuleA

 ****************************************************************/

#define BEGIN_DECLARE_MODULE(moduleName) \
	class moduleName##Base \
		: protected Module \
		, private StaticRegistry<moduleName##Base> \
	{

// static invoker (registers the static dependency to representationName)
#define REQUIRE(representationName) \
	private: \
		class representationName##StaticRequiringRegistrator : public StaticRequiringRegistrator<representationName>\
		{ \
		public:\
			representationName##StaticRequiringRegistrator() : StaticRequiringRegistrator<representationName>(#representationName) {} \
		} the##representationName; \
	protected: \
		inline const representationName& get##representationName() const { \
			static const representationName& representation = the##representationName.get(getBlackBoard()); \
			return representation; \
		}

// static invoker (registers the static dependency to representationName)
#define RECYCLE(representationName) \
	private: \
		class representationName##StaticRecyclingRegistrator : public StaticRecyclingRegistrator<representationName>\
		{ \
		public: \
			representationName##StaticRecyclingRegistrator() : StaticRecyclingRegistrator<representationName>(#representationName){} \
		} the##representationName; \
	protected: \
		inline const representationName& get##representationName() const { \
			static const representationName& representation = the##representationName.get(getBlackBoard()); \
			return representation; \
		}

// static invoker (registers the static dependency to representationName)
#define PROVIDE(representationName) \
	private: \
		class representationName##StaticProvidingRegistrator : public StaticProvidingRegistrator<representationName>\
		{ \
		public: \
			representationName##StaticProvidingRegistrator() : StaticProvidingRegistrator<representationName>(#representationName){} \
		} the##representationName; \
	protected: \
		inline representationName& get##representationName() const { \
			return the##representationName.getData(); \
		}


#define END_DECLARE_MODULE(moduleName) \
	public: \
		moduleName##Base(): Module(#moduleName) \
		{ \
			registerRequiring(*getRequiringRegistry<moduleName##Base>()); \
			registerRecycling(*getRecyclingRegistry<moduleName##Base>()); \
			registerProviding(*getProvidingRegistry<moduleName##Base>()); \
		} \
		virtual ~moduleName##Base() { \
			unregisterRequiring(*getRequiringRegistry<moduleName##Base>()); \
			unregisterRecycling(*getRecyclingRegistry<moduleName##Base>()); \
			unregisterProviding(*getProvidingRegistry<moduleName##Base>()); \
		} \
		inline static const char* getModuleDebugSymbol() { \
			static const char* name = "modules.debug."#moduleName; \
			return name; \
		} \
		inline void debug(const char* format, ...) const __attribute__ ((format (printf, 2, 3))) { \
			static const DebuggingOption* debugOption = ::Debugging::getInstance().getDebugOption(getModuleDebugSymbol()); \
			if (debugOption && debugOption->enabled) { \
				va_list vl; \
				va_start(vl, format); \
				::Debugging::getInstance().sendDebugText(debugOption, format, vl); \
				va_end(vl); \
			} \
		} \
	};

#endif //__Module_h_
