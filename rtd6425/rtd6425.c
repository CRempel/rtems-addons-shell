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

#define LIB_DEFINED

/* #define DEBUG 1 */

#include <bsp.h>
#include <stdio.h>
#include <stdlib.h>
#include "rtd6425.h"
#include <bsp/irq.h>

/* RTEMS Ids for Wait Queues */
rtems_id     rtd6425_dio_wq;
unsigned int rtd6425_dio_missed_interrupts;


typedef struct {
  uint16_t  irq;
  uint8_t   irqmask;
  uint8_t   previous;
} rtd6425_control_t;

rtd6425_control_t rtd6425_info[2];

/*
 *  Limits on number of buffered discrete input interrupts in
 *  the message queue.
 */
#define MAXIMUM_BUFFERED_DISCRETE_INTERRUPTS 1024

typedef struct {
  struct timespec timestamp;
  int             pin;
} rtd6425_din_message_t;

typedef struct {
  uint8_t  p0_direction; /* each bit 1 == output, 0 == input */
  bool     p1_direction; /* true when all output */
  uint8_t  p2_direction; /* each bit 1 == output, 0 == input */
  bool     p3_direction; /* true when all output */
  uint16_t control_reg;  /* value to program the conrtol register */
  uint16_t irq_reg;      /* value to program the irq register */
} rtd6425_config_t;

rtd6425_config_t rtd6425_configuration = {
  0x00,   /* p0_direction - each bit 1 == output, 0 == input */
  false,  /* p1_direction - true when all output */
  0x00,   /* p2_direction - each bit 1 == output, 0 == input */
  false,  /* p3_direction - true when all output */
  0x00,
  0x00
};

typedef enum  {
  RTD6425_ADC_RANGE_NEG_5_TO_5   = 0x0,
  RTD6425_ADC_RANGE_NEG_10_TO_10 = 0x1,
  RTD6425_ADC_RANGE_0_TO_10      = 0x2
} rtd6425_adc_range_t;

typedef enum  {
  RTD6425_ADC_GAIN_NONE = 0x0,
  RTD6425_ADC_GAIN_X1   = 0x1,
  RTD6425_ADC_GAIN_X2   = 0x2,
  RTD6425_ADC_GAIN_X4   = 0x4,
  RTD6425_ADC_GAIN_X8   = 0x8, 
} rtd6425_adc_gain_t; 

typedef enum {
  RTD6425_ADC_SINGLE_ENDED = 0,
  RTD6425_ADC_DIFFERENTIAL = 1
} rtd6425_adc_se_diff_t;

typedef struct {
   rtd6425_adc_range_t   range;
   rtd6425_adc_gain_t    gain;
   rtd6425_adc_se_diff_t se_diff;
} rtd6425_adc_config_t;

#define RTD6425_ADC_DEFAULT_CONFIGURATION \
  { RTD6425_ADC_RANGE_NEG_10_TO_10, RTD6425_ADC_GAIN_NONE, RTD6425_ADC_SINGLE_ENDED }

rtd6425_adc_config_t rtd6425_adc_configuration[ RTD6425_ADCs ] = {
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 00 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 01 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 02 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 03 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 04 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 05 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 06 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 07 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 08 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 09 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 10 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 11 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 12 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 13 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 14 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 15 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 16 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 17 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 18 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 19 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 20 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 21 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 22 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 23 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 24 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 25 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 26 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 27 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 28 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 29 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 30 */
  RTD6425_ADC_DEFAULT_CONFIGURATION,  /* ADC 31 */
};

bool rtd6425_is_writable( int pin )
{
  int      port;
  int      position;
  uint8_t  mask;

  port  = pin / 8;

  if ( port == 1 ) {
    if ( rtd6425_configuration.p1_direction == true )
      return false;
    return true;
  }

  if ( port == 3 ) {
    if ( rtd6425_configuration.p3_direction == true )
      return false;
    return true;
  }

  if ( port == 0 ) 
    mask = rtd6425_configuration.p0_direction;
  else
    mask = rtd6425_configuration.p2_direction;

  position = pin / 8;
  if ( (mask & (1 << position)) != 0 )
    return true;

  return false;
}

#if 1
  #define DEBUG_PRINT(...) printk( __VA_ARGS__ )
#else
  #define DEBUG_PRINT(...) 
#endif

#define rtd6425_outport_byte( _port, _value ) \
  do { \
    DEBUG_PRINT( "OUTB 0x%04x to 0x%04x\n", _value, _port ); \
    outport_byte( _port, _value ); \
  } while (0)

#define rtd6425_inport_byte( _port, _value ) \
  do { \
    inport_byte( _port, _value ); \
    DEBUG_PRINT( "INB 0x%04x from 0x%04x\n", _value, _port ); \
  } while (0)

#define rtd6425_outport_word( _port, _value ) \
  do { \
    DEBUG_PRINT( "OUTW 0x%04x to 0x%04x\n", _value, _port ); \
    outport_word( _port, _value ); \
  } while (0)

#define rtd6425_inport_word( _port, _value ) \
  do { \
    inport_word( _port, _value ); \
    DEBUG_PRINT( "INW 0x%04x from 0x%04x\n", _value, _port ); \
  } while (0)

uint16_t rtd6425_base;

#define DIO_SELECT_CLEAR      0 
#define DIO_SELECT_DIRECTION  1
#define DIO_SELECT_MASK       2
#define DIO_SELECT_COMPARE    3

void rtd6425_DIO_select_register(uint8_t port, uint8_t reg)
{
  uint8_t value;

  rtd6425_inport_byte(
    rtd6425_base  + RTD6425_MODE_5812 + (port * 0x400),
    value
  );

DEBUG_PRINT("select_reg1: port: %d, reg: %d, val: 0x%02x\n", port, reg, value);

  value &= 0xFC;
  value |= reg;
  
  rtd6425_outport_byte( rtd6425_base + RTD6425_MODE_5812 + (port * 0x400), value);

  rtd6425_inport_byte(
    rtd6425_base  + RTD6425_MODE_5812 + (port * 0x400),
    value
  );

DEBUG_PRINT("select_reg2: port: %d, reg: %d, val: 0x%02x\n", port, reg, value);
}

void rtd6425_write_dio_status(uint8_t chip, uint8_t mask, uint8_t new)
{
  uint8_t value;

  rtd6425_inport_byte(
    rtd6425_base  + RTD6425_MODE_5812 + (chip * 0x400),
    value
  );

  DEBUG_PRINT("write_dio_status1: chip: %d, mask: %d, val: 0x%02x\n", chip, mask, value);


  value &= 0xFC;
  value &= ~mask;
  value |= new;

  rtd6425_outport_byte( rtd6425_base + RTD6425_MODE_5812 + (chip * 0x400), value);

  rtd6425_inport_byte(
    rtd6425_base  + RTD6425_MODE_5812 + (chip * 0x400),
    value
  );

  DEBUG_PRINT("write_dio_status2: chip: %d, mask: %d, val: 0x%02x\n", chip, mask, value);

}

uint8_t rtd6425_read_dio_status( uint8_t chip )
{
  uint8_t value;

  rtd6425_inport_byte(
    rtd6425_base  + RTD6425_MODE_5812 + (chip * 0x400),
    value
  );

  DEBUG_PRINT("read_dio_status: chip: %d val: 0x%02x\n", chip, value);

  return value;
}

void rtd6425_din_queue_create(
  rtems_name  name,
  rtems_id   *id
)
{
  rtems_status_code rc;

  rc = rtems_message_queue_create(
    name,
    MAXIMUM_BUFFERED_DISCRETE_INTERRUPTS,
    sizeof(rtd6425_din_message_t),
    RTEMS_DEFAULT_ATTRIBUTES,
    id
  );
  if ( rc == RTEMS_SUCCESSFUL )
    return;

  DEBUG_PRINT( "Unable to create RTD6425 DIN IRQ Message Queue\n" );
  exit(1);
}

void rtd6425_initialize(
  unsigned short base_port,
  unsigned short irq0,
  unsigned short irq1
)
{
  uint16_t ignored;
  uint16_t value;
  int      i;

  /* Create RTEMS Objects */
  rtd6425_din_queue_create(
    rtems_build_name( 'd', 'i', 'o', ' ' ),
    &rtd6425_dio_wq
  );
  rtd6425_dio_missed_interrupts = 0;

printf( "RTD6426 Init( 0x%04x, 0x%04x, 0x%04x )\n", base_port, rtd6425_info[0].irq, rtd6425_info[1].irq );
  rtd6425_base = base_port;
  rtd6425_info[0].irq = irq0;
  rtd6425_info[1].irq  = irq1;

  /*
   * Validate ADC Table.
   */
  for( i=0; i<RTD6425_ADCs; i=i+2 ) {
    if ( rtd6425_adc_configuration[i].se_diff != 
         rtd6425_adc_configuration[i].se_diff ) {
      DEBUG_PRINT("Error==> ADC single ended mismatch\n");
      exit (0);
    } 

    if ( rtd6425_adc_configuration[i].se_diff == RTD6425_ADC_DIFFERENTIAL ) {
      if (rtd6425_adc_configuration[i].range != 
         rtd6425_adc_configuration[i].range ) {
        DEBUG_PRINT("Error==> ADC range mismatch - Either choose single ended or match the range of the twisted pair\n");
        exit (0);
      }
    }
  }

  /*
   *  Reset Board and clear it out
   */
  rtd6425_outport_word( rtd6425_base + RTD6425_CLEAR, 0x0001 );
  rtd6425_inport_word( rtd6425_base + RTD6425_CLEAR, ignored );

  /*
   *  Clear ADDMADone
   */
  rtd6425_outport_word( rtd6425_base + RTD6425_CLEAR, 0x0004 );
  rtd6425_inport_word( rtd6425_base + RTD6425_CLEAR, ignored );

  /*
   * Clear Gain Table
   */
  rtd6425_outport_word( rtd6425_base + RTD6425_CLEAR, 0x0008 );
  rtd6425_inport_word( rtd6425_base + RTD6425_CLEAR, ignored );

  /*
   *  Clear ADC FIFOs
   */
  rtd6425_outport_word( rtd6425_base + RTD6425_CLEAR, 0x0002 );
  rtd6425_inport_word( rtd6425_base + RTD6425_CLEAR, ignored );

  /*
   *  Clear DIN FIFOs
   */
  rtd6425_outport_word( rtd6425_base + RTD6425_CLEAR, 0x0020 );
  rtd6425_inport_word( rtd6425_base + RTD6425_CLEAR, ignored );

  /*
   * Program Discrete I/O directions
   */
  rtd6425_DIO_select_register( 0, DIO_SELECT_DIRECTION ); 
  rtd6425_outport_word(
    rtd6425_base + RTD6425_PORT_DIR_5812 + (0*0x400),
    rtd6425_configuration.p0_direction
  );

  rtd6425_write_dio_status(
    0,
    0x04, 
    (rtd6425_configuration.p1_direction) ? 0x04 : 0x00
  );

  rtd6425_DIO_select_register( 1, DIO_SELECT_DIRECTION ); 
  rtd6425_outport_word(
    rtd6425_base + RTD6425_PORT_DIR_5812 + (1*0x400),
    rtd6425_configuration.p2_direction
  );

  rtd6425_write_dio_status(
    1,
    0x04, 
    (rtd6425_configuration.p1_direction) ? 0x04 : 0x00
  );
DEBUG_PRINT("END: Program discrete I/O directions\n");

  rtd6425_outport_word( rtd6425_base + RTD6425_TRIGGER, 0x00 );

  /*
   * Set the
   */
  //Enable loading channel gain latch
  rtd6425_configuration.control_reg  &= 0xFFFC;
  rtd6425_outport_word( rtd6425_base + RTD6425_CONTROL, rtd6425_configuration.control_reg );
  
  for( i=0; i<RTD6425_ADCs; i++) {
  value = (i) | 
          (rtd6425_adc_configuration[i].gain << 5) | 
          (rtd6425_adc_configuration[i].range << 8) | 
          (rtd6425_adc_configuration[i].se_diff << 10);

    rtd6425_outport_word( rtd6425_base + RTD6425_CHANNEL_GAIN, value );
  }
DEBUG_PRINT("End ADC setup\n");
}

void rtd6425_chip_mode_select( uint8_t chip )
{
    uint8_t value;
    uint8_t mode = 0; /* Event Mode */ 

    /* Clear Bit to set to event mode */
    rtd6425_inport_byte(
      rtd6425_base  + RTD6425_MODE_5812 + (chip * 0x400),
      value
    );
    value  &= 0xf7;
    value |= (mode << 3);
    rtd6425_outport_byte( 
      rtd6425_base + RTD6425_MODE_5812 + (chip * 0x400), 
      value
    );
}

void rtd6425_set_IRQ(uint8_t chip, uint16_t source, uint16_t channel)
{
  uint16_t value;

  switch(channel) {
      case 3:  channel = 1; break;
      case 5:  channel = 2; break;
      case 9:  channel = 3; break;
      case 10: channel = 4; break;
      case 11: channel = 5; break;
      case 12: channel = 6; break;
      case 15: channel = 7; break;
      default: channel = 0; break;
  }
   
  if (chip == 0) 
    rtd6425_configuration.irq_reg = 
      rtd6425_configuration.irq_reg & 0xff00;
  else
    rtd6425_configuration.irq_reg = 
      rtd6425_configuration.irq_reg & 0x00ff;

  value = source | (channel << 5);
  rtd6425_configuration.irq_reg |= value << (chip*8);
  rtd6425_outport_word( 
    rtd6425_base + RTD6425_IRQ, 
    rtd6425_configuration.irq_reg 
  );
}

void rtd6425_enable_interrupt(uint8_t chip, uint8_t enable)
{
  uint8_t value;

  rtd6425_info[chip].irqmask  = (enable) ? 0xff : 0;
  rtd6425_inport_byte(
    rtd6425_base + (chip * 0x400) + RTD6425_PORT_0_5812,
    rtd6425_info[chip].previous
  );

  /* Clear Bit to set to event mode */
  rtd6425_inport_byte(
    rtd6425_base  + RTD6425_MODE_5812 + (chip * 0x400),
    value
  );
  value &= 0xef;
  value |= (enable << 4);
  rtd6425_outport_byte( 
    rtd6425_base + RTD6425_MODE_5812 + (chip * 0x400), 
    value
  );
}

void rtd6425_dio_check_pin_change(
  uint8_t                 base_bit,
  uint8_t                 previous,
  uint8_t                 current,
  uint8_t                 mask,
  rtd6425_din_message_t  *din
)
{
  uint8_t            changed;
  int                pin;
  int                pin_mask;
  rtems_status_code  rc;

  /*
   * Calculate which bits changed that we care about
   */
  changed = (previous ^ current) & mask;

  // printk( "check: %d prev=0x%02x curr=0x%02x mask=0x%02x\n", 
  //    base_bit, previous, current, mask );

  for ( pin=0 ; changed != 0 && pin<8 ; pin++ ) {
    pin_mask = 1 << pin;
    if ( (changed & pin_mask) == 0 )
      continue;

    din->pin = base_bit + pin;
    // printk( "check: %d changed\n", din->pin );
    rc = rtems_message_queue_send(
      rtd6425_dio_wq,
      din,
      sizeof(rtd6425_din_message_t)
    );
    if ( rc != RTEMS_SUCCESSFUL ) {
      rtd6425_dio_missed_interrupts++;
      DEBUG_PRINT("<1>Missed DIO interrupt %d\n", rc );
    }
    changed &= ~pin_mask;
  }
}

void  rtd6425_clear_Irq6425( void )
{
    uint16_t value;

    /* ClearIRQ16425 */
    rtd6425_outport_word( rtd6425_base  + RTD6425_CLEAR, 0x00C0 );
    rtd6425_inport_word( rtd6425_base  + RTD6425_CLEAR, value );
}

void  rtd6425_clear_Irq5812(uint8_t chip)
{
  uint8_t value;

  rtd6425_DIO_select_register(chip, 0);
  rtd6425_inport_byte(
    rtd6425_base  + RTD6425_CLEAR_5812 + (chip * 0x400),
    value
  );
}

rtems_isr rtd6425_dio_handler(void *arg);

rtems_irq_connect_data rtd6425_dio_cd0 = {
 0, rtd6425_dio_handler, (void *) &rtd6425_dio_cd0, NULL, NULL, NULL, NULL
};
rtems_irq_connect_data rtd6425_dio_cd1 = {
 0, rtd6425_dio_handler, (void *) &rtd6425_dio_cd1, NULL, NULL, NULL, NULL
};

int rtd6425_enable_dio_interrupt(void)
{
     int i;
     uint8_t mask, status;
  uint8_t v8;
DEBUG_PRINT("Start  rtd6425_enable_dio_interrupt\n");

  /* Set IRQ Source Digital IO. */
  /* XXX - Verify this */
  rtd6425_set_IRQ(0, 10, rtd6425_info[0].irq );
  rtd6425_set_IRQ(1, 16, rtd6425_info[1].irq );

  rtd6425_dio_cd0.name = rtd6425_info[0].irq;
  rtd6425_dio_cd1.name = rtd6425_info[1].irq;

  /* Install handler for each irq */
  BSP_install_rtems_shared_irq_handler( &rtd6425_dio_cd0 );
  BSP_install_rtems_shared_irq_handler( &rtd6425_dio_cd1 );

  // insure there won't be any interrupts from preset values on ports 0/1
  rtd6425_outport_byte(
    rtd6425_base + RTD6425_PORT_0_5812 + (0 * 0x400) + (1 * 2),
    0Xff
  );
  rtd6425_outport_byte( 
    rtd6425_base + RTD6425_PORT_0_5812 + (1 * 0x400) + (1 * 2), 
    0Xff
  );

  // set event mode for port interrupt
  rtd6425_chip_mode_select( 0 );
  rtd6425_chip_mode_select( 1 );

  // insures that no bits will be masked for interrupts
  DEBUG_PRINT("Set Mask for chip\n");
  rtd6425_DIO_select_register(0, 2 );
  rtd6425_outport_byte( 
    rtd6425_base + RTD6425_PORT_MASK_5812 + (0  * 0x400) , 
    0x00
  );
  rtd6425_DIO_select_register(1, 2 );
  rtd6425_outport_byte( 
     rtd6425_base + RTD6425_PORT_MASK_5812 + (1  * 0x400) , 
    0x00
  );

  DEBUG_PRINT("Turn interrupts on\n");
  rtd6425_enable_interrupt(0, 1);         // enable interrupt circuit at Port
  rtd6425_enable_interrupt(1, 1);         // enable interrupt circuit at Port
  
  /* ClearIrq5812(PORTSELECT01); */
  rtd6425_DIO_select_register( 0, 0 );
  rtd6425_inport_byte( rtd6425_base + (0 * 0x400)  + RTD6425_CLEAR_5812, v8 );
  rtd6425_DIO_select_register( 1, 0 );
  rtd6425_inport_byte( rtd6425_base + (1 * 0x400)  + RTD6425_CLEAR_5812, v8 );
   rtd6425_clear_Irq6425();

  rtd6425_clear_Irq5812(0);
  rtd6425_clear_Irq5812(1);

  
  for (i=0; i<2;i++){
     rtd6425_inport_byte( 
      rtd6425_base + RTD6425_PORT_MASK_5812 + (i  * 0x400), 
      mask
    );
     rtd6425_inport_byte( 
      rtd6425_base + RTD6425_STATUS_5812 + (i  * 0x400), 
      status
    ); 
    printk( "chip %d Mask: 0x%x Status: 0x%x IRQ: 0x%x\n",
      i,mask, status, rtd6425_configuration.irq_reg
    );
  }

  DEBUG_PRINT("Return from rtd6425_enable_dio_interrupt\n");

  return 0;
}

int rtd6425_dio_enab_bit_int(int bit_number, int polarity)
{
  int       dio_port;
  uint8_t   value;
  uint8_t   mask;
  uint16_t  io_port;
  uint8_t   new;

  if ( bit_number > RTD6425_DISCRETE_IO_BITS )
    return -1;

  dio_port = bit_number / 8;
  mask     = 1 << (bit_number % 8);
  new      = ((polarity) ? mask : 0);
  mask     = ~mask;

  switch ( dio_port ) {
    case 0: io_port = RTD6425_PORT_0_5812;         break;
    case 1: io_port = RTD6425_PORT_1_5812;         break;
    case 2: io_port = RTD6425_PORT_0_5812 + 0x400; break;
    case 3: io_port = RTD6425_PORT_1_5812 + 0x400; break;
    default: return -1;
  }

  rtd6425_DIO_select_register(0, 2 );

  // read the mask register and calculate the field
  rtd6425_inport_byte( 
    rtd6425_base + RTD6425_PORT_MASK_5812 + (0  * 0x400), 
    value
  );

DEBUG_PRINT("read interrupt mask 0x%x\n", value);
  value &= mask;
  value |= ~mask;
DEBUG_PRINT("write interrupt mask 0x%x\n", value);

  // write the value to the mask register
  rtd6425_DIO_select_register(0, 2 );
  rtd6425_outport_byte( 
    rtd6425_base + RTD6425_PORT_MASK_5812 + (0  * 0x400) , 
    value
  );


  return 0;
}

void rtd6425_flush_buffered_ints(void)
{
}

int  rtd6425_wait_dac_int_with_timeout(int dac_num, int milliseconds)
{
  return 0;
}

int  rtd6425_wait_dio_int_with_timeout(int milliseconds)
{
  return 0;
}

int  rtd6425_wait_dio_int_with_timestamp(
  int              milliseconds,
  struct timespec *timestamp
)
{
  rtems_status_code      rc;
  rtd6425_din_message_t  din;
  size_t                 received;

  mio_error_code = MIO_SUCCESS;

  rc = rtems_message_queue_receive(
    rtd6425_dio_wq,
    &din,
    &received,
    RTEMS_DEFAULT_OPTIONS,
    RTEMS_MILLISECONDS_TO_TICKS(milliseconds)
  );
  if ( rc == RTEMS_UNSATISFIED ) {
    mio_error_code = MIO_READ_DATA_FAILURE;
    return -1;
  }

  if ( rc == RTEMS_TIMEOUT ) {
    mio_error_code = MIO_TIMEOUT_ERROR;
    return -1;
  }

  if ( rc != RTEMS_SUCCESSFUL ) {
    DEBUG_PRINT( "wait_dio_int_with_timestamp - error %d\n", rc );
    exit( 0 );
  }

  if (timestamp)
    *timestamp = din.timestamp;
  return din.pin;
}

int rtd6425_dio_get_missed_interrupts(void)
{
  return 0;
}

float rtd6425_adc_get_channel_voltage(int channel)
{ 
  int16_t  value;
  float    result = 0;

  rtd6425_outport_word( rtd6425_base + RTD6425_CLEAR, 0x0002 );
  rtd6425_inport_word( rtd6425_base + RTD6425_CLEAR, value );

  rtd6425_configuration.control_reg  &= 0xFFFC;    //Enable loading channel gain latch
  rtd6425_outport_word( rtd6425_base + RTD6425_CONTROL, rtd6425_configuration.control_reg );
 
  value = (channel) | 
          (rtd6425_adc_configuration[channel].gain << 5) | 
          (rtd6425_adc_configuration[channel].range << 8) | 
          (rtd6425_adc_configuration[channel].se_diff << 10);

  rtd6425_outport_word( rtd6425_base + RTD6425_CHANNEL_GAIN, value );

  /* Start Conversion */
  rtd6425_inport_word( rtd6425_base +  RTD6425_START_CONVERSION, value );
  
  /* Wait on Fifo to not be empty */
   while ( ( value & 0x1 ) == 0 )
     rtd6425_inport_word( rtd6425_base +  RTD6425_STATUS, value );
    
   rtd6425_inport_word( rtd6425_base +  RTD6425_AD, value );
   value = value >> 3;
   printf("ADC Read: 0x%03x\n", value );

   switch( rtd6425_adc_configuration[channel].range ) {
    case RTD6425_ADC_RANGE_NEG_5_TO_5:
      result = (( 10.0 / 4095.0) * value);
      break;
    case RTD6425_ADC_RANGE_NEG_10_TO_10:
      result = (( 20.0 / 4095.0) * value);
      break;
    case RTD6425_ADC_RANGE_0_TO_10:
      result = (( 10.0 / 4095.0) * value);
      break;
   };

   return result;
}

int rtd6425_set_dac_voltage(int channel, float voltage)
{
  uint16_t  value;
  float     per_bit;
  uint16_t  dac_setup;
  uint16_t  range_select;

  // printf( "Write DAC %d %fV\n", channel, voltage );
  /*
   * Read DAC Setup, clean out bits indicating DAC selected
   */
  rtd6425_inport_word( rtd6425_base + RTD6425_DA_SETUP, dac_setup );
  dac_setup &= ~(3 << (channel * 2));

  /*
   *  Do some magic per range
   */
  if ( voltage >= 0.0 && voltage <= 5.0 ) {
    per_bit      = 5.0 / 4095;
    value        = (uint16_t) (voltage / per_bit);
    range_select = RTD6425_DAC_0V_to_5V;
  } else if ( voltage >= -5.0 && voltage <= 5.0 ) {
    per_bit      = 10.0 / 4095;
    value        = (uint16_t) ((voltage + 5.0) / per_bit);
    range_select = RTD6425_DAC_MINUS_5V_TO_5V;
  } else if ( voltage >= 0.0 && voltage <= 10.0 ) {
    per_bit      = 10.0 / 4095;
    value        = (uint16_t) (voltage / per_bit);
    range_select = RTD6425_DAC_0V_TO_10V;
  } else if ( voltage >= -10.0 && voltage <= 10.0 ) {
    per_bit      = 20.0 / 4095;
    value        = (uint16_t) ((voltage + 10.0) / per_bit);
    range_select = RTD6425_DAC_MINUS_10_TO_10V;
  } else {
    printf( "Voltage is out of range %f\n", voltage );
    return -1;
  }

  /*
   * Select voltage range needed
   */
  dac_setup |= range_select << (channel * 2); 
  rtd6425_outport_word( rtd6425_base + RTD6425_DA_SETUP, dac_setup );

  /*
   * Now write the selected voltage
   */
  rtd6425_outport_word( rtd6425_base + RTD6425_DAC_BASE + (channel * 2), value );
  return 0;
}

int rtd6425_dio_read_bit(int bit_number)
{
  int       dio_port;
  uint8_t   value;
  uint8_t   mask;
  uint16_t  io_port;

  if ( bit_number > RTD6425_DISCRETE_IO_BITS )
    return -1;

  dio_port = bit_number / 8;
  mask     = 1 << (bit_number % 8);

  switch ( dio_port ) {
    case 0: io_port = RTD6425_PORT_0_5812;         break;
    case 1: io_port = RTD6425_PORT_1_5812;         break;
    case 2: io_port = RTD6425_PORT_0_5812 + 0x400; break;
    case 3: io_port = RTD6425_PORT_1_5812 + 0x400; break;
    default: return -1;
  }

  rtd6425_inport_byte( rtd6425_base + io_port, value );
  return ((value & mask) ? 1 : 0);
}

int rtd6425_dio_write_bit(int bit_number, int val)
{
  int       dio_port;
  uint8_t   value;
  uint8_t   mask;
  uint16_t  io_port;
  uint8_t   new;

  if ( bit_number > RTD6425_DISCRETE_IO_BITS )
    return -1;

  dio_port = bit_number / 8;
  mask     = 1 << (bit_number % 8);
  new      = ((val) ? mask : 0);
  mask     = ~mask;

  switch ( dio_port ) {
    case 0: io_port = RTD6425_PORT_0_5812;         break;
    case 1: io_port = RTD6425_PORT_1_5812;         break;
    case 2: io_port = RTD6425_PORT_0_5812 + 0x400; break;
    case 3: io_port = RTD6425_PORT_1_5812 + 0x400; break;
    default: return -1;
  }

  rtd6425_inport_byte( rtd6425_base + io_port, value );
  value = (value & mask) | new;
  rtd6425_outport_byte( rtd6425_base + io_port, value );
  return 0;
}

bool rtd6425_is_chip_irq( uint8_t chip )
{
  uint8_t value;

  rtd6425_inport_word( rtd6425_base +  RTD6425_STATUS, value );
  if ( (value & 0x40) == 0 )    /* XXX - Validate this is correct bit. */
    return false;

  return true; 
}

/*
 * This is the RTD6425 DIO interrupt handler. It is called by the
 * actual hardware ISR.
 */
rtems_isr rtd6425_dio_handler(void *arg)
{
  uint8_t                 chip;
  uint8_t                 v8;
  uint16_t                v16;
  rtd6425_din_message_t   din;
  rtems_irq_connect_data *irq_conn = (rtems_irq_connect_data *)arg;
  uint16_t                base;

  rtems_clock_get_uptime( &din.timestamp );

printk("DIO ISR\n");

  chip = (irq_conn == & rtd6425_dio_cd0) ? 0 : 1;
  base = rtd6425_base + (chip * 0x400);

  /* check pins changed for chip n, port 1 */
  rtd6425_inport_word( base + RTD6425_PORT_0_5812, v8 );
  rtd6425_dio_check_pin_change(
    (chip == 0) ? 0 : 16,                 /* pins 0-7 or 16-23 */
    rtd6425_info[chip].previous,          /* last value we saw */
    v8,                                   /* current value */
    rtd6425_info[chip].irqmask,           /* pins we care about */
    &din                                  /* message */
  );
  rtd6425_info[chip].previous = v8;

  /* ClearIRQ16425 */
  rtd6425_outport_word( base + RTD6425_CLEAR, (0x0040<<chip) );
  rtd6425_inport_word( base + RTD6425_CLEAR, v16 );

  /* ClearIrq5812(PORTSELECT01); */
  rtd6425_DIO_select_register( chip, 0 );
  rtd6425_inport_byte( base  + RTD6425_CLEAR_5812, v8 );

  rtd6425_clear_Irq5812(chip);
  rtd6425_clear_Irq6425();

  if ( irq_conn->name >= 8 ) {
    rtd6425_outport_byte( 0x20, 0x20 );
  } else {
    rtd6425_outport_byte( 0xA0, 0x20 );
  }

  #ifdef DEBUG
    DEBUG_PRINT("<1>Buffering DIO interrupt on bit %d\n",int_num);
  #endif
}
