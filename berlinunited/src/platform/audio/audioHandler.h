#ifndef AUDIOHANDLER_H_
#define AUDIOHANDLER_H_

#include <inttypes.h>


typedef enum {
	  SND_WAV = 0

} SOUNDFORMAT;


/**
 ** An audio handler is the interface between our code and the
 ** hardware (or more accurately the external, e.g. OS, interface
 ** to the audio hardware).
 */

class AudioHandler {
public:
	AudioHandler();
	virtual ~AudioHandler();

	virtual void    play(const void* soundData, SOUNDFORMAT=SND_WAV);
	virtual void    play(const char* soundFilename);

	virtual void    setVolume(uint8_t volumeInPercent);
	virtual uint8_t getVolume();

	virtual void    mute();
	virtual void    unmute();
};

#endif /* AUDIOHANDLER_H_ */
