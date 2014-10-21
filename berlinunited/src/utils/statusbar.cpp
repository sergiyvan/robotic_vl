#include "statusbar.h"
#include "ansiTools.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>



/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

Statusbar::Statusbar(int height)
	: height(height)
	, startLine(0)
{
	aquire();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

Statusbar::~Statusbar() {
	release();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void Statusbar::aquire() {
	// get current window size
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
printf("window height %d\n", w.ws_row);
	// reserve the lower lines
	w.ws_row -= height;
	printf("top window height %d\n", w.ws_row);
	if (ioctl(STDOUT_FILENO, TIOCSWINSZ, &w) < 0)
		printf("TIOCSWINSZ failed to reserve space for menu bar\n");

	// remember start of statusbar
	startLine = w.ws_row + 1;
	printf("startLine %d\n", startLine);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void Statusbar::release() {
	// get current window size
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	// release the lower lines
	w.ws_row += height;
	if (ioctl(STDOUT_FILENO, TIOCSWINSZ, &w) < 0)
		printf("TIOCSWINSZ failed to release space for menu bar\n");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void Statusbar::clear() {

}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void Statusbar::update(std::stringstream ss) {
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void Statusbar::print(int line, std::string str) {
	std::stringstream ss;
	ss << TERM_SAVE_CURSOR
	   << "\033[";
	ss << startLine;
	ss << ";1H"
	   << str
	   << TERM_LOAD_CURSOR;

	release();
	printf("%s\n", ss.str().c_str());
	aquire();
}
