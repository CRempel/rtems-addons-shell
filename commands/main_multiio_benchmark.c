/*
 *  multiio_irq command
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

char multiio_benchmark_usage[] =
  "Usage: %s [-i interrupts] [-v]\n"
  "Where: maximum interrupts must be >= 1\n";

#define PRINT_USAGE() \
   printf( multiio_benchmark_usage, argv[0] )

static uint64_t to_usecs( struct timespec *time )
{
  uint64_t usecs;
  usecs = _Timespec_Get_seconds( time ) * 1000000;
  usecs += _Timespec_Get_nanoseconds( time ) / 1000;
  return usecs;
}

int main_multiio_benchmark(int argc, char **argv)
{
  int                 milliseconds;
  int                 maximum;
  int                 sc;
  char                ch;
  bool                verbose;
  struct getopt_data  getopt_reent;
  const  char        *s;
  int                 interrupts;
  struct timespec     timestamp;
  struct timespec     now;
  struct timespec     min;
  struct timespec     max;
  struct timespec     total;
  struct timespec     took;
  struct timespec     average;

  /*
   * Parse arguments here
   */
  milliseconds = 1000;
  maximum = 1;
  verbose = false;

  memset(&getopt_reent, 0, sizeof(getopt_data));
  while ((ch = getopt_r(argc, argv, "i:v:", &getopt_reent)) != -1) {
    switch (ch) {
      case 'i': /* maximum interrupts */
        s = getopt_reent.optarg;
        if ( rtems_string_to_int( s, &maximum, NULL, 0 ) ) {
          printf( "Maximum interrupts (%s) is not a number\n", s );
          PRINT_USAGE();
          return -1;
        }
	if ( maximum <= 0 ) {
	  printf( "Maximum interrupts (%d) is invalid\n", maximum );
	  PRINT_USAGE();
          return -1;
	}
        break;
      case 'v': /* verbose */
        verbose = true;
        break;
      default:
        printf( multiio_benchmark_usage, argv[0] );
        return -1;
    }
  }

  printf( "Benchmarking for DIN IRQ for %d interrupts\n", maximum );

  /*
   *  Now sample in the loop
   */
  interrupts   = 0;
  _Timespec_Set( &min, 0xffffffff, 999999999 );
  _Timespec_Set_to_zero( &max );
  _Timespec_Set_to_zero( &total );

  rtems_din_flush_buffered_interrupts();
  while (1) {
    sc = 0;
   
    sc = rtems_din_wait_interrupt_with_timestamp(0, &timestamp);

    if ( sc == -1 )
      continue;

    rtems_clock_get_uptime( &now );

    _Timespec_Subtract( &now, &timestamp, &took );
    _Timespec_Add_to( &total, &took );

    if ( _Timespec_Less_than(&took, &min) )    min = took;
    if ( _Timespec_Greater_than(&took, &max) ) max = took;

    interrupts++;

    if (interrupts >= maximum )
      break;
  }

  _Timespec_Divide_by_integer( &total, maximum, &average );

  printf(
    "Number of Interrupts:  %d\n"
    "Total:                 %lld\n"
    "min/max/avg:           %lld/%lld/%lld\n",
    maximum,
    to_usecs( &total ),
    to_usecs( &min ),
    to_usecs( &max ),
    to_usecs( &average )
  );
  return 0;
}

rtems_shell_cmd_t Shell_MULTIIO_Benchmark_Command = {
  "multiio_benchmark",                             /* name */
  "Benchmark Multi IOInterrupts",                  /* usage */
  "multiio",                                       /* topic */
  main_multiio_benchmark,                          /* command */
  NULL,                                            /* alias */
  NULL                                             /* next */
};
