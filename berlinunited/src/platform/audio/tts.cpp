#include "tts.h"
#include "debug.h"

#include <string.h>

#include <espeak/speak_lib.h>


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

static int espeak_callback(short *wav, int numsamples, espeak_EVENT *events) {
	// process events
	for (espeak_EVENT *event=events; event->type != 0; event++) {
		if (event->type == espeakEVENT_MSG_TERMINATED) {
			printf("tts finished\n");

			// free memory for asynchronous playback
			if (event->user_data != 0) {
				delete [] (char*)event->user_data;
			}
		}
	}

	return 0;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

TTS::TTS() {
	int sampleRateInHz = espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 300, NULL, 0);
	if (sampleRateInHz < 0) {
		WARNING("Initialization of TextToSpeech-system failed.");
	}

	espeak_SetSynthCallback(espeak_callback);
}




/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

TTS::~TTS() {

}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Outputs spoken text
 */

void TTS::speak(const char* text, SpokenLanguage language, SpeechMode mode) {
	uint16_t textLength  = strlen(text);
	char* textToBeSpoken = new char[textLength+1];
	strcpy(textToBeSpoken, text);

	if (language == ENGLISH)
		espeak_SetVoiceByName("en");
	else if (language == GERMAN)
		espeak_SetVoiceByName("de");
	else
		ERROR("Unknown language passed, using default.");

	espeak_Synth(
			textToBeSpoken,  // text to be spoken
			textLength,      // length of text
			0,               // start position
			POS_CHARACTER,   // unit in which start/end position is counted
			0,               // end position
			0,               // flags
			0,               // unique identifier
			textToBeSpoken   // user data (will be passed to callback function)
		);

	// wait for output of speech if requested
	if (mode == SPEAK_SYNCHRONOUSLY) {
		espeak_Synchronize();
	}
}
