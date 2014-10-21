/** @file
 **
 **
 */

#include "debugging.h"
#include "imageDebugger.h"
#include "services.h"

#include "communication/comm.h"
#include "management/commandLine.h"

#include "msg_debugging.pb.h"
#include "msg_image.pb.h"

#include <sstream>

#include <utility> // needed for std::move

/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
 */
Debugging::Debugging()
	: consoleLogLevel(CONSOLE_LOG_PERMIT_ALL)
	, cs()
	, registeredImages()
	, imageDebuggers()
	, debugging_options()
{
	cs.setName("Debugging");

	// Register a new fake image (index 0) for backward compatibility reasons.
	// This should never be provided. The reason is that in the past the hard
	// coded image types started at value 1, so we need to "hide" index 0 now.
	registerImage("Invalid");

	// Register the default image view.
	registerImage("Camera");

	Services::getInstance().getMessageRegistry().registerMessageCallback(this, "debuggingCommands");
	Services::getInstance().getMessageRegistry().registerMessageCallback(this, "imageTypeRequest");
	setConsoleLogLevel(CONSOLE_LOG_PERMIT_ALL);
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Destructor
 */
Debugging::~Debugging() {
	DebugOptionContainer::iterator it;
	for (it = debugging_options.begin(); it != debugging_options.end(); it++)
		delete it->second;

	debugging_options.clear();

	for (auto it = imageDebuggers.begin(); it != imageDebuggers.end(); ++it) {
		delete (*it);
	}
	imageDebuggers.clear();
}

/*------------------------------------------------------------------------------------------------*/

/** Registers an image
 **
 */

uint32_t Debugging::registerImage(const std::string &imageName) {
	CriticalSectionLock lock(cs);

	uint32_t index = imageDebuggers.size();
	imageDebuggers.push_back(new ImageDebugger(imageName, index));
	registeredImages[imageName] = index;
	return index;
}

/*------------------------------------------------------------------------------------------------*/

/** Registers a debug option
 **
 ** @param name    Name of debug option
 ** @param type    Type of debug option
 ** @param flags   Additional flags
 **
 ** @return true iff debug option was registered successfully
 */

bool Debugging::registerDebugOption(
	std::string          name,
	DebuggingOptionType  type,
	unsigned char        flags,
	const char*          filename,
	const char*          description)
{
	CriticalSectionLock lock(cs);

	DebuggingOption *new_option = new DebuggingOption();
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	new_option->name = std::move(name);
	new_option->type = type;
	new_option->srcLocation = filename    ? filename    : "";
	new_option->description = description ? description : new_option->srcLocation;

	if ((flags & ENABLED) == ENABLED)
		new_option->enabled = true;
	else
		new_option->enabled = false;

	if ((flags & CMDLOUT) == CMDLOUT)
		new_option->cmdlineout = true;
	else
		new_option->cmdlineout = false;

	if ((flags & NONET) == NONET) {
		if (type == TEXT) {
			new_option->netout = false;
		} else {
			printf("ERROR: Flag NONET (for debug option %s) is only available for TEXT.\n", new_option->name.c_str());
			new_option->netout = true;
		}
	} else
		new_option->netout = true;

	typedef Debugging::DebugOptionContainer::value_type pair;
	if (debugging_options.insert( pair(new_option->name, new_option) ).second == false ) {
		printf("ERROR: debug option %s already exists.\n", new_option->name.c_str());
		delete new_option;
	}

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/** Message callback
 **
 ** @param messageName    Name/type of message
 ** @param msg            Received message
 ** @param senderID       ID of sender
 **
 ** @return true if message was processed
 */

bool Debugging::messageCallback(
	const std::string               &messageName,
	const google::protobuf::Message &msg,
	int32_t                          id,
	RemoteConnectionPtr             &remote)
{
	if (messageName == "de.fumanoids.message.imageTypeRequest") {
		de::fumanoids::message::Message msg;
		de::fumanoids::message::ImageTypes &types = *msg.MutableExtension(de::fumanoids::message::imageTypes);

		CriticalSectionLock lock(cs);
		for (uint32_t i = 1; i < imageDebuggers.size(); i++) {
			types.add_types(imageDebuggers[i]->getName());
		}

		services.getComm().sendMessage(msg, remote.get());
		return true;
	}

	if (messageName != "de.fumanoids.message.debuggingCommands") {
		return false;
	}

	const de::fumanoids::message::DebuggingCommands &message =
			(const de::fumanoids::message::DebuggingCommands&)msg;

	if (message.has_optionpull()) {
		de::fumanoids::message::Message msg;
		de::fumanoids::message::DebuggingCommands             &dbg     = *msg.MutableExtension(de::fumanoids::message::debuggingCommands);
		de::fumanoids::message::DebuggingCommands::OptionPush &options = *dbg.mutable_optionpush();

		msg.set_robotid(services.getID());

		cs.enter();
		for (DebugOptionContainer::const_iterator it = debugging_options.begin(); it != debugging_options.end(); ++it) {
			de::fumanoids::message::DebuggingCommands::DebuggingOption &option = *options.add_options();

			option.set_name((*it).second->name);
			option.set_type((de::fumanoids::message::DebuggingCommands_DebuggingOptionType)(*it).second->type);
			option.set_enabled((*it).second->enabled);
			option.set_description((*it).second->description);
			option.set_srclocation((*it).second->srcLocation);
		}
		cs.leave();

		services.getComm().sendMessage(msg, remote.get());
	}

	if (message.has_optionenable()) {
		for (int i=0; i<message.optionenable().options_size(); i++)
			switchOption(message.optionenable().options(i), true);
	}

	if (message.has_optiondisable()) {
		for (int i=0; i<message.optiondisable().options_size(); i++)
			switchOption(message.optiondisable().options(i), false);
	}

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Changes the status of <i>option</i> and all "childs" according to
 ** <i>enable</i>.
 **
 ** @param option
 ** @param enable
 */

void Debugging::switchOption(const std::string &option, bool enable) {
	DebugOptionContainer::iterator it;

	CriticalSectionLock lock(cs);
	for (it = debugging_options.begin(); it != debugging_options.end(); ++it) {
		if (   (*it).second->name.find(option + ".") == 0
			|| (*it).second->name == option)
		{
			(*it).second->enabled = enable;
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param option
 ** @param format
 ** @param vl
 */

void Debugging::processMessageOut(
	const DebuggingOption *option,
	const char *format,
	va_list &vl) const
{
	if (option->type != TEXT)
		return;

	const int PayLoadMaxLength = 1024;
	char payload[PayLoadMaxLength] = { 0 };

	if (vsnprintf(payload, PayLoadMaxLength, format, vl) < 0)
		return;

	de::fumanoids::message::Message    msg;
	de::fumanoids::message::Debugging &dbg = *msg.MutableExtension(de::fumanoids::message::debugging);

	msg.set_robotid(services.getID());

	de::fumanoids::message::Debugging::OptionText &txt = *dbg.mutable_optiontext();
	txt.set_option(option->name);
	txt.set_payload(payload);

	sendMessage(msg);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param option
 ** @param payload
 */

void Debugging::processMessageOut(const DebuggingOption *option, const char *payload) const {
	if (option->type != TEXT)
		return;

	de::fumanoids::message::Message    msg;
	de::fumanoids::message::Debugging &dbg = *msg.MutableExtension(de::fumanoids::message::debugging);

	msg.set_robotid(services.getID());

	de::fumanoids::message::Debugging::OptionText &txt = *dbg.mutable_optiontext();
	txt.set_option(option->name);
	txt.set_payload(payload);

	sendMessage(msg);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param option
 ** @param series
 ** @param x
 ** @param y
 */

void Debugging::processMessageOut(
	const DebuggingOption *option,
	const std::string &series,
	double x,
	double y) const
{
	if (option->type != PLOTTER)
		return;

	de::fumanoids::message::Message    msg;
	de::fumanoids::message::Debugging &dbg = *msg.MutableExtension(de::fumanoids::message::debugging);

	msg.set_robotid(services.getID());

	de::fumanoids::message::Debugging::OptionPlotter &plotter = *dbg.mutable_optionplotter();
	plotter.set_option(option->name);
	plotter.set_series(series);
	plotter.set_x(x);
	plotter.set_y(y);

	sendMessage(msg);
}

/**
 **
 ** @param option
 ** @param series
 ** @param x
 ** @param y
 */

void Debugging::processMessageOut(
	const DebuggingOption *option,
	const de::fumanoids::message::Debug3D &commands) const
{
	if (option->type != PLOTTER3D)
		return;

	de::fumanoids::message::Message    msg;
	de::fumanoids::message::Debugging &dbg = *msg.MutableExtension(de::fumanoids::message::debugging);

	msg.set_robotid(services.getID());

	de::fumanoids::message::Debugging::Option3D &plotter3d = *dbg.mutable_option3d();
	plotter3d.set_option(option->name);
	*plotter3d.mutable_data() = commands;

	sendMessage(msg);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param option
 ** @param row
 ** @param column
 ** @param payload
 */

void Debugging::processMessageOut(
	const DebuggingOption *option,
	const std::string &key,
	const std::string &payload) const
{
	if (option->type != TABLE)
		return;

	de::fumanoids::message::Message    msg;
	de::fumanoids::message::Debugging &dbg = *msg.MutableExtension(de::fumanoids::message::debugging);

	msg.set_robotid(services.getID());

	de::fumanoids::message::Debugging::OptionTable &table = *dbg.mutable_optiontable();
	table.set_option(option->name);
	table.set_key(key);
	table.set_payload(payload);

	sendMessage(msg);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param option
 ** @param row
 ** @param column
 ** @param payload
 */

void Debugging::processMessageOut(
	const DebuggingOption *option,
	const std::string &key,
	const double payload) const
{
	if (option->type != TABLE)
		return;

	std::stringstream ss;
	ss << payload;
	processMessageOut(option, key, ss.str());
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param msg
 ** @return
 */

void Debugging::sendMessage(de::fumanoids::message::Message &msg) const {
	services.getComm().broadcastMessage(msg);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void Debugging::sendImageDebug(int frameNumber) {
	for (auto it = imageDebuggers.begin(); it != imageDebuggers.end(); ++it) {
		(*it)->streamImage(frameNumber);
	}
}
