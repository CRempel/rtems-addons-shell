/* dacout.c WinSystems PCM-MIO driver Demonstration Program
*
*  $Header$
*
*  $Id$
*
*  $Log$
*
*
*  Compile with :
*
*
*	gcc dacout.c mio_io.o -o dacout
*
*
*	This extremely simple program uses the set_dac_voltage function to update
*   the dac output to a specific voltage.
*/

#include <stdio.h>
#include <stdlib.h>
#include "mio_io.h"

main(int argc, char *argv[])
{
int channel = 0;
float voltage;

	/* We must have arguments for channel and voltage */

	if(argc != 3)
	{
		printf("usage : dacout channel voltage\n");
		exit(1);
	}


	/* Channel is the first argument. We'll let the driver
	   check for valid channel numbers just to show how the
	   mio_error_string works
   */
	channel = atoi(argv[1]);

	/* The same goes for the voltage argument. The driver will tell us
	   if the input is out of range
   */

	voltage = atof(argv[2]);

	printf("Setting DAC channel %d to %9.5f Volts\n",channel,voltage);

	set_dac_voltage(channel, voltage);


	/* Here's where any problems with the input parameters will be determined.
	   by checking mio_error_code for a non-zero value we can detect error
	   conditions.
   */

	if(mio_error_code)
	{
		/* We'll print out the error and exit */

		printf("%s\n",mio_error_string);
		exit(1);
	}

}


