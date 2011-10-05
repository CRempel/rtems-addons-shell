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

#ifndef __RTD6425_h
#define __RTD6425_h

#ifdef __cplusplus
extern "C" {
#endif


/* These are the error codes for mio_error_code */

#define MIO_SUCCESS 0
#define MIO_OPEN_ERROR 1
#define MIO_TIMEOUT_ERROR 2
#define MIO_BAD_CHANNEL_NUMBER 3
#define MIO_BAD_MODE_NUMBER 4
#define MIO_BAD_RANGE 5
#define MIO_COMMAND_WRITE_FAILURE 6
#define MIO_READ_DATA_FAILURE 7
#define MIO_MISSING_IRQ 8
#define MIO_ILLEGAL_VOLTAGE 9

/*
 *  Configuration information for multiio interface
 */
#define RTD6425_DISCRETE_IO_BITS 31
#define RTD6425_ADCs             32  /* 0 based for single ended */
#define RTD6425_DACs             3   /* 0 based for 4 DACs */

#define RTD6425_VOLTAGE_MAXIMUM  10.0
#define RTD6425_VOLTAGE_MINIMUM  -10.0

/*
 *  Register offsets
 */
#define RTD6425_CLEAR                0  // Clear Register (Read/Write)
#define RTD6425_STATUS               2  // Status Register (Read)
#define RTD6425_CONTROL              2  // Control Register (Write)
#define RTD6425_AD                   4  // AD Data (Read)
#define RTD6425_CHANNEL_GAIN         4  // Channel/Gain Register (Write)
#define RTD6425_AD_TABLE             4  // AD Table (Write)
#define RTD6425_DIGITAL_TABLE        4  // Digital Table (Write)
#define RTD6425_START_CONVERSION     6  // Start Conversion (Read)
#define RTD6425_TRIGGER              6  // Trigger Register (Write)
#define RTD6425_IRQ                  8  // IRQ Register (Write)
#define RTD6425_DIN_FIFO             10 // Digital Input FIFO Data (Read)
#define RTD6425_DIN_CONFIG           10 // Config Digital Input FIFO (Write)
#define RTD6425_LOAD_AD_SAMPLE_COUNT 14 // Load A/D Sample Counter (Read)
#define RTD6425_TIMER_CLCK0          16 // Timer/Counter 0 (Read/Write)
#define RTD6425_TIMER_CLCK1          18 // Timer/Counter 1 (Read/Write)
#define RTD6425_TIMER_CLCK2          20 // Timer/Counter 2 (Read/Write)
#define RTD6425_TIMER_CTRL           22 // Timer/Counter Control Word (Write)

#define RTD6425_PORT_0_5812          24 // Digital I/O Port 0 Register
#define RTD6425_PORT_1_5812          26 // Digital I/O Port 1 Register
#define RTD6425_PORT_DIR_5812        28 // Digital I/O Port 0 Direction Register
#define RTD6425_PORT_MASK_5812       28 // Digital I/O Port 0 Mask Register
#define RTD6425_PORT_COMP_5812       28 // Digital I/O Port 0 Compare Register
#define RTD6425_CLEAR_5812           28 // Digital I/O Clear Register
#define RTD6425_STATUS_5812          30 // Digital I/O Status Register
#define RTD6425_MODE_5812            30 // Digital I/O Mode Register


#define RTD6425_DA_SETUP             0x400
#define RTD6425_DA_SIMULT            0x402
#define RTD6425_DAC_BASE             0x404
#define RTD6425_ANTRIG_SETUP         0x410

#define RTD6425_RTD_ID_BA            0x800   // base address of RTD ID hardware
#define RTD6425_SRAM                 0xC00

/*
 *  DAC Range Select Constants
 */
#define RTD6425_DAC_MINUS_5V_TO_5V    0
#define RTD6425_DAC_0V_to_5V          1
#define RTD6425_DAC_MINUS_10_TO_10V   2
#define RTD6425_DAC_0V_TO_10V         3


#ifdef LIB_DEFINED

/* These are used by the library functions */

int mio_error_code;
char mio_error_string[128];
#endif

void rtd6425_initialize(
  unsigned short _base_port,
  unsigned short _irq1,
  unsigned short  irq2
);

int rtd6425_enable_dio_interrupt(void);

int rtd6425_dio_enab_bit_int(int bit_number, int polarity);

void rtd6425_flush_buffered_ints(void);

int  rtd6425_wait_dac_int_with_timeout(int dac_num, int milliseconds);

int  rtd6425_wait_dio_int_with_timeout(int milliseconds);

int  rtd6425_wait_dio_int_with_timestamp(
  int              milliseconds,
  struct timespec *timestamp
);

int rtd6425_dio_get_missed_interrupts(void);

float rtd6425_adc_get_channel_voltage(int channel);

int rtd6425_set_dac_voltage(int channel, float voltage);

int rtd6425_dio_read_bit(int bit_number);

int rtd6425_dio_write_bit(int bit_number, int val);

#ifdef __cplusplus
}
#endif

#endif
/* end of include file */
