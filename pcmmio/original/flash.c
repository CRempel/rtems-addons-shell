/* Flash.c WinSystems PCM-MIO driver Demonstration Program
*
*  $Id$
*
*  Compile for GNU/Linux with:
*
*    gcc flash.c kbhit.c mio_io.o -o flash
*
*  This very simple program, toggles each of the 48  DIO lines in succession,
*  holding it low for 250ms and then releasing it.
* 
*/


/* These function prototypes are for the keyboard support function kbhit() */

void init_keyboard(void);
void close_keyboard(void);
int kbhit(void);
int readch(void);


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mio_io.h"

main(int argc, char *argv[])
{
int x;

	x = dio_read_bit(1);	/* Just a test for availability */
	if(mio_error_code)
	{

		/* Print the error and exit, if one occurs */

		printf("\n%s\n",mio_error_string);
		exit(1);
	}


	printf("Flashing - Press any key to exit\n");

	init_keyboard();
	while(!kbhit())
	{
		for(x=1; x<=48; x++)
		{
			dio_set_bit(x);	/* Turn on the LED */

			/* Ideally, we should check mio_error_code after all calls. Practically, there's little to 
			   go wrong once we've validated the driver presence.
		    */


			/* Got to sleep for 250ms */

			usleep(25000);

			dio_clr_bit(x);	/* Turn off the LED */
		}
	}
	readch();
	close_keyboard();
	printf("\n");
}


