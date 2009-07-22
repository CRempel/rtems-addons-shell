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
#include <rtems/string2.h>

#include <stdlib.h>

#if defined(TESTING)
  #define set_dac_voltage(_dac, _voltage) \
    printf( "Testing: Write %6.4f to to dac %d\n", _voltage, _dac );
#endif

char pcmmio_dac_usage[] =
  "Usage: %s dac voltage\n"
  "Where: dac must be 0-7\n"
  "       voltage must be -10V to +10V\n";

#define PRINT_USAGE() \
   printf( pcmmio_dac_usage, argv[0] );

int main_pcmmio_dac(int argc, char **argv)
{
  int    dac;
  float  voltage;
  bool   fail = false;

  /*
   *  Verify that we have the right number of arguments.
   */
  if ( argc != 3 ) {
    printf( "Incorrect number of arguments\n" );
    PRINT_USAGE();
    return -1;
  }

  /*
   *  Convert the string arguments into number values
   */
  if ( !rtems_string_to_int( argv[1], &dac, NULL, 0 ) ) {
    printf( "DAC (%s) is not a number\n", argv[1] );
    fail = true;
  }

  if ( !rtems_string_to_float( argv[2], &voltage, NULL ) ) {
    printf( "Voltage (%s) is not a number\n", argv[2] );
    fail = true;
  }

  /*
   *  Validate the output dac and voltage.
   */
  if ( dac < 0 || dac > 7 ) {
    puts( "DAC number must be 0-7" );
    fail = true;
  }

  if ( voltage < -10.0 || voltage > 10.0 ) {
    printf( "Voltage must be between -10.0V and +10.0V\n" );
    fail = true;
  }

  if ( fail ) {
    PRINT_USAGE();
    return -1;
  }

  /*
   *  Now write the voltage
   */
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

