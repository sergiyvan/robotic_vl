/**
* @file ModuleCreator.h
*
* @author <a href="mailto:mellmann@informatik.hu-berlin.de">Heinrich Mellmann</a>
* Declaration of class ModuleCreator
*/

#ifndef __ModuleCreator_h__
#define __ModuleCreator_h__

#include "BlackBoardInterface.h"
#include "debug.h"

class Module;

/**
 * AbstractModuleCreator is an interface.
 * It is used to create lists of modules.
 * Additionally it provides functionality to enable/disable the module
 */
class AbstractModuleCreator
{
public:
	virtual void setEnabled(bool value) = 0;
	virtual bool isEnabled() const = 0;
	virtual void execute() = 0;
	virtual Module* getModule() = 0;
	virtual ~AbstractModuleCreator() {};
};

/**
 * ModuleInstance is needed to instantiate the 
 * BlackBoardInterface of the class V (if it has one)
 * with a blackboard.
 * We assume that the class V already inherits from BlackBoardInterface.
 * Thereby 'virtual' inheritance is essential.
 * 
 * (in fact, what we are doing is to extend the default constructor of the class V
 *  by providing a pointer to a blackboard instance, i.e., we call another 
 *  constructor of the BlackBoardInterface)
 */
template<class V>
class ModuleInstance: virtual public BlackBoardInterface, virtual public V
{
public:
	ModuleInstance(BlackBoard& theBlackBoard)
		: BlackBoardInterface(&theBlackBoard)
	{
	}
};

/**
 * ModuleCreator implements the AbstractModuleCreator.
 */
template<class V>
class ModuleCreator: public AbstractModuleCreator {
private:
	BlackBoard& theBlackBoard;
	ModuleInstance<V>* theInstance;

	// cannot be copied
	ModuleCreator& operator=(const ModuleCreator&) {
	}

	// whether module is enabled
	bool enabled;

	// whether module is initialized
	bool initialized;

public:

	ModuleCreator(BlackBoard& theBlackBoard)
		: theBlackBoard(theBlackBoard)
		, theInstance(NULL)
		, enabled(false)
		, initialized(false)
	{
	}

	virtual ~ModuleCreator() {
		delete theInstance;
	}

	void create() {
		theInstance = new ModuleInstance<V>(theBlackBoard);
	}

	virtual bool isEnabled() const {
		return enabled;
	}

	virtual void setEnabled(bool value) {
		enabled = value;
	}

	virtual bool isInitialized() {
		ASSERT(isEnabled());
		if (theInstance == NULL)
			create();

		return initialized;
	}

	virtual void execute() {
		ASSERT(isEnabled());

		// create module if required
		if (theInstance == NULL)
			create();

		// initialize module if necessary
		if (false == isInitialized())
			init();

		theInstance->execute();
	}

	virtual Module* getModule() {
		// todo: check, the class V is not necessary a module
		if (theInstance == NULL)
			create();

		return (Module*) (theInstance);
	}

	V* getModuleT() {
		if (theInstance == NULL)
			create();
		return static_cast<V*>(theInstance);
	}

	virtual void init() {
		if (true == initialized)
			return;

		if (theInstance == NULL)
			create();

		theInstance->init();
		initialized = true;
	}
};

#endif //__ModuleCreator_h__
