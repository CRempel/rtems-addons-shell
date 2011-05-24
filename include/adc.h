/**
 *  @file  multiio.h
 *
 *  This include file contains all the prototypes and definitions
 *  for the multiio user interface.
 */

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

/**
 * @brief multiio supported types
 */
typedef enum {
  MULTIIO_DAC,
  MULTIIO_ADC,
  MULTIIO_DOUT,
  MULTIIO_DIN,
  MULTIIO_THERMO
} rtems_multio_type;
  
/**
 * @brief multiio control structure.
 */
typedef struct {

  /** 
   * The Address that controls the pin.
   */ 
  void        *addr;

  /** 
   * The adc, dac, din, dout... that this 
   * structure controls
   */ 
  int          pin;
} rtems_multiio_control;

/**
 *  @brief Initialize the multiio interface
 *
 *  This function initializes the multiio interface.
 *
 *  @return This method returns 0 if the interface was initialized
 *  without any problems.
 */
int rtems_multiio_initialize(void);

/**
 *  @brief Get the name of the multiio device
 *
 *  This function returns the name of the multiio device.
 *
 *  @return This method returns a character string representing
 *  the name of the multiio device.
 */
const char *rtems_multiio_get_name(void);

/*
 *  ADC 
 */

/**
 * @brief adc control structure.
 */
typedef rtems_multiio_control rtems_adc_control;

/**
 *  @brief Get the maximum number of adcs supported.
 *
 *  This function returns the maximum supported adc.
 *  Note that the adcs are numbered 0 to maximum.
 *
 *  @return This method returns an integer representing the 
 *  maximum number of the adc supported.
 */
int rtems_adc_get_maximum(void);

/**
 *  @brief Get the rtems_adc_control for the @a adc.
 *
 *  This function returns the control structure of the 
 *  given adc.
 *
 *  @return This method returns the control structure 
 *  for @a adc. 
 */
rtems_adc_control *adc_lookup( int adc );

/**
 *  @brief Get adc channel voltage
 *
 *  This function returns the voltage in volts for the 
 *  channel represented by the @a adc input.
 *
 *  @param[in] adc that the voltage corresponds to
 *
 *  @return This method returns the volts for the 
 *  @a adc channel.
 */
float rtems_adc_get_channel_voltage( rtems_adc_control *adc );

/* 
 * DAC 
 */

/**
 * @brief dac control structure.
 */
typedef rtems_multiio_control rtems_dac_control;

/**
 *  @brief Get the maximum number of the supported dac
 *
 *  This function returns the maximum number of supported
 *  dacs.
 *
 *  @return This method returns an integer representing the 
 *  maximum supported dac.
 */
int rtems_dac_get_maximum(void);

/**
 *  @brief Get the rtems_dac_control for the @a dac.
 *
 *  This function returns the control structure of the 
 *  given dac.
 *
 *  @return This method returns the control structure 
 *  for @a dac. 
 */
rtems_dac_control *dac_lookup( int dac );

/**
 *  @brief Get the mimimum voltage
 *
 *  This function returns the minimum voltage that the
 *  dac may be programmed to.
 *
 *  @param[in] dac that the voltage corresponds to
 *
 *  @return This method returns an floating point voltage
 *  representing the minimum value the voltage may be
 *  programmed to.
 */
float rtems_dac_get_minimum_voltage(rtems_dac_control *dac);

/**
 *  @brief Get the maximum voltage
 *
 *  This function returns the maximum voltage that the
 *  dac may be programmed to.
 *
 *  @param[in] dac that the voltage corresponds to
 *
  *  @return This method returns an floating point voltage
 *  representing the maximum value the voltage may be
 *  programmed to.
 */
float rtems_dac_get_maximum_voltage(rtems_dac_control *dac);

/**
 *  @brief Set the dac voltage
 *
 *  This function sets the given @a dac voltage
 *  to the specified @a voltage.
 *
 *  @param[in] dac that the voltage corresponds to
 *
 *  @param[in] voltage to set the dac to
 *
 *  @return This method returns a 0 if floating point 
 *  voltage was set.
 */
int rtems_set_dac_voltage(rtems_dac_control *dac, float voltage);

/* 
 * Discrete Outputs 
 */

/**
 * @brief discrete out control structure.
 */
typedef rtems_multiio_control rtems_dout_control;

/**
 *  @brief Get the maximum discrete out
 *
 *  This function returns the maximum discrete out.
 *
 *  @return This method returns an integer representing the 
 *  maximum discrete out.
 */
int rtems_dout_get_maximum(void);

/**
 *  @brief Get the rtems_dout_control for the @a dout.
 *
 *  This function returns the control structure of the 
 *  given discrete out.
 *
 *  @return This method returns the control structure 
 *  for @a dout. 
 */
rtems_dout_control *dout_lookup( int dout );

/**
 *  @brief Set the discrete out value
 *
 *  This function sets the given @a dout
 *  to the specified @a value.
 *
 *  @param[in] dout to set
 *
 *  @param[in] raw value to set
 *
 *  @return This method returns a 0 if discrete out was
 *  set to the raw value specified.
 */
int rtems_set_dout(rtems_dout_control *dout, int value);

/* 
 * Discrete Inputs
 */

/**
 * @brief discrete out control structure.
 */
typedef rtems_multiio_control rtems_din_control;

/**
 *  @brief Get the maximum discrete in
 *
 *  This function returns the maximum discrete in.
 *
 *  @return This method returns an integer representing
 *  maximum supported discrete in.
 */
int rtems_din_get_maximum(void);

/**
 *  @brief Get the rtems_din_control for the @a din.
 *
 *  This function returns the control structure of the 
 *  given discrete input.
 *
 *  @return This method returns the control structure 
 *  for @a din. 
 */
rtems_din_control *din_lookup( int din );


/**
 *  @brief Get the discrete in value
 *
 *  This function returns the raw value of the specified
 *  @a discrete in.
 *
 *  @return This method returns an integer representing the 
 *  raw value of @a din
 */
int rtems_din_get( rtems_din_control *din );

/**
 * @brief Flush buffered inputs
 *
 * This function flushes buffered discrete inputs.
 * 
 * @return This method returns 0 when successful.
 */
int rtems_din_flush_buffered_interrupts(void);

/**
 * @brief Discrete In wait interrupt with timeout
 *
 * This function will wait for ANY discrete in to change
 * state for up to @a milliseconds.
 *
 * @param[in] milliseconds to wait for a change
 *
 * @param[out] timestamp of when the change occurred
 *
 * @param[out] discrete in control structure for the changed discrete.
 *
 * @return This method returns 0 when successful.
 *
 */
int rtems_din_wait_interrupt_with_timestamp(
  int                milliseconds,
  struct timespec   *timestamp,
  rtems_din_control *din
);

#ifdef __cplusplus
}
#endif

#endif
