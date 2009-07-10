/*
 *  RTEMS Configuration and Initialization for PCMMIO Demonstration Programs	
 *
 *  $Id$
 */

#include <rtems.h>
#include <stdlib.h>

#include "mio_io.h"

extern int main(int, char **);

void *POSIX_Init(
  void *ignored
)
{
  /* default jumpers --> 0x300 and 0 --> no IRQ */
  pcmmio_initialize(0x300, 0);

  main(0,NULL);

  exit(0);
  return NULL;
}

/* configuration information */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_POSIX_INIT_THREAD_TABLE
#define CONFIGURE_MAXIMUM_POSIX_THREADS      3
#define CONFIGURE_MAXIMUM_BARRIERS           4

#define CONFIGURE_INIT
#include <rtems/confdefs.h>

