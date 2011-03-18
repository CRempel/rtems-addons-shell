/*
 *  COPYRIGHT (c) 1989-2011.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#include "multiio.h"
#include "mio_io.h"
#include <stdint.h>
#include <rtems.h>

int rtems_multiio_initialize(void)
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

  return 0;
}

const char *rtems_multiio_get_name(void)
{
  return "Winsystems";
}

/*
 *  ADC
 */
int rtems_adc_get_maximum(void)
{
  return 16;
}

float rtems_adc_get_channel_voltage(int adc)
{
  return adc_get_channel_voltage(adc);
}

/*
 *  DAC
 */
int rtems_dac_get_maximum(void)
{
  return 8;
}

float rtems_dac_get_minimum_voltage(void)
{
  return -10.0;
}

float rtems_dac_get_maximum_voltage(void)
{
  return 10.0;
}

int rtems_set_dac_voltage(int dac, float voltage)
{
  set_dac_voltage(dac, voltage);
  return 0;
}

/*
 *  Discrete Outputs
 */
int rtems_dout_get_maximum(void)
{
  return 48;
}

int rtems_set_dout(int dout, int value)
{
  dio_write_bit(dout+1, value);
  return 0;
}

/*
 *  Discrete Inputs
 */
int rtems_din_get_maximum(void)
{
  return 48;
}

int rtems_din_get(int din)
{
  return dio_read_bit(din+1);
}

int rtems_din_flush_buffered_interrupts(void)
{
  flush_buffered_ints();
  return 0;
}

int rtems_din_wait_interrupt_with_timestamp(
  int                milliseconds,
  struct timespec   *timestamp
)
{
  rtems_status_code sc;

  sc = wait_dio_int_with_timestamp(milliseconds, timestamp);
  return 0;
}

