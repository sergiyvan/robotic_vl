#pragma once
#ifndef MODULEMANAGERCLOCKED_H_
#define MODULEMANAGERCLOCKED_H_

#include "ModuleManager.h"

#include "utils/units.h"
#include "management/config/configOption.h"



class ModuleManagerClocked
	: virtual public ModuleManager
	, virtual public Thread
{
public:
	ModuleManagerClocked();
	virtual ~ModuleManagerClocked();

	virtual void startManager(int runlevel) override;
	virtual void stopManager() override;

	virtual void threadMain() override;

protected:
	CriticalSection cs;

	int runLevel;
};

#endif
