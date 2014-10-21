#pragma once
#ifndef MODULEMANAGERTRIGGERED_H_
#define MODULEMANAGERTRIGGERED_H_


#include "ModuleManager.h"

#include "utils/units.h"
#include "platform/system/events.h"



class ModuleManagerTriggered
	: virtual public ModuleManager
	, public EventCallback
{
public:
	ModuleManagerTriggered(EventType eventType);
	virtual ~ModuleManagerTriggered();

	virtual void startManager(int runlevel) override;
	virtual void stopManager() override;

	virtual void eventCallback(EventType evtType, void* data);

protected:
	EventType eventType;
	int runLevel;
};


#endif
