/*
 *  pcmmio_adc command
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

#define __need_getopt_newlib
#include <getopt.h>

#if defined(TESTING)
  #define adc_get_channel_voltage(_x) (_x + 1.5)
#endif

void pcmmio_adc_read(
  float *adc
)
{
  int   input;
  
  for ( input=0 ; input<16 ; input++ ) {
    adc[input] = adc_get_channel_voltage(input);
  }
}

void pcmmio_adc_printf(
  float *adc
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

char pcmmio_adc_usage[] =
  "Usage: %s [-i iterations] [-p period] [-v]\n"
  "Where: maximum iterations defaults to 1\n"
  "       the period is in milliseconds and defaults to 1000\n";

int main_pcmmio_adc(int argc, char **argv)
{
  int    milliseconds;
  int    maximum;
  int    iterations;
  char   ch;
  bool   changed;
  bool   verbose;
  struct getopt_data getopt_reent;
  float  adc_last[16];
  float  adc_current[16];

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
        maximum = rtems_shell_str2int( getopt_reent.optarg );
        break;
      case 'p': /* sampling period */
        milliseconds = rtems_shell_str2int( getopt_reent.optarg );
        break;
      case 'v': /* verbose*/
        verbose = true;
        break;
      default:
        printf( pcmmio_adc_usage, argv[0] );
        return -1;
    }
  }

  if ( maximum != 1 )
    printf(
      "Polling analog inputs for %d iterations with %d msec period\n",
      maximum,
      milliseconds
    );

  /*
   *  Now sample in the loop
   */
  changed = false;

  iterations = 1;
  while (1) {
    pcmmio_adc_read(adc_current);
   
    if ( iterations == 1 )
      changed = true;
    else if ( memcmp( adc_last, adc_current, sizeof(adc_current) ) )
      changed = true;
    
    if ( verbose || changed ) {
      pcmmio_adc_printf(adc_current);
      memcpy( adc_last, adc_current, sizeof(adc_current) );
      changed = false;
    }

    if (iterations++ >= maximum )
      break;

    (void) rtems_task_wake_after( RTEMS_MILLISECONDS_TO_TICKS(milliseconds) );
  }
  return 0;
}

rtems_shell_cmd_t Shell_PCMMIO_ADC_Command = {
  "pcmmio_adc",                                    /* name */
  "Read PCMMIO Analog Inputs",                     /* usage */
  "pcmmio",                                        /* topic */
  main_pcmmio_adc,                                 /* command */
  NULL,                                            /* alias */
  NULL                                             /* next */
};

rtems_shell_alias_t Shell_PCMMIO_ADC_Alias = {
  "pcmmio_adc",          /* command */
  "adc"                  /* alias */
};

