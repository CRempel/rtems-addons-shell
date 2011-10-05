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
#include "rtd6425.h"
#include <stdint.h>
#include <rtems.h>

int rtems_multiio_initialize(void)
{
  /*
   *  Initialize the RTD6425 module to use IRQ 11 and 7.  This is sofware
   *  programmable.  Ours is jumpered for 0x300  base address.  
   *  We use discrete input interrupts so enable all of the
   *  as edge triggered on 1.
   */
  rtd6425_initialize(0x300, 10, 11);
  rtd6425_enable_dio_interrupt();

/*  XXX - Enabling all interrupts atm
  { int i;
    for (i=1 ; i<=RTD6425_DISCRETE_IO_BITS ; i++ )
      rtd6425_dio_enab_bit_int(i, 1);
  }
*/
  return 0;
}

const char *rtems_multiio_get_name(void)
{
  return "RTD6425";
}

/*
 *  ADC
 */
int rtems_adc_get_maximum(void)
{
  return RTD6425_ADCs;
}

float rtems_adc_get_channel_voltage(int adc)
{
  return rtd6425_adc_get_channel_voltage(adc);
}

/*
 *  DAC
 */
int rtems_dac_get_maximum(void)
{
  return RTD6425_DACs;
}

float rtems_dac_get_minimum_voltage(void)
{
  return RTD6425_VOLTAGE_MINIMUM;
}

float rtems_dac_get_maximum_voltage(void)
{
  return RTD6425_VOLTAGE_MAXIMUM;
}

int rtems_set_dac_voltage(int dac, float voltage)
{
  rtd6425_set_dac_voltage(dac, voltage);
  return 0;
}

/*
 *  Discrete Outputs
 */
int rtems_dout_get_maximum(void)
{
  return RTD6425_DISCRETE_IO_BITS;
}

int rtems_set_dout(int dout, int value)
{
  rtd6425_dio_write_bit(dout, value);
  return 0;
}

/*
 *  Discrete Inputs
 */
int rtems_din_get_maximum(void)
{
  return RTD6425_DISCRETE_IO_BITS;
}

int rtems_din_get(int din)
{
  return rtd6425_dio_read_bit(din);
}

int rtems_din_flush_buffered_interrupts(void)
{
  rtd6425_flush_buffered_ints();
  return 0;
}

int rtems_din_wait_interrupt_with_timestamp(
  int                milliseconds,
  struct timespec   *timestamp
)
{
  rtems_status_code sc;

  sc = rtd6425_wait_dio_int_with_timestamp(milliseconds, timestamp);
  return sc;
}

