#ifndef IMAGESTREAMPROVIDER_H__
#define IMAGESTREAMPROVIDER_H__

#include "debugging/debugging.h"
#include "debugging/imageDebugger.h"
#include "communication/remoteConnection.h"
#include "communication/remoteTCPConnection.h"
#include "communication/comm.h"
#include "management/config/config.h"
#include "platform/system/transport/transport_tcp.h"

#include "services.h"
#include "debug.h"

class ImageStreamProvider : public EventCallback {
public:
	ImageStreamProvider()
		: imageDebugger(nullptr)
		, imageStreamingEnabled(false)
		, imageStreamingInterval(1000*milliseconds)
		, imageStreamingByUDP(false)
	{
		services.getEvents().registerForEvent(EVT_CONFIGURATION_LOADED, this);
	}

	virtual ~ImageStreamProvider() {
	}

	void setupImageProvider(const std::string &imageTypeName) {
		imageDebugger = &Debugging::getInstance().getImageDebugger(imageTypeName);

		// (re-?)load configuration initially
		this->eventCallback(EVT_CONFIGURATION_LOADED, &(services.getConfig()));
	}

	virtual void streamUsingBroadcast(Millisecond interval = 500 * milliseconds) {
		imageDebugger->setImageStreaming(true, interval, services.getComm().broadcastConnection);
	}

	virtual void streamUsingTCP(Millisecond interval = 500 * milliseconds) {
		int port = services.getConfig().get<int>("comm.remote.port", -1);
		if (port <= 0) {
			ERROR("Incomplete setup of remote server, not streaming via TCP.");
			return;
		}

		std::string ip = services.getConfig().get<std::string>("comm.remote.ip", "");
		if (ip == "") {
			ERROR("Incomplete setup of remote server, not streaming via TCP.");
			return;
		}

		TransportTCP *transport = new TransportTCP(port, ip);
		if (false == transport->open()) {
//			ERROR("Could not connect to remote server, not streaming via TCP.");
		}

		std::shared_ptr<RemoteConnection> connection(new RemoteTCPConnection(transport));
		imageDebugger->setImageStreaming(true, interval, connection);
	}

	// this should be called whenever a new image is available
	virtual void stream(int frameNumber) {
		if (nullptr == imageDebugger) {
			ERROR("ImageDebugger not set up.");
			return;
		}

		if (false == imageDebugger->requiresImageData())
			return;

		de::fumanoids::message::Image pbImage;
		pbImage.set_frameno(frameNumber);
		pbImage.set_robotid(services.getID());

		de::fumanoids::message::ImageData *pbRawImage = pbImage.add_imagedata();
		setImageData(pbRawImage);
		pbRawImage->set_original(true);

		imageDebugger->setImage(pbImage);
	}

	virtual void setImageData(de::fumanoids::message::ImageData *pbImageData) = 0;

	virtual void eventCallback(EventType eventType, void* data) {
		if (eventType == EVT_CONFIGURATION_LOADED) {
			std::string enabledOptionName  = "streaming." + imageDebugger->getName() + ".enabled";
			std::string intervalOptionName = "streaming." + imageDebugger->getName() + ".interval";
			std::string udpOptionName      = "streaming." + imageDebugger->getName() + ".udp";

			bool        newImageStreamingEnabled  = services.getConfig().get<bool>(enabledOptionName);
			Millisecond newImageStreamingInterval = services.getConfig().get<Millisecond>(intervalOptionName);
			bool        newImageStreamingUDP      = services.getConfig().get<bool>(udpOptionName);

			bool configChanged =    imageStreamingEnabled != newImageStreamingEnabled
			                     || imageStreamingByUDP != newImageStreamingUDP
			                     || imageStreamingInterval != newImageStreamingInterval;

			imageStreamingEnabled  = newImageStreamingEnabled;
			imageStreamingInterval = newImageStreamingInterval;
			imageStreamingByUDP    = newImageStreamingUDP;

			// stop streaming when the configuration changed
			if (configChanged)
				imageDebugger->setImageStreaming(false, 0*milliseconds, nullptr);

			// if streaming is active, start it now
			if (imageStreamingEnabled) {
				if (services.getConfig().get<bool>(udpOptionName)) {
					streamUsingBroadcast(imageStreamingInterval);
				} else {
					streamUsingTCP(imageStreamingInterval);
				}
			}
		}
	}

protected:
	ImageDebugger *imageDebugger;

	bool           imageStreamingEnabled;
	Millisecond    imageStreamingInterval;
	bool           imageStreamingByUDP;
};

#endif
