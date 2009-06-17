/* getvolt.c WinSystems PCM-MIO driver Demonstration Program
 *
 *  $Id$
 *
 * Compile for GNU/Linux with:
 *
 *
 *   gcc getvolt.c mio_io.o -o getvolt
 *
 *
 * This program demonstrates the adc_auto_get_channel_voltage
 * function. It reads the voltage on the channel specified
 * prints the value and then exits.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mio_io.h"

main(int argc, char *argv[])
{
int channel = 0;
float result;


	/* If an argument is present,we'll use is as the channel
	   number. Otherwise we'll default to channel 0
   */

	if(argc > 1)
		channel = atoi(argv[1]);

	/* We'll let the driver validate the channel number */

	result = adc_auto_get_channel_voltage(channel);

	/* Check for an error */

	if(mio_error_code)
	{

		/* If an error occured. Display the error and exit */

		printf("%s\n",mio_error_string);
		exit(1);
	}

	/* Print the results */

	printf(" Channel %d  =  %9.4f\n",channel,result);
}


