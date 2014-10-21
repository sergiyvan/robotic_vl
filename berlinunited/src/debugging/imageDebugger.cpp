#include "imageDebugger.h"
#include "services.h"
#include "communication/comm.h"

#include "msg_debugging.pb.h"


/*------------------------------------------------------------------------------------------------*/

REGISTER_IMAGE_OPTIONS(Camera);


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

ImageDebugger::ImageDebugger(const std::string &name, uint32_t type)
	: drawCommands()
	, cs()
	, name(name)
	, type(type)
	, manualStreaming(false)
	, manualStreamingInterval(1000*milliseconds)
	, manualStreamingLastSent(0*milliseconds)
	, manualStreamingConnection(nullptr)
	, clients()
	, imageMessage()
	, hasImage(false)
{
//	printf("registered image debugger for %d\n", type);
	cs.setName("ImageDebugger");

	// register for message callbacks
	Services::getInstance().getMessageRegistry().registerMessageCallback( this, "imageRequest");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

ImageDebugger::~ImageDebugger() {
	// unregister
	services.getMessageRegistry().unregisterMessageCallback( this, "imageRequest");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool ImageDebugger::messageCallback(const std::string &messageName, const google::protobuf::Message &msg, int32_t robot_id, RemoteConnectionPtr &remote) {
	const de::fumanoids::message::ImageRequest request = (const de::fumanoids::message::ImageRequest&)msg;

	// filter for our type of image
	if (request.type() != type) {
		return false;
	}

	CriticalSectionLock lock(cs);

	StreamingClient *client = new StreamingClient();

	client->countRemaining = request.count();
	client->interval       = request.interval()*milliseconds;
	client->type           = request.type();
	client->scaling        = request.scaling();
	client->remote         = std::move( remote );
	client->lastTime       = 0L;
	client->compress       = request.compressed();
	client->rgbImage       = request.includergb();
	client->rawImage       = request.includeraw();

	clients.push_back(client);
//	printf("adding client (type %d, %d images in %d interval)\n", type, client->countRemaining, (int)client->interval.value());
//	printf("connection %d\n", client->remote.get()->isConnected());
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/** Check whether an image should be streamed (i.e. data provided).
 **
 ** @return true iff image data should be provided for streaming. If false is
 **         returned, image data can still be supplied but won't be sent.
 */

bool ImageDebugger::requiresImageData() {
	if (manualStreaming) {
		if (manualStreamingLastSent + manualStreamingInterval < getCurrentTime())
			return true;
	}

	// stream image to clients
	for (uint32_t index = 0; index < clients.size(); ) {
		// get rid of obsolete connections
		if (   0 == clients[index]->countRemaining
		    || false == clients[index]->remote.get()->isConnected())
		{
			CriticalSectionLock lock(cs);
			delete clients[index];
			clients.erase(clients.begin() + index);
			continue;
		} else {
			if (clients[index]->lastTime + clients[index]->interval < getCurrentTime()) {
				return true;
			}

			index++;
		}
	}

	return false;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ImageDebugger::setImageStreaming(bool enabled, Millisecond interval, std::shared_ptr<RemoteConnection> connection) {
	CriticalSectionLock lock(cs);

	manualStreamingConnection = connection;
	manualStreaming = enabled;

	if (enabled) {
		printf("enabled image streaming for %s\n", this->name.c_str());
		manualStreamingInterval = interval;
	}
}



/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ImageDebugger::setImage(const de::fumanoids::message::Image &pbImage) {
	CriticalSectionLock lock(cs);

	imageMessage.Clear();
	imageMessage.MutableExtension(de::fumanoids::message::image)->CopyFrom(pbImage);
	imageMessage.MutableExtension(de::fumanoids::message::image)->set_type(type);

	// fill in missing fields
	if (false == pbImage.has_time())
		imageMessage.MutableExtension(de::fumanoids::message::image)->set_time(getCurrentTime().value());

	if (false == pbImage.has_robotid())
		imageMessage.MutableExtension(de::fumanoids::message::image)->set_robotid(services.getID());

	hasImage = true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ImageDebugger::streamImage(int frameNumber) {
	CriticalSectionLock lock(cs);

	// broadcast drawings
	if (drawCommands.size() > 0) {
		de::fumanoids::message::Message response;
		de::fumanoids::message::Debugging &debug = *response.MutableExtension(de::fumanoids::message::debugging);

		for (ImageDebugger::DrawCommandMap::const_iterator it = drawCommands.begin(); it != drawCommands.end(); it++) {
			de::fumanoids::message::Debugging::OptionDrawing *optionDrawing = debug.add_optiondrawing();
			optionDrawing->set_option(it->first);
			optionDrawing->set_framenumber(frameNumber);

			const de::fumanoids::message::Drawings &drawings = it->second;
			typedef ::google::protobuf::RepeatedPtrField<de::fumanoids::message::Drawing> Drawings;
			for (Drawings::const_iterator drawIt = drawings.drawings().begin(); drawIt != drawings.drawings().end(); drawIt++) {
				de::fumanoids::message::Drawing *drawing = optionDrawing->add_drawings();
				*drawing = *drawIt;
			}
		}

		services.getComm().broadcastMessage(response);
		clear();
	}

	// stream image to clients
	if (hasImage) {
		bool imageSent = false;

		de::fumanoids::message::Image *pbImage = imageMessage.MutableExtension(de::fumanoids::message::image);
		pbImage->set_frameno(frameNumber);
		pbImage->set_robotid(services.getID());

		// check clients
		for (uint32_t index = 0; index < clients.size(); ) {
			// get rid of obsolete connections
			if (   clients[index]->countRemaining <= 0
			    || false == clients[index]->remote.get()->isConnected())
			{
				CriticalSectionLock lock(cs);
				delete clients[index];
				clients.erase(clients.begin() + index);
				continue;
			} else {
				if (clients[index]->lastTime + clients[index]->interval < getCurrentTime()) {
					services.getComm().sendMessage(imageMessage, clients[index]->remote.get());

					if (clients[index]->remote.get()->isConnected() == false) {
						CriticalSectionLock lock(cs);
						delete clients[index];
						clients.erase(clients.begin() + index);
						continue;
					}

					clients[index]->countRemaining--;
					clients[index]->lastTime = getCurrentTime();
					imageSent = true;
				}

				index++;
			}
		}

		// check manual
		if (manualStreaming && manualStreamingLastSent + manualStreamingInterval < getCurrentTime()) {
			if (nullptr == manualStreamingConnection) {
				services.getComm().broadcastMessage(imageMessage);
				imageSent = true;
			} else {
				if (false == manualStreamingConnection->isConnected()) {
//					WARNING("not connected, trying to re-establish");
					if (manualStreamingConnection->connect()) {
//						INFO("Image streaming connection re-established.");
					} else {
//						INFO("Re-establishing failed");
					}
				}
				if (manualStreamingConnection->isConnected()) {
					services.getComm().sendMessage(imageMessage, manualStreamingConnection);
					imageSent = true;
				}
			}

			manualStreamingLastSent = getCurrentTime();
		}

		if (imageSent) {
			imageMessage.Clear();
			hasImage = false;
		}
	}
}
