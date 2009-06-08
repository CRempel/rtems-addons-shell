/* getall.c Demonstration program for use with the WinSystems PCM-MIO Linux driver
*
*	$Header$
*
*	$Id$
*
*	$Log$
*
*	Compile with :
*
*			gcc -static getall.c mio_io.o -o getall
*/

#include <stdio.h>
#include <stdlib.h>
#include "mio_io.h"

/* This program demonstrates usage of the adc_convert_all_channels function call.
   This allows for all 16 channels to be quickly converted and the results passed
   back in an 16 element array. Note that this function does not return voltage values,
   it returns raw 16-bit data directly from the converter.
*/


/* This array will receive the result values for all 16 channels */

unsigned short values[16];

int main(void)
{
int channel;
unsigned short result;
float current;

		/* We set the mode on all 16 channels to single-ended bipolar +/- 10V scale.
		   This allows for any device legal input voltages.
		*/
		
		for(channel=0; channel < 16; channel++)
		{			
			adc_set_channel_mode(channel,ADC_SINGLE_ENDED,ADC_BIPOLAR,ADC_TOP_10V);
			
			/* Check for an error by loooking at mio_error_code */
			
			if(mio_error_code)
			{
				/* If an error occurs, print out the string and exit */

				printf("%s - Aborting\n",mio_error_string);
				exit(1);
			}
		}
		
		/* This is it! When this function returns the values from all 8 channels
			   will be present in the values array.
		*/
			
		adc_convert_all_channels(values);
			
		/* Check for possible errors */
			
		if(mio_error_code)
		{
			printf("%s - Aborting\n",mio_error_string);
			exit(1);
		}	
			
		/* Now we'll extract the data, convert it to volts, and display the results */
			
		for(channel =0; channel <16; channel++)
		{
			/* This is for print formatting */

			if(channel == 4 || channel == 8 || channel == 12)
				printf("\n");
			
			/* Get a result from the array */
				
			result = values[channel];
			
			/* Convert the raw value to voltage */

			current = adc_convert_to_volts(channel,result);

			/* Display the result */
						
			printf("CH%2d%8.4f | ",channel,current);
		}
		
		printf("\n\n");
}
