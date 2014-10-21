/*
 * keyboard.h
 *
 *  Created on: Mar 3, 2012
 *      Author: dseifert
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

void takeKeyboard();
void releaseKeyboard();

int getKeyWithMsTimeout(int timeoutInMilliseconds);
int getKeyWithUsTimeout(int timeoutInMicroSeconds);
int getKey();


#endif /* KEYBOARD_H_ */
