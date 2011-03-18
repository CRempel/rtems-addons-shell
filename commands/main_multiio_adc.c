/*
 *  adc command
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

static void adc_read(
  float *adc,
  int    max
)
{
  int   input;
  
  for ( input=0 ; input<max ; input++ ) {
    adc[input] = rtems_adc_get_channel_voltage(input);
  }
}

static void adc_printf(
  float *adc,
  int    max
)
{
  struct timespec ts;

  (void) rtems_clock_get_uptime( &ts );
  printf(
    "%03ld:%06ld %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n" 
    "%11s%7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n",
    ts.tv_sec, ts.tv_nsec/1000,
    adc[0], adc[1], adc[2], adc[3], adc[4], adc[5], adc[6], adc[7],
    "", adc[8], adc[9], adc[10], adc[11], adc[12], adc[13], adc[14], adc[15]
  );
}

char static adc_usage[] =
  "Usage: %s [-i iterations] [-p period] [-v]\n"
  "Where: maximum iterations defaults to 1\n"
  "       the period is in milliseconds and defaults to 1000\n";

#define PRINT_USAGE() \
   printf( adc_usage, argv[0] )

int main_multiio_adc(int argc, char **argv)
{
  int                 milliseconds;
  int                 maximum;
  int                 iterations;
  char                ch;
  bool                changed;
  bool                verbose;
  struct getopt_data  getopt_reent;
  float               *adc_last = NULL;
  float               *adc_current = NULL;
  int                  adcs;
  const  char         *s;

  /*
   * Parse arguments here
   */
  milliseconds = 1000;
  maximum = 1;
  verbose = false;

  memset(&getopt_reent, 0, sizeof(getopt_data));
  while ((ch = getopt_r(argc, argv, "i:p:v", &getopt_reent)) != -1) {
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
      case 'v': /* verbose*/
        verbose = true;
        break;
      default:
        PRINT_USAGE();
        return -1;
    }
  }

  if ( maximum != 1 ) {
    printf(
      "Polling analog inputs for %d iterations with %d msec period\n",
      maximum,
      milliseconds
    );
  }
  
  adcs        = rtems_adc_get_maximum();
  adc_last    = calloc( 1, adcs * sizeof(float) );
  adc_current = calloc( 1, adcs * sizeof(float) );

  /*
   *  Now sample in the loop
   */
  changed = false;

  iterations = 1;
  while (1) {
    adc_read(adc_current, adcs);
   
    if ( iterations == 1 )
      changed = true;
    else if ( memcmp( adc_last, adc_current, sizeof(adc_current) ) )
      changed = true;
    
    if ( verbose || changed ) {
      adc_printf(adc_current, adcs);
      memcpy( adc_last, adc_current, sizeof(adc_current) );
      changed = false;
    }

    if (iterations++ >= maximum )
      break;

    (void) rtems_task_wake_after( RTEMS_MILLISECONDS_TO_TICKS(milliseconds) );
  }
  return 0;
}

rtems_shell_cmd_t Shell_MULTIIO_ADC_Command = {
  "multiio_adc",                            /* name */
  "Read Analog Inputs",                     /* usage */
  "multiio",                                /* topic */
  main_multiio_adc,                         /* command */
  NULL,                                     /* alias */
  NULL                                      /* next */
};

rtems_shell_alias_t Shell_MULTIIO_ADC_Alias = {
  "multiio_adc",         /* command */
  "adc"                  /* alias */
};

