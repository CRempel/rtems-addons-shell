/*
 *  pcmmio_irq command
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
#include <stdint.h>
#include <rtems/stringto.h>
#include <libcpu/cpuModel.h> /* for rdtsc */

#define __need_getopt_newlib
#include <getopt.h>

char pcmmio_benchmark_usage[] =
  "Usage: %s [-i interrupts] [-v]\n"
  "Where: maximum interrupts must be >= 1\n";

#define PRINT_USAGE() \
   printf( pcmmio_benchmark_usage, argv[0] )

/*
 *  Compute the number of TSC clicks per microsecond.
 */ 
extern uint64_t tsc_per_microsecond(void);
int din_timestamp_subtract( uint64_t, uint64_t );

uint64_t to_usecs( uint64_t cycles )
{
  return din_timestamp_subtract( 1, cycles + 1 );
}

int main_pcmmio_benchmark(int argc, char **argv)
{
  int                 maximum;
  int                 sc;
  char                ch;
  bool                verbose;
  struct getopt_data  getopt_reent;
  const  char        *s;
  int                 interrupts;
  uint64_t            timestamp;
  uint64_t            now;
  uint32_t            min_cycles;
  uint32_t            max_cycles;
  uint64_t            total_cycles;
  uint64_t            cycles;

  /*
   * Parse arguments here
   */
  maximum = 1;
  verbose = false;

  memset(&getopt_reent, 0, sizeof(getopt_data));
  while ((ch = getopt_r(argc, argv, "i:v", &getopt_reent)) != -1) {
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
        printf( pcmmio_benchmark_usage, argv[0] );
        return -1;
    }
  }

  printf( "Benchmarking for DIN IRQ for %d interrupts\n", maximum );

  /*
   *  Now catch interrupts in the loop
   */
  interrupts   = 0;
  min_cycles   = 0xffffffff;
  max_cycles   = 0;
  total_cycles = 0;

  flush_buffered_ints();
  while (1) {
    sc = 0;
   
    sc = wait_dio_int_with_timestamp(0, &timestamp);

    if ( sc == -1 )
      continue;

    now = rdtsc();

    cycles = now - timestamp;
    total_cycles += cycles;

    if ( cycles < min_cycles ) min_cycles = cycles;
    if ( cycles > max_cycles ) max_cycles = cycles;

    interrupts++;

    if (interrupts >= maximum )
      break;
  }

  printf(
    "Number of Interrupts:     %d\n"
    "Total Cycles:             %lld\n"
    "min/max/avg cycles:       %ld/%ld/%lld\n"
    "min/max/avg microseconds: %lld/%lld/%lld\n",
    maximum,
    total_cycles,
    min_cycles, max_cycles, total_cycles / maximum,
    to_usecs( min_cycles ),
    to_usecs( max_cycles ),
    to_usecs( total_cycles / maximum )
  );
  return 0;
}

rtems_shell_cmd_t Shell_PCMMIO_Benchmark_Command = {
  "pcmmio_benchmark",                              /* name */
  "Benchmark PCMMIO Interrupts",                   /* usage */
  "pcmmio",                                        /* topic */
  main_pcmmio_benchmark,                           /* command */
  NULL,                                            /* alias */
  NULL                                             /* next */
};
