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
    /* printf( "Testing: Write %6.4f to to dac %d\n", _voltage, _dac ); */
#endif

#define VALIDATE_VOLTAGE(_v) \
  if ( (_v) < -10.0 || (_v) > 10.0 ) { \
    printf( "Voltage must be between -10.0V and +10.0V\n" ); \
    fail = true; \
  }

char pcmmio_dac_usage[] =
  "Usage: %s dac voltage\n"
  "       %s dac low high step time_per_step maximum_time\n"
  "\n"
  "Where: dac must be 0-7\n"
  "       voltages and step must be -10V to +10V\n"
  "       times are in milliseconds\n"
  "  First form is a single write.\n"
  "  Second form writes a pattern.\n";

#define PRINT_USAGE() \
   printf( pcmmio_dac_usage, argv[0], argv[0] );

int main_pcmmio_dac(int argc, char **argv)
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
  if ( !rtems_string_to_int( argv[1], &dac, NULL, 0 ) ) {
    printf( "DAC (%s) is not a number\n", argv[1] );
    fail = true;
  }

  if ( !rtems_string_to_float( argv[2], &low_voltage, NULL ) ) {
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
    set_dac_voltage(dac, low_voltage);
  }

  /*
   *  Finish parsing the arguments to do a pattern
   */
  fail = false;

  if ( !rtems_string_to_float( argv[3], &high_voltage, NULL ) ) {
    printf( "Voltage (%s) is not a number\n", argv[3] );
    fail = true;
  }

  VALIDATE_VOLTAGE( high_voltage );

  if ( !rtems_string_to_float( argv[4], &step_voltage, NULL ) ) {
    printf( "Step voltage (%s) is not a number\n", argv[4] );
    fail = true;
  }

  VALIDATE_VOLTAGE( step_voltage );

  if ( step_voltage < 0.0 ) {
    printf( "Step voltage must be greater than 0\n" );
    fail = true;
  }

  if ( !rtems_string_to_int( argv[5], &step_time, NULL, 0 ) ) {
    printf( "Step time (%s) is not a number\n", argv[5] );
    fail = true;
  }

  if ( !rtems_string_to_int( argv[6], &maximum_time, NULL, 0 ) ) {
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
    set_dac_voltage(dac, current_voltage);

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

