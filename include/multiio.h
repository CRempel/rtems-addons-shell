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

#ifndef __multiio_h
#define __multiio_h

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

int rtems_multiio_initialize(void);

const char *rtems_multiio_get_name(void);

/* ADC */

int rtems_adc_get_maximum(void);

float rtems_adc_get_channel_voltage(int adc);

/* DAC */

int rtems_dac_get_maximum(void);

float rtems_dac_get_minimum_voltage(void);

float rtems_dac_get_maximum_voltage(void);

int rtems_set_dac_voltage(int dac, float voltage);

/* Discrete Outputs */

int rtems_dout_get_maximum(void);

int rtems_set_dout(int dout, int value);

/* Discrete Inputs */

int rtems_din_get_maximum(void);

int rtems_din_get(int din);

int rtems_din_flush_buffered_interrupts(void);

int rtems_din_wait_interrupt_with_timestamp(
  int                milliseconds,
  struct timespec   *timestamp
);

#ifdef __cplusplus
}
#endif

#endif
