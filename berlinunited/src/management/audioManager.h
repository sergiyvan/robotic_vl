#ifndef AUDIOMANAGER_H_
#define AUDIOMANAGER_H_

#include "platform/audio/tts.h"
#include "platform/audio/audioHandler.h"

/*------------------------------------------------------------------------------------------------*/

/**
 ** The AudioManager handles audio requests.
 **
 */

class AudioManager {
public:
	AudioManager();
	virtual ~AudioManager();

	void speak(const char* text, SpokenLanguage language=ENGLISH);

protected:
	AudioHandler audioHandler;
	TTS          tts;
};

#endif /* AUDIOMANAGER_H_ */
