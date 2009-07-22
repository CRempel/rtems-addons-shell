/*
 *  pcmmio_irq command
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

#include "pcmmio_commands.h"
#include "mio_io.h"
#include <rtems/stringto.h>

#define __need_getopt_newlib
#include <getopt.h>

char pcmmio_irq_usage[] =
  "Usage: %s [-i iterations] [-p period] [-v] [-d|-D DAC|-a ADC]\n"
  "Where: maximum iterations defaults to 1\n"
  "       the period is in milliseconds and defaults to 1000\n";

#define PRINT_USAGE() \
   printf( pcmmio_irq_usage, argv[0] )

int main_pcmmio_irq(int argc, char **argv)
{
  int                 milliseconds;
  int                 maximum;
  int                 iterations;
  int                 sc;
  char                ch;
  bool                changed;
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

  /*
   * Parse arguments here
   */
  milliseconds = 1000;
  maximum = 1;
  verbose = false;

  memset(&getopt_reent, 0, sizeof(getopt_data));
  while ((ch = getopt_r(argc, argv, "i:p:vdD:a:", &getopt_reent)) != -1) {
    switch (ch) {
      case 'i': /* maximum iterations */
        s = getopt_reent.optarg;
        if ( !rtems_string_to_int( s, &maximum, NULL, 0 ) ) {
          printf( "Maximum iterations (%s) is not a number\n", s );
          PRINT_USAGE();
          return -1;
        }

        break;
      case 'p': /* sampling period */
        s = getopt_reent.optarg;
        if ( !rtems_string_to_int( s, &milliseconds, NULL, 0 ) ) {
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
        if ( !rtems_string_to_int( s, &dac, NULL, 0 ) ) {
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
        if ( !rtems_string_to_int( s, &adc, NULL, 0 ) ) {
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
        printf( pcmmio_irq_usage, argv[0] );
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
  changed = false;

  iterations = 1;
  while (1) {
   
    if ( do_din == true ) {
      sc = wait_dio_int_with_timeout(milliseconds);
    } else if ( do_dac == true ) {
      sc = wait_dac_int_with_timeout(dac, milliseconds);
    } else if ( do_adc == true ) {
      sc = wait_dac_int_with_timeout(adc, milliseconds);
    }

    #ifdef TESTING
      fprintf( stderr, "%d ", sc );
    #endif
    if (iterations++ >= maximum )
      break;

  }
  fprintf( stderr, "\n" );
  return 0;
}

rtems_shell_cmd_t Shell_PCMMIO_IRQ_Command = {
  "pcmmio_irq",                                    /* name */
  "Wait for PCMMIO Interrupts",                    /* usage */
  "pcmmio",                                        /* topic */
  main_pcmmio_irq,                                 /* command */
  NULL,                                            /* alias */
  NULL                                             /* next */
};
