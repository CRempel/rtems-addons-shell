/*
 *  dac command
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

#define VALIDATE_VOLTAGE(_v) \
  if ( (_v) < rtems_dac_get_minimum_voltage() || \
       (_v) > rtems_dac_get_maximum_voltage() ) { \
    printf( "Voltage must be between %fV and %f\n", \
              rtems_dac_get_minimum_voltage(), \
              rtems_dac_get_maximum_voltage()); \
    fail = true; \
  }

static char dac_usage[] =
  "Usage: %s dac voltage\n"
  "       %s dac low high step time_per_step maximum_time\n"
  "\n"
  "Where: dac must be 0-%d\n"
  "       voltages and step must be %fV to %fV\n"
  "       times are in milliseconds\n"
  "  First form is a single write.\n"
  "  Second form writes a pattern.\n";

#define PRINT_USAGE() \
   printf( dac_usage, argv[0], argv[0], \
           rtems_dac_get_maximum(), \
           rtems_dac_get_minimum_voltage(), \
           rtems_dac_get_maximum_voltage()); \

int main_multiio_dac(int argc, char **argv)
{
  int       dac;
  float     low_voltage;
  float     high_voltage;
  float     step_voltage;
  float     current_voltage;
  float     current_step;
  int       step_time;
  int       maximum_time;
  uint32_t  step_ticks;
  bool      fail = false;
  int       elapsed;

  /*
   *  Verify that we have the right number of arguments.
   */
  if ( (argc != 3) && (argc != 7) ) {
    printf( "Incorrect number of arguments\n" );
    PRINT_USAGE();
    return -1;
  }

  /*
   *  Convert the string arguments into number values
   */
  if ( rtems_string_to_int( argv[1], &dac, NULL, 0 ) ) {
    printf( "DAC (%s) is not a number\n", argv[1] );
    fail = true;
  }

  if ( rtems_string_to_float( argv[2], &low_voltage, NULL ) ) {
    printf( "Voltage (%s) is not a number\n", argv[2] );
    fail = true;
  }

  /*
   *  Validate the output dac and voltage.
   */
  if ( dac < 0 || dac > rtems_dac_get_maximum() ) {
    printf( "DAC number must be 0-%d\n", rtems_dac_get_maximum() );
    fail = true;
  }

  VALIDATE_VOLTAGE( low_voltage );

  /*
   *  Now do a single write to the DAC
   */
  if ( argc == 3 ) {
    if ( fail ) {
      PRINT_USAGE();
      return -1;
    }
    printf( "Write %6.4f to to dac %d\n", low_voltage, dac );
    rtems_set_dac_voltage(dac, low_voltage);
    return 0;
  }

  /*
   *  Finish parsing the arguments to do a pattern
   */
  fail = false;

  if ( rtems_string_to_float( argv[3], &high_voltage, NULL ) ) {
    printf( "Voltage (%s) is not a number\n", argv[3] );
    fail = true;
  }

  VALIDATE_VOLTAGE( high_voltage );

  if ( rtems_string_to_float( argv[4], &step_voltage, NULL ) ) {
    printf( "Step voltage (%s) is not a number\n", argv[4] );
    fail = true;
  }

  VALIDATE_VOLTAGE( step_voltage );

  if ( step_voltage < 0.0 ) {
    printf( "Step voltage must be greater than 0\n" );
    fail = true;
  }

  if ( rtems_string_to_int( argv[5], &step_time, NULL, 0 ) ) {
    printf( "Step time (%s) is not a number\n", argv[5] );
    fail = true;
  }

  if ( rtems_string_to_int( argv[6], &maximum_time, NULL, 0 ) ) {
    printf( "Maximum time (%s) is not a number\n", argv[6] );
    fail = true;
  }

  if ( step_time >= maximum_time ) {
    printf(
      "Step time (%d) must be less than maximum time (%d)\n",
      step_time,
      maximum_time
    );
    fail = true;
  }

  if ( step_time < 0 ) {
    printf( "Step time must be greater than 0\n" );
    fail = true;
  }

  if ( maximum_time < 0 ) {
    printf( "Maximum time must be greater than 0\n" );
    fail = true;
  }

  /*
   *  Now write the pattern to the DAC
   */

  if ( fail ) {
    PRINT_USAGE();
    return -1;
  }

  printf(
    "Write %6.4f-%6.4f step=%6.4f stepTime=%d msecs dac=%d max=%d msecs\n",
    low_voltage,
    high_voltage,
    step_voltage,
    step_time,
    dac,
    maximum_time
  );

  elapsed         = 0;
  step_ticks      = RTEMS_MILLISECONDS_TO_TICKS(step_time);
  current_voltage = low_voltage;
  current_step    = step_voltage;

  if ( low_voltage > high_voltage ) 
    current_step    *= -1.0;

  while (1) {
  
    #if defined(TESTING)
      printf( "%d: Write %6.4f to to dac %d\n", elapsed, current_voltage, dac );
    #endif
    rtems_set_dac_voltage(dac, current_voltage);

    current_voltage += current_step;
    if ( current_voltage < low_voltage ) {
      current_step    = step_voltage;
      current_voltage = low_voltage;
    } else if ( current_voltage > high_voltage ) {
      current_step    = -1.0 * step_voltage;
      current_voltage = high_voltage;
    }

    elapsed += step_time;
    if ( elapsed > maximum_time )
      break;

    rtems_task_wake_after( step_ticks );
  }
   
  return 0;
}

rtems_shell_cmd_t Shell_MULTIIO_DAC_Command = {
  "multiio_dac",                            /* name */
  "Write Analog Outputs",                   /* usage */
  "multiio",                                /* topic */
  main_multiio_dac,                         /* command */
  NULL,                                     /* alias */
  NULL                                      /* next */
};


rtems_shell_alias_t Shell_MULTIIO_DAC_Alias = {
  "multiio_dac",         /* command */
  "dac"                  /* alias */
};
