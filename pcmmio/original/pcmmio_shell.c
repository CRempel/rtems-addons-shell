/*
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

rtems_task Init(
  rtems_task_argument ignored
)
{
  /*
   *  Initialize the PCMMIO module to use IRQ6.  Ours is jumpered for 0x300
   *  base address.  We use discrete input interrupts so enable all of the
   *  as edge triggered on 1.
   */
  pcmmio_initialize(0x300, 6);
  enable_dio_interrupt();
  { int i;
    for (i=1 ; i<=48 ; i++ )
      dio_enab_bit_int(i, 1);
  }

  /*
   *  Display a herald with some hints on usage
   */
  printf(
    "\n"
    "\n"
    "*** Shell Configured with WinSystems PCMMIO Commands ***\n"
    "Type \"help pcmmio\" for more information\n"
    "\n"
  );
  rtems_shell_init(
    "SHLL",                          /* task_name */
    RTEMS_MINIMUM_STACK_SIZE * 4,    /* task_stacksize */
    100,                             /* task_priority */
    "/dev/console",                  /* devname */
    false,                           /* forever */
    true,                            /* wait */
    NULL                             /* login */
  );
}

/* RTEMS configuration */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#ifdef RTEMS_BSP_HAS_IDE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_IDE_DRIVER
#endif
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_MICROSECONDS_PER_TICK     1000

#define CONFIGURE_MAXIMUM_TASKS                  20
#define CONFIGURE_MAXIMUM_SEMAPHORES             20
#define CONFIGURE_MAXIMUM_BARRIERS                3
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES          1
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 20

/*
 * discrete in messages are 12 bytes and there are 1024 of them 
 */
#define CONFIGURE_MESSAGE_BUFFER_MEMORY \
        CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(1024, 12)

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (6 * RTEMS_MINIMUM_STACK_SIZE)

#define CONFIGURE_STACK_CHECKER_ENABLED
#define CONFIGURE_MALLOC_STATISTICS

#define CONFIGURE_INIT
#include <rtems/confdefs.h>

#define CONFIGURE_SHELL_USER_COMMANDS \
  CONFIGURE_PCMMIO_COMMANDS
#define CONFIGURE_SHELL_USER_ALIASES \
  CONFIGURE_PCMMIO_ALIASES

#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL

#include <rtems/shellconfig.h>

