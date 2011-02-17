/*
 *  pcmmio_adc command
 *
 *  COPYRIGHT (c) 1989-2011.
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
  float *adc,
  int    channel
)
{
  struct timespec ts;

  (void) rtems_clock_get_uptime( &ts );
  printf( "%03ld:%06ld ", ts.tv_sec, ts.tv_nsec/1000 );

  if ( channel == -1 ) {
    printf(
      "%7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n" 
      "%11s%7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n",
      adc[0], adc[1], adc[2], adc[3], adc[4], adc[5], adc[6], adc[7],
      "", adc[8], adc[9], adc[10], adc[11], adc[12], adc[13], adc[14], adc[15]
    );
  } else {
    printf( "%7.4f\n", adc[channel] );
  }
}

char pcmmio_adc_usage[] =
  "Usage: %s [-i iterations] [-p period] [-v] [channel]\n"
  "Where: maximum iterations defaults to 1\n"
  "       the period is in milliseconds and defaults to 1000\n";

#define PRINT_USAGE() \
   printf( pcmmio_adc_usage, argv[0] )

int main_pcmmio_adc(int argc, char **argv)
{
  int                 milliseconds;
  int                 maximum;
  int                 iterations;
  int                 channel;
  int                 i;
  char                ch;
  bool                changed;
  bool                verbose;
  struct getopt_data  getopt_reent;
  float               adc_last[16];
  float               adc_current[16];
  const  char        *s;

  /*
   * Parse arguments here
   */
  milliseconds = 1000;
  maximum      = 1;
  channel      = -1;
  verbose      = false;

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

  /*
   *  Is there a specific ADC we are interested in?
   */
  i = getopt_reent.optind;
  if ( i < argc ) {
    if ( rtems_string_to_int( argv[i], &channel, NULL, 0 ) ) {
      printf( "ADC (%s) is not a number\n", argv[i] );
      return -1;
    }
    if ( channel < 0 || channel > 15 ) {
      puts( "ADC number must be 0-15" );
      return -1;
    }
  }

  /*
   *  Now sample in the loop
   */
  changed = false;

  iterations = 1;
  while (1) {
    pcmmio_adc_read(adc_current);
   
    if ( iterations == 1 ) {
      changed = true;
    } else if ( channel == -1 ) {
      if ( memcmp( adc_last, adc_current, sizeof(adc_current) ) )
	changed = true;
    } else if ( adc_last[channel] != adc_current[channel] ) {
       changed = true;
    } 

    if ( verbose || changed ) {
      pcmmio_adc_printf(adc_current, channel);
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

