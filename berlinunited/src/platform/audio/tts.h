#ifndef TTS_H_
#define TTS_H_

/*------------------------------------------------------------------------------------------------*/

typedef enum {
	ENGLISH,
	GERMAN
} SpokenLanguage;

typedef enum {
	SPEAK_ASYNCHRONOUSLY,
	SPEAK_SYNCHRONOUSLY
} SpeechMode;


/*------------------------------------------------------------------------------------------------*/

/**
 ** The TTS class handles text-to-speech output.
 **
 */

class TTS {
public:
	TTS();
	virtual ~TTS();

	void speak(const char* text, SpokenLanguage language=ENGLISH, SpeechMode mode=SPEAK_SYNCHRONOUSLY);

protected:
};

#endif /* TTS_H_ */
