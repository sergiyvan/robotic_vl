/*
 * ansiTools.h
 *
 *  Created on: Mar 14, 2012
 *      Author: dseifert
 */

#ifndef ANSICOLORS_H_
#define ANSICOLORS_H_

#include "platform/system/macroMagic.h"


#define TERM_RESET                      "\033[0m"

#define TERM_UNDERLINE                  "\033[4m"
#define TERM_UNDERLINE_OFF              "\033[24m"

#define TERM_BLINK_SLOW                 "\033[5m"
#define TERM_BLINK_FAST                 "\033[6m"
#define TERM_BLINK_OFF                  "\033[25m"

#define TERM_BLACK                      "\033[30m"
#define TERM_RED                        "\033[31m"
#define TERM_GREEN                      "\033[32m"
#define TERM_BROWN                      "\033[33m"
#define TERM_BLUE                       "\033[34m"
#define TERM_PURPLE                     "\033[35m"
#define TERM_CYAN                       "\033[36m"
#define TERM_LIGHT_GRAY                 "\033[37m"

#define TERM_SAVE_CURSOR                "\033[s"
#define TERM_LOAD_CURSOR                "\033[u"

#define TERM_CURSOR_POS(y, x)           "\033[" STRINGIFY(y) ";" STRINGIFY(x) "H"
#define TERM_CURSOR_UP(n)               "\033[" STRINGIFY(n) "A"
#define TERM_CURSOR_DOWN(n)             "\033[" STRINGIFY(n) "B"
#define TERM_CURSOR_RIGHT(n)            "\033[" STRINGIFY(n) "C"
#define TERM_CURSOR_LEFT(n)             "\033[" STRINGIFY(n) "D"
#define TERM_GOTO_PREVIOUS_LINE(n)      "\033[" STRINGIFY(n) "F"

#define TERM_CLEAR_SCREEN               "\033[2J"
#define TERM_CLEAR_SCREEN_DOWN          "\033[0J"
#define TERM_CLEAR_SCREEN_UP            "\033[1J"

#define TERM_ERASE_TO_END_OF_LINE       "\033[0K"
#define TERM_ERASE_FROM_START_OF_LINE   "\033[1K"
#define TERM_ERASE_LINE                 "\033[2K"

#define TOP_MENU(lines, title)          { printf(TERM_SAVE_CURSOR TERM_CURSOR_POS(1,1)); \
                                          for (int i=0; i<lines+3; i++) printf(TERM_ERASE_LINE "\n"); \
                                          printf(TERM_CURSOR_POS(1, 1) TERM_CYAN "--------- " TERM_PURPLE " %s " TERM_CYAN "-------------------------------------------------------------------------------------\n" TERM_RESET, title); \
                                          printf(TERM_CURSOR_POS(lines, 1) TERM_CYAN "\n\n-----------------------------------------------------------------------------------------------\n" TERM_RESET); \
                                          printf(TERM_CURSOR_POS(lines, 1) "\n\n\n" TERM_ERASE_LINE); \
                                          printf(TERM_CURSOR_POS(2, 1) TERM_ERASE_LINE); \
                                        }
#define END_MENU                        printf(TERM_LOAD_CURSOR);


#endif /* ANSITOOLS_H_ */
