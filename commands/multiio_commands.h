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

extern int main_din(int argc, char **argv);

extern rtems_shell_cmd_t Shell_MULTIIO_DIN_Command;
extern rtems_shell_cmd_t Shell_MULTIIO_DOUT_Command;
extern rtems_shell_cmd_t Shell_MULTIIO_ADC_Command;
extern rtems_shell_cmd_t Shell_MULTIIO_DAC_Command;
extern rtems_shell_cmd_t Shell_MULTIIO_IRQ_Command;
extern rtems_shell_cmd_t Shell_MULTIIO_Benchmark_Command;

#define CONFIGURE_MULTIIO_COMMANDS \
  &Shell_MULTIIO_DIN_Command, \
  &Shell_MULTIIO_DOUT_Command, \
  &Shell_MULTIIO_ADC_Command, \
  &Shell_MULTIIO_DAC_Command, \
  &Shell_MULTIIO_IRQ_Command, \
  &Shell_MULTIIO_Benchmark_Command \

extern rtems_shell_alias_t Shell_MULTIIO_DIN_Alias;
extern rtems_shell_alias_t Shell_MULTIIO_DOUT_Alias;
extern rtems_shell_alias_t Shell_MULTIIO_ADC_Alias;
extern rtems_shell_alias_t Shell_MULTIIO_DAC_Alias;

#define CONFIGURE_MULTIIO_ALIASES \
  &Shell_MULTIIO_DIN_Alias, \
  &Shell_MULTIIO_DOUT_Alias, \
  &Shell_MULTIIO_ADC_Alias, \
  &Shell_MULTIIO_DAC_Alias
  
#endif
