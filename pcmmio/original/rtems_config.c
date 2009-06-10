/*
 *  $Id$
 */

#include <rtems.h>
#include <stdlib.h>

#include "mio_io.h"

extern int main(int, char **);

rtems_task Init(
  rtems_task_argument ignored
)
{
    
  pcmmio_initialize(0, 0);
  main(0,NULL);
  exit(0);
}

/* configuration information */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_TASKS       1
#define CONFIGURE_MAXIMUM_BARRIERS    4

#define CONFIGURE_INIT
#include <rtems/confdefs.h>

