/*
 *  RTEMS Simple Application to verify we survive initialization
 *  of board and can see it. :)
 *
 *  $Id$
 */

#include <rtems.h>
#include <stdio.h>

#include "mio_io.h"

int main(
  int argc,
  char **argv
)
{ 
  int bit;

  while ( 1 ) {
    for ( bit=0 ; bit<48 ; bit+=8 ) {
       printf(
          "%d-%d: %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d\n",
          bit,
          bit + 15,
          dio_read_bit(bit +  0), dio_read_bit(bit +  1),
          dio_read_bit(bit +  2), dio_read_bit(bit +  3),
          dio_read_bit(bit +  4), dio_read_bit(bit +  5),
          dio_read_bit(bit +  6), dio_read_bit(bit +  7),
          dio_read_bit(bit +  8), dio_read_bit(bit +  9),
          dio_read_bit(bit + 10), dio_read_bit(bit + 11),
          dio_read_bit(bit + 12), dio_read_bit(bit + 13),
          dio_read_bit(bit + 14), dio_read_bit(bit + 15)
      );
    }
    (void) rtems_task_wake_after( RTEMS_MILLISECONDS_TO_TICKS(1000) );
  }
  return 0;
}
