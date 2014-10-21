#include "audioManager.h"
#include "services.h"

#include "management/config/configRegistry.h"
#include "management/config/config.h"
#include "platform/audio/audioHandler.h"


/*------------------------------------------------------------------------------------------------*/

namespace {
	auto cfgAudioEnabled = ConfigRegistry::registerOption<bool>("audio.enabled", true, "Enables or disables audio output");
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
 */

AudioManager::AudioManager()
	: audioHandler()
	, tts()
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Destructor
 */

AudioManager::~AudioManager() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Outputs spoken text
 */

void AudioManager::speak(const char* text, SpokenLanguage language) {
	if (cfgAudioEnabled->get())
		tts.speak(text, language, SPEAK_SYNCHRONOUSLY);
}
