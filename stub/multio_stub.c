#include "multi_io.h"

int rtems_multiio_initialize(void)
{
  return 0;
}

const char *rtems_multiio_get_name(void)
{
  return "MultiIO Stub";
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
  return adc * 1.5;
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
  return 0;
}

/*
 *  Discrete Outputs
 */
int rtems_dout_get_maximum(void)
{
  return 0;
}

int rtems_set_dout(int dout, int value)
{
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
  return 0;
}

int rtems_din_flush_buffered_interrupts(void)
{
  return 0;
}

int rtems_din_wait_interrupt_with_timestamp(
  int                milliseconds,
  struct timespec   *timestamp
)
{
  return 0;
}

