/*
 *  pcmmio_dout command
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

#if defined(TESTING)
  #define dio_write_bit(_bit, _value) \
    printf( "Testing: Write %d to to bit %d\n", _value, _bit );
#endif

char pcmmio_dout_usage[] =
  "Usage: %s bit value\n"
  "Where: bit must be 0-47\n"
  "       value must be 0 or 1\n";

#define PRINT_USAGE() \
   printf( pcmmio_dout_usage, argv[0] )

int main_pcmmio_dout(int argc, char **argv)
{
  int    bit;
  int    value;
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
  if ( rtems_string_to_int( argv[1], &bit, NULL, 0 ) ) {
    printf( "Bit (%s) is not a number\n", argv[1] );
    fail = true;
  }

  if ( rtems_string_to_int(argv[2], &value, NULL, 0) ) {
    printf( "Value (%s) is not a number\n", argv[2] );
    fail = true;
  }

  /*
   *  Validate the output bit and value.
   */
  if ( bit < 0 || bit > 47 ) {
    printf( "Bit number must be 0-47\n" );
    fail = true;
  }

  if ( value != 0 && value != 1 ) {
    printf( "Value must be 0 or 1\n" );
    fail = true;
  }

  if ( fail ) {
    PRINT_USAGE();
    return -1;
  }

  /*
   *  Now write the value
   */
  printf( "Write %d to to bit %d\n", value, bit );
  dio_write_bit(bit, value);

  return 0;
}

rtems_shell_cmd_t Shell_PCMMIO_DOUT_Command = {
  "pcmmio_dout",                                   /* name */
  "Write PCMMIO Discrete Outputs",                 /* usage */
  "pcmmio",                                        /* topic */
  main_pcmmio_dout,                                /* command */
  NULL,                                            /* alias */
  NULL                                             /* next */
};

rtems_shell_alias_t Shell_PCMMIO_DOUT_Alias = {
  "pcmmio_dout",         /* command */
  "dout"                 /* alias */
};

