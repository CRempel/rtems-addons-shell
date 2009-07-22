/*
 *  pcmmio_din command
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
#include <rtems/stringto.h>

#define __need_getopt_newlib
#include <getopt.h>

#if defined(TESTING)
  #define dio_read_bit(_x) (_x & 1)
#endif

void pcmmio_dio_read(
  int *dio
)
{
  int   bit;
  
  for ( bit=0 ; bit<48 ; bit++ ) {
    dio[bit] = dio_read_bit(bit);
  }
}

void pcmmio_dio_printf(
  int *dio
)
{
  struct timespec ts;
  int             bit;

  (void) rtems_clock_get_uptime( &ts );
  printf( "%ld:%ld ", ts.tv_sec, ts.tv_nsec );
  for ( bit=0 ; bit<48 ; bit+=4 ) {
     printf(
	"%d%d%d%d%s",
	dio[bit +  0], dio[bit +  1],
	dio[bit +  2], dio[bit +  3],
        ((bit == 44) ? "\n" : " " )
    );
  }
}

char pcmmio_din_usage[] =
  "Usage: %s [-i iterations] [-p period] [-v]\n"
  "Where: maximum iterations defaults to 1\n"
  "       the period is in milliseconds and defaults to 1000\n";

#define PRINT_USAGE() \
   printf( pcmmio_din_usage, argv[0] )

int main_pcmmio_din(int argc, char **argv)
{
  int                 milliseconds;
  int                 maximum;
  int                 iterations;
  char                ch;
  bool                changed;
  bool                verbose;
  struct getopt_data  getopt_reent;
  int                 dio_last[48];
  int                 dio_current[48];
  const  char        *s;

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
      case 'v': /* verbose*/
        verbose = true;
        break;
      default:
        printf( pcmmio_din_usage, argv[0] );
        return -1;
    }
  }

  if ( maximum != 1 )
    printf(
      "Polling discrete inputs for %d iterations with %d msec period\n",
      maximum,
      milliseconds
    );

  /*
   *  Now sample in the loop
   */
  changed = false;

  iterations = 1;
  while (1) {
    pcmmio_dio_read(dio_current);
   
    if ( iterations == 1 )
      changed = true;
    else if ( memcmp( dio_last, dio_current, sizeof(dio_current) ) )
      changed = true;
    
    if ( verbose || changed ) {
      pcmmio_dio_printf(dio_current);
      memcpy( dio_last, dio_current, sizeof(dio_current) );
      changed = false;
    }

    if (iterations++ >= maximum )
      break;

    (void) rtems_task_wake_after( RTEMS_MILLISECONDS_TO_TICKS(milliseconds) );
  }
  return 0;
}

rtems_shell_cmd_t Shell_PCMMIO_DIN_Command = {
  "pcmmio_din",                                    /* name */
  "Read PCMMIO Discrete Inputs",                   /* usage */
  "pcmmio",                                        /* topic */
  main_pcmmio_din,                                 /* command */
  NULL,                                            /* alias */
  NULL                                             /* next */
};

rtems_shell_alias_t Shell_PCMMIO_DIN_Alias = {
  "pcmmio_din",          /* command */
  "din"                  /* alias */
};

