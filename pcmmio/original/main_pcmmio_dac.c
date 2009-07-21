/*
 *  pcmmio_dac command
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
#include <stdlib.h>

#if defined(TESTING)
  #define set_dac_voltage(_dac, _voltage) \
    printf( "Testing: Write %6.4f to to dac %d\n", _voltage, _dac );
#endif

char pcmmio_dac_usage[] =
  "Usage: %s [-v] dac voltage\n"
  "Where: dac must be 0-7\n"
  "       voltage must be -10V to +10V\n";

#define PRINT_USAGE() \
   printf( pcmmio_dac_usage, argv[0] );

int main_pcmmio_dac(int argc, char **argv)
{
  char   ch;
  bool   verbose;
  struct getopt_data getopt_reent;
  int    dac;
  float  voltage;

  /*
   * Parse arguments here
   */
  verbose = false;

  memset(&getopt_reent, 0, sizeof(getopt_data));
  while ((ch = getopt_r(argc, argv, "v", &getopt_reent)) != -1) {
    switch (ch) {
      case 'v': /* verbose*/
        verbose = true;
        break;
      default:
        PRINT_USAGE();
        return -1;
    }
  }

  if ( (argc - getopt_reent.optind) != 2 ) {
    printf( "Incorrect number of arguments\n" );
    PRINT_USAGE();
    return -1;
  }

  /*
   *  Convert the string arguments into number values
   */
  dac     = strtof( argv[getopt_reent.optind], NULL );
  voltage = strtof( argv[getopt_reent.optind + 1], NULL );

  /*
   *  Validate the output dac and voltage.
   */
  if ( dac < 0 || dac > 7 ) {
    printf( "DAC number must be 0-7\n" );
    PRINT_USAGE();
    return -1;
  }

  if ( voltage < -10.0 || voltage > 10.0 ) {
    printf( "Voltage must be between -10.0V and +10.0V\n" );
    PRINT_USAGE();
    return -1;
  }

  /*
   *  Now write the voltage
   */
  if ( verbose )
    printf( "Write %6.4f to to dac %d\n", voltage, dac );
  set_dac_voltage(dac, voltage);

  return 0;
}

rtems_shell_cmd_t Shell_PCMMIO_DAC_Command = {
  "pcmmio_dac",                                    /* name */
  "Read PCMMIO Analog Inputs",                     /* usage */
  "pcmmio",                                        /* topic */
  main_pcmmio_dac,                                 /* command */
  NULL,                                            /* alias */
  NULL                                             /* next */
};

rtems_shell_alias_t Shell_PCMMIO_DAC_Alias = {
  "pcmmio_dac",          /* command */
  "dac"                  /* alias */
};

