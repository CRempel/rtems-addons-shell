/*
 *  dout command
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

static char dout_usage[] =
  "Usage: %s bit value\n"
  "Where: bit must be 0-%d\n"
  "       value must be 0 or 1\n";

#define PRINT_USAGE() \
   printf( dout_usage, argv[0], maximum )

int main_multiio_dout(int argc, char **argv)
{
  int    bit;
  int    value;
  bool   fail = false;
  int    maximum = rtems_dout_get_maximum();

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
  if ( bit < 0 || bit > maximum ) {
    printf( "Bit number must be 0-maximum\n" );
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
  rtems_set_dout(bit, value);

  return 0;
}

rtems_shell_cmd_t Shell_MULTIIO_DOUT_Command = {
  "multiio_dout",                           /* name */
  "Write Discrete Outputs",                 /* usage */
  "multiio",                                /* topic */
  main_multiio_dout,                        /* command */
  NULL,                                     /* alias */
  NULL                                      /* next */
};

rtems_shell_alias_t Shell_MULTIIO_DOUT_Alias = {
  "multiio_dout",         /* command */
  "dout"                  /* alias */
};
