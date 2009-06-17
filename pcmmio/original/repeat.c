/* repeat.c
 * Demonstration program for WinSystems PCM-MIO Driver
 *
 * $Id$
 *
 * Compile for GNU/Linux with:
 *
 *    gcc repeat.c kbhit.c mio_io.o -o repeat
 *
 * This program demonstrates the adc_convert_single_repeated function
 * which allows for a sequence of conversions to be accomplished
 * with a single call on the same channel.
 */

#include "mio_io.h"    

#include <stdio.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdlib.h>
#include <pthread.h>

// This array will store the results of 2000 conversions

unsigned short values[2000];
volatile unsigned long count = 0;

void *thread_function(void *arg);
void *thread_function2(void *arg);

/* Event count, counts the number of events we've handled */

volatile int exit_flag = 0;

void init_keyboard(void);
void close_keyboard(void);
int kbhit(void);
int readch(void);


int main(int argc, char* argv[])
{
int channel = 0;
unsigned short result;
float min,max,current;
int x,c;
int res;
int res2;
pthread_t a_thread;
pthread_t b_thread;

	// We'll keep track of the minimum and maximum voltage values
	// we see on a channel as well as the count of conversions
	// completed.

	max = -10.0;
	min	= 10.0;
	count = 0;

	// The default channel is 0 but an alternate channel can be specified
	// as a command line argument.

	if(argc > 1)
		channel = atoi(argv[1]);

	// Check for a valid channel number. Abort if bad

	if(channel <0 || channel > 15)
	{
		printf("Channel numbers must be between 0 and 15 - Aborting\n");
		exit(0);
	}

	// This call sets the mode for the specified channel. We are going to
	// set up for single-ended +/- 10 V range. That way any legal input 
	// can be accomodated.

	adc_set_channel_mode(channel,ADC_SINGLE_ENDED,ADC_BIPOLAR,ADC_TOP_10V);

	if(mio_error_code)
	{
		printf("\nError occured - %s\n",mio_error_string);
		exit(1);
	}

	/* Enable interrupts on both controllers */

	enable_adc_interrupt(0);
	enable_adc_interrupt(1);

	if(mio_error_code)
	{
		printf("\nError occured - %s\n",mio_error_string);
		exit(1);
	}


    res = pthread_create(&a_thread,NULL,thread_function,NULL);

    if(res != 0)
    {
		perror("Thread 1 creation failed");
		exit(EXIT_FAILURE);
    }


    res2 = pthread_create(&b_thread,NULL,thread_function2,NULL);

    if(res != 0)
    {
		perror("Thread 2 creation failed");
		exit(EXIT_FAILURE);
    }



	init_keyboard();

	while(1)
	{
			// We'll keep running until a recognized key is pressed 
		if(kbhit())
		{
				c = readch();

				// The 'C' key clears the min/max and count values

				if(c== 'c' || c == 'C')
				{
					count = 0;
					min = 10.0;
					max = -10.0;
				}

				// The 'N' key moves to the next channel, wrapping from 15
				// back to 0 when appropriate.

				else if(c == 'n' || c == 'N')
				{
					printf("\n");
					channel++;
					if(channel > 15)
						channel = 0;

					// When we change channels we need to make sure we set
					// the channel's mode to a valid range.

					adc_set_channel_mode(channel,ADC_SINGLE_ENDED,ADC_BIPOLAR,ADC_TOP_10V);
					if(mio_error_code)
					{
						printf("\nError occured - %s\n",mio_error_string);
						exit(1);
					}

					// A new channel also clears the count and min/max values.

					count = 0;
					min = 10.0;
					max = -10.0;
				}
				else
				{
					disable_adc_interrupt(0);
					disable_adc_interrupt(1);
				    pthread_cancel(a_thread);
					exit_flag = 1;
					close_keyboard();
					printf("\n\n");
					exit(0);
				}
		}

		// Finally the real thing. This function-call results in 2000
		// conversions on the specified channel with the results going into
		// a buffer called "values".

		adc_convert_single_repeated(channel,2000,values);

		if(mio_error_code)
		{
			printf("\nError occured - %s\n",mio_error_string);
			exit(1);
		}

		// Bump up the count

;		count += 2000;

		// Now we'll read out the 2000 conversion values. Convert them to 
		// floating point voltages and set the min and max values as appropriate
		
		for(x=0; x<2000; x++)
		{
			result = values[x];
			
			current = adc_convert_to_volts(channel,result);

			// Check and load the min/max values as needed

			if(current < min)
				min = current;

			if(current > max)
				max = current;

			// Print the values

			printf("CH %02d %09ld %9.5f Min =%9.5f Max =%9.5f\r",channel,count,current,min,max);
		}
	}


	return 0;
}



void *thread_function(void *arg)
{
int c;

	while(1)
	{
	    pthread_testcancel();

	    if(exit_flag)
			break;

	    /* This call will put THIS process to sleep until either an
	       interrupt occurs or a terminating signal is sent by the 
	       parent or the system.
            */
	    c = wait_adc_int(0);

	    /* We check to see if it was a real interrupt instead of a
	       termination request.
	    */
	    if(c == 0)
	    {
#ifdef DEBUG
			printf("Interrupt occured on ADC 0\n");
#endif
			++count;

	    }
	    else
		break;
	}
}



void *thread_function2(void *arg)
{
int c;

	while(1)
	{
	    pthread_testcancel();

	    if(exit_flag)
			break;

	    /* This call will put THIS process to sleep until either an
	       interrupt occurs or a terminating signal is sent by the 
	       parent or the system.
            */
	    c = wait_adc_int(1);

	    /* We check to see if it was a real interrupt instead of a
	       termination request.
	    */
	    if(c == 0)
	    {
#ifdef DEBUG
			printf("Interrupt occured on ADC 1\n");
#endif
			++count;

	    }
	    else
		break;
	}
}

