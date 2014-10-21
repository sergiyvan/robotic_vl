/*
 * keyboard.cpp
 *
 *  Created on: Mar 3, 2012
 *      Author: dseifert
 */

#include "keyboard.h"

#include <stdio.h>         // UNIX standard function definitions
#include <unistd.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>         // File control definitions
#include <errno.h>
#include <string.h>

static bool keyboardIsSet = false;


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

static void toggleKeyboard() {
	static struct termios oldt;
	static struct termios newt;

	if (keyboardIsSet == false) {
		tcgetattr(STDIN_FILENO, &oldt); /*store old settings */
		newt = oldt; /* copy old settings to new settings */
		newt.c_lflag &= ~(ICANON | ECHO); /* make one change to old settings in new settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &newt); /*apply the new settings immediately */
		keyboardIsSet = true;
	} else {
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*re-apply the old settings */
		keyboardIsSet = false;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

void takeKeyboard() {
	if (keyboardIsSet == false)
		toggleKeyboard();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

void releaseKeyboard() {
	if (keyboardIsSet == true)
		toggleKeyboard();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

int getKey() {
	if (keyboardIsSet == true)
		return getchar();
	else {
		takeKeyboard();
		int c = getchar();
		releaseKeyboard();
		return c;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 * Reads one char from stdin and returns its value.
 * Does not need CR as confirmation of the input.
 *
 * @return value of read char
 */

int getKeyWithMsTimeout(int timeoutInMilliSeconds) {
	return getKeyWithUsTimeout(1000*timeoutInMilliSeconds);
}


/*------------------------------------------------------------------------------------------------*/

/**
 * Reads one char from stdin and returns its value.
 * Does not need CR as confirmation of the input.
 *
 * @return value of read char
 */

int getKeyWithUsTimeout(int timeoutInMicroSeconds) {
	bool controlKeyboard = false;
	if (!keyboardIsSet) {
		controlKeyboard = true;
		takeKeyboard();
	}

	struct timeval timeout = { 0, timeoutInMicroSeconds /* us */ };
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	select(FD_SETSIZE, &readfds, 0, 0, &timeout);
	int c = 0;
	if (FD_ISSET(STDIN_FILENO, &readfds))
		c = getKey();

	if (controlKeyboard) {
		releaseKeyboard();
	}

	return c;
}
