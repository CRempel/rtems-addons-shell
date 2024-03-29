/* poll.c
 * Demonstration program for WinSystems PCM-MIO Driver
 *
 * $Id$
 */

/* This program demonstrates one manner in which an unprivileged 
 * application running in user space can synchronize to hardware events 
 * handled by a device driver running in Kernel space.
 */

#include "mio_io.h"    

#include <stdio.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdlib.h>
#include <pthread.h>


/* This function will be a sub-processes using the Posix threads 
   capability of Linux. This thread will simulate a type of 
   Interrupt service routine in that it will start up and then suspend 
   until an interrupt occurs and the driver awakens it.
*/

void *thread_function(void *arg);

/* Event count, counts the number of events we've handled */

volatile int event_count;
volatile int exit_flag = 0;

char line[80];

main(int argc, char *argv[])
{
int res, res1;
pthread_t a_thread;
int c;
int x;

		/* Do a read_bit to test for port/driver availability */

	  c = dio_read_bit(1);
	  if(mio_error_code)
	  {
		  printf("%s\n",mio_error_string);
		  exit(1);
	  }


	/* Here, we'll enable all 24 bits for falling edge interrupts on both 
     chips. We'll also make sure that they're ready and armed by 
     explicitly calling the clr_int() function.
	*/

    for(x=1; x < 25; x++)
    {
        dio_enab_bit_int(x,FALLING);
		dio_clr_int(x);
    }


    /* We'll also clear out any events that are queued up within the 
       driver and clear any pending interrupts
    */

	enable_dio_interrupt();
	
	if(mio_error_code)
	{
		printf("%s\n",mio_error_string);
		exit(1);
	}

    while((x= dio_get_int()))
    {
		printf("Clearing interrupt on Chip 1 bit %d\n",x);
		dio_clr_int(x);
    }


    /* Now the sub-thread will be started */

    printf("Splitting off polling process\n");

    res = pthread_create(&a_thread,NULL,thread_function,NULL);

    if(res != 0)
    {
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
    }


    /* The thread is now running in the background. It will execute up
      to the point were there are no interrupts and suspend. We as its
      parent continue on. The nice thing about POSIX threads is that we're 
      all in the same data space the parent and the children so we can 
      share data directly. In this program we share the event_count 
      variable.
   */


    /* We'll continue on in this loop until we're terminated */

    while(1)
    {
	/* Print Something so we know the foreground is alive */

		printf("**");


	/* The foreground will now wait for an input from the console
	   We could actually go on and do anything we wanted to at this 
	   point.
        */

		fgets(line,75,stdin);

		if(line[0] == 'q' || line[0] == 'Q')
			break;

	/* Here's the actual exit. If we hit 'Q' and Enter. The program
	   terminates.
	*/

    }


    /* This flag is a shared variable that the children can look at to
	know we're finished and they can exit too.
    */

    exit_flag = 1;

	disable_dio_interrupt();

    /* Display our event count total */

    printf("Event count = %05d\r",event_count);

    printf("\n\nAttempting to cancel subthread\n");
    
    /* If out children are not in a position to see the exit_flag, we
       will use a more forceful technique to make sure they terminate with
       us. If we leave them hanging and we try to re-run the program or
       if another program wants to talk to the device they may be locked
       out. This way everything cleans up much nicer.
    */

    pthread_cancel(a_thread);
    printf("\nExiting Now\n");

    fflush(NULL);

}

    /* This is the the sub-procesc. For the purpose of this
       example, it does nothing but wait for an interrupt to be active on
       chip 1 and then reports that fact via the console. It also
       increments the shared data variable event_count.
    */

void *thread_function(void *arg)
{
int c;

	while(1)
	{
		/* Test for a thread cancellation signal */

	    pthread_testcancel();

		/* Test the exit_flag also for exit */

	    if(exit_flag)
			break;

	    /* This call will put THIS process to sleep until either an
	       interrupt occurs or a terminating signal is sent by the 
	       parent or the system.
            */
	    c = wait_dio_int();

	    /* We check to see if it was a real interrupt instead of a
	       termination request.
	    */
	    
		if(c > 0)
	    {
		    printf("Event sense occured on bit %d\n",c);
		    ++event_count;
	    }
	    else
		break;
	}
}

