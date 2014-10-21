#include "doxygenDemo.h"
#include "stdio.h"


/*------------------------------------------------------------------------------------------------*/

/**
 * C'tor: setzt alle vars.
 *
 * sonst wird nichts gemacht. Kommentare im header haben vorrang. Also
 * aufpassen, dass man nicht aus versehen mehrere Kommentare hat.
 */
DoxygenDemo::DoxygenDemo()
	: counter(10)
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 * docs in cpp.
 */
void DoxygenDemo::print()
{
	printf("%d", counter);
}
