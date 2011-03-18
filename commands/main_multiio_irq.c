/*
 *  multiio_irq command
 *
 *  COPYRIGHT (c) 1989-2009.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <rtems.h>
#include <rtems/shell.h>
#include <rtems/stringto.h>
#include "multiio.h"

#define __need_getopt_newlib
#include <getopt.h>

char multiio_irq_usage[] =
  "Usage: %s [-i iterations] [-p period] [-v] [-d|-D DAC|-a ADC]\n"
  "Where: maximum iterations defaults to 1\n"
  "       the period is in milliseconds and defaults to 1000\n";

#define PRINT_USAGE() \
   printf( multiio_irq_usage, argv[0] )

static int to_usecs(
  struct timespec *start,
  struct timespec *end
)
{
  int usecs;
  struct timespec took;

  _Timespec_Subtract( start, end, &took );
  usecs = _Timespec_Get_seconds( &took ) * 1000000;
  usecs += _Timespec_Get_nanoseconds( &took ) / 1000;
  return usecs;
}

int main_multiio_irq(int argc, char **argv)
{
  int                 milliseconds;
  int                 maximum;
  int                 iterations;
  int                 sc;
  char                ch;
  bool                verbose;
  struct getopt_data  getopt_reent;
  const  char        *s;
  bool                do_dac = false;
  bool                do_adc = false;
  bool                do_din = false;
  int                 dac = -1;
  int                 adc = -1;
  int                 selected;
  const char         *irq = "";
  int                 elapsed;
  int                 interrupts;
  struct timespec     previousTimestamp;
  struct timespec     timestamp;

  /*
   * Parse arguments here
   */
  milliseconds = 1000;
  maximum = 1;
  verbose = false;
  _Timespec_Set_to_zero( &previousTimestamp );

  memset(&getopt_reent, 0, sizeof(getopt_data));
  while ((ch = getopt_r(argc, argv, "i:p:vdD:a:", &getopt_reent)) != -1) {
    switch (ch) {
      case 'i': /* maximum iterations */
        s = getopt_reent.optarg;
        if ( rtems_string_to_int( s, &maximum, NULL, 0 ) ) {
          printf( "Maximum iterations (%s) is not a number\n", s );
          PRINT_USAGE();
          return -1;
        }

        break;
      case 'p': /* sampling period */
        s = getopt_reent.optarg;
        if ( rtems_string_to_int( s, &milliseconds, NULL, 0 ) ) {
          printf( "Sampling period (%s) is not a number\n", s );
          PRINT_USAGE();
          return -1;
        }
        if ( milliseconds == 0 ) {
          printf( "Sampling period (%d) is 0\n", milliseconds );
          PRINT_USAGE();
          return -1;
        }
        break;
      case 'd': /* DIN enable */
        do_din = true;
        break;
      case 'D': /* DAC enable */
        s = getopt_reent.optarg;
        if ( rtems_string_to_int( s, &dac, NULL, 0 ) ) {
          printf( "DAC (%s) is not a number\n", s );
          PRINT_USAGE();
          return -1;
        }

	if ( dac < 0 || dac > 7 ) {
	  puts( "DAC number must be 0-7" );
          PRINT_USAGE();
          return -1;
	}

        do_dac = true;
        break;
      case 'a': /* ADC enable */
        s = getopt_reent.optarg;
        if ( rtems_string_to_int( s, &adc, NULL, 0 ) ) {
          printf( "ADC (%s) is not a number\n", s );
          PRINT_USAGE();
          return -1;
        }

	if ( adc < 0 || adc > 7 ) {
	  puts( "ADC number must be 0-7" );
          PRINT_USAGE();
          return -1;
	}
        do_adc = true;
        break;
      case 'v': /* verbose */
        verbose = true;
        break;
      default:
        printf( multiio_irq_usage, argv[0] );
        return -1;
    }
  }

  /*
   *  Did they select one item and ONLY one item?
   */
  selected = 0;
  if ( do_din == true ) selected++;
  if ( do_dac == true ) selected++;
  if ( do_adc == true ) selected++;

  if ( selected == 0 ) {
    puts( "No IRQ Sources selected" );
    return -1;
  }

  if ( selected > 1 ) {
    puts( "More than 1 IRQ Sources selected" );
    return -1;
  }

  if ( do_din == true ) {
    irq = "DIN";
  } else if ( do_dac == true ) {
    irq = "DAC";
  } else if ( do_adc == true ) {
    irq = "ADC";
  }
  if ( maximum != 1 )
    printf(
      "Polling for %s IRQ for %d iterations with %d msec period\n",
      irq,
      maximum,
      milliseconds
    );

  /*
   *  Now sample in the loop
   */
  elapsed    = 0;
  iterations = 1;
  interrupts = 0;

  rtems_din_flush_buffered_interrupts();
  while (1) {
    sc = 0;
   
    if ( do_din == true ) {
      sc = rtems_din_wait_interrupt_with_timestamp(milliseconds, &timestamp);
    } else if ( do_dac == true ) {
      ; /* sc = wait_dac_int_with_timeout(dac, milliseconds); */
    } else if ( do_adc == true ) {
      ; /* sc = wait_dac_int_with_timeout(adc, milliseconds); */
    }

    if ( sc != -1 ) {
      interrupts++;
      if ( do_din == true ) {
        printf(
          "%d %s irq pin %d @ %ld:%ld (%d usecs since last)\n",
          elapsed,
          irq,
          sc - 1,
          timestamp.tv_sec, timestamp.tv_nsec,
          to_usecs( &previousTimestamp, &timestamp )
        );
        previousTimestamp = timestamp;
      } else {
        printf( "%d %s irq\n", elapsed, irq );
      }
    }

    elapsed += milliseconds;
    if (iterations++ >= maximum )
      break;
  }
  printf(
    "%d total interrupts from %s in %d milliseconds\n",
    interrupts,
    irq,
    elapsed
  );
  return 0;
}

rtems_shell_cmd_t Shell_MULTIIO_IRQ_Command = {
  "multiio_irq",                                /* name */
  "Wait for DIN Interrupts",                    /* usage */
  "multiio",                                    /* topic */
  main_multiio_irq,                             /* command */
  NULL,                                         /* alias */
  NULL                                          /* next */
};
