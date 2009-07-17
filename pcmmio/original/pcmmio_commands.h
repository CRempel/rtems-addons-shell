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

#ifndef __RTEMS_PCMMIO_COMMANDS_h
#define __RTEMS_PCMMIO_COMMANDS_h

#include <rtems.h>
#include <rtems/shell.h>

extern int main_pcmmio_din(int argc, char **argv);

extern rtems_shell_cmd_t Shell_PCMMIO_DIN_Command;

#define CONFIGURE_PCMMIO_COMMANDS \
  &Shell_PCMMIO_DIN_Command

#endif
