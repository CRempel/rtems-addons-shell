/* buffered.c Demonstration program for WinSystems PCM-MIO Driver */

/*
*
* $Id$
*
* Compile for GNU/Linux with:
*
*   gcc buffered.c kbhit.c mio_io.o -o buffered -lpthread
*
*
*  This program demonstrates the adc_buffered_channel_conversions function
*  which allows for a programmed sequence of conversions to be accomplished
*  with a single call.
*/

/* Our IOCTL definitions and all function prototypes */

#include "mio_io.h"    

#include <stdio.h>
#include <stdlib.h>

/* This array will hold 2000 channel numbers plus a terminating 0xff charcter */

unsigned char to_do_channels[2001];

/* This array will store the results of 2000 conversions */

unsigned short values[2000];

/* Keyboard support function prototypes */

void init_keyboard(void);
void close_keyboard(void);
int kbhit(void);
int readch(void);


int main(int argc, char* argv[])
{
int channel = 0;
unsigned short result;
float current;
unsigned long count = 0;
int x;

	/* Set up all 16 channels for the +/- 10 Volt range */

	for(channel =0; channel < 16; channel ++)
	{
		adc_set_channel_mode(channel,ADC_SINGLE_ENDED,ADC_BIPOLAR,ADC_TOP_10V);
		if(mio_error_code)
		{
			printf("\nError occured - %s\n",mio_error_string);
			exit(1);
		}
	}

	/* We'll fill the to_do list with the four different channels 500
	  each successively.
    */

	for(x=0; x < 500; x++)
	{
		to_do_channels[x] = 0;
		to_do_channels[x+500] = 1;
		to_do_channels[x+1000] = 2;
		to_do_channels[x+1500] = 3;

	}

	/* Load the "terminator" into the last position */ 

	to_do_channels[2000] = 0xff;

	/*  We'll keep going until a key is pressed */

	init_keyboard();

	while(!kbhit())
	{
		/* Start up the conversions. This function returns when all 2000 of
	     our conversions are complete.
	    */

		adc_buffered_channel_conversions(to_do_channels,values);
		
		count += 2000;

		if(mio_error_code)
		{
			printf("\nError occured - %s\n",mio_error_string);
			exit(1);
		}

		/* We'll extract our data from the "values" array. In order to make the
		 display more readable, we'll take a value from each channel display them,
		 and move to the next result.
		*/

		for(x=0; x < 500; x++)
		{
			printf("%08ld  ",count);

			/* Get the raw data */

			result = values[x];

			/* Convert to voltage */

			current = adc_convert_to_volts(0, result);

			/* Display the value */

			printf("CH0 %9.5f ",current);

			/* Repeat for channels 1 - 3  */

			result = values[x+500];
			current = adc_convert_to_volts(1, result);
			printf("CH1 %9.5f ",current);

			result = values[x+1000];
			current = adc_convert_to_volts(2, result);
			printf("CH2 %9.5f ",current);
			
			result = values[x+1500];
			current = adc_convert_to_volts(3, result);
			printf("CH3 %9.5f ",current);
			printf("\r");
		}
	}
	readch();
	printf("\n\n");
	close_keyboard();
	return 0;
}
