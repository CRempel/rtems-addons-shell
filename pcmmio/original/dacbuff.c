/* dacbuff.c Demonstration program for WinSystems PCM-MIO Driver */

/*
*
* $Id$
*
*
* Compile for GNU/Linux with:
*
*     gcc dacbuff.c kbhit.c mio_io.o -o dacbuff
*
*
* This program demonstrates the adc_buffered_channel_conversions function
* which allows for a programmed sequence of conversions to be accomplished
* with a single call.
*/

#include "mio_io.h"    
#include <stdio.h>
#include <stdlib.h>

// This array will store the results of 2000 conversions

unsigned char commands[16385];
unsigned short values[16384];

void init_keyboard(void);
void close_keyboard(void);
int kbhit(void);
int readch(void);


int main(int argc, char* argv[])
{
unsigned x;


        // Set all 8 DAC outputs to a known span +/- 10V

        for(x=0; x<8; x++)
		{
			set_dac_span(x, DAC_SPAN_BI10);
			if(mio_error_code)
			{
				printf("\n%s\n",mio_error_string);
				exit(1);
			}
		}


        // For this program we are going to use only channel 0
        //   with 16384 updates.
        

        // The data will step up from -10V to +10V in 4/65536 increments

        for(x = 0; x < 16384; x++)
        {
            commands[x] = 0;
            values[x] = x * 4;
        }

        /* We need to terminate the command so that it knows when its done */

        commands[16384] = 0xff;

 
        /* This program runs until a key is pressed. It prints nothing
           on the screen to keep from slowing it down. To see the results
           it would be necessary to attach an oscilloscope to DAC channel
           0.
        */

		init_keyboard();

		printf("DACBUFF running - press any key to exit\n");

        while(!kbhit())
        {

            /* This command returns when all 16,384 samples have been
               sent to the DAC as fast as possible
            */

            buffered_dac_output(commands,values);
			if(mio_error_code)
			{
				printf("\n%s\n",mio_error_string);
				exit(2);
			}

 
        }

        readch();
		close_keyboard();
		return 0;
}

