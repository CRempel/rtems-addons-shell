/* mio_io.c WinSystems support module file for the  PCM-MIO RTEMS driver
 *
 *  $Id$
 *
 *  This file implements the hardware access routines as implemented for RTEMS.
 *  This is very likely close to what is required with no OS.
 */

/* #define DEBUG 1 */

#include "mio_io.h"    

#include <stdio.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdlib.h>     /* for exit */

#include <rtems.h>
#include <i386_io.h>
#include <bsp/irq.h>

/*
 *  These are configured by the initialization call.
 */

/* IRQ source or 0 ==> polled */
static unsigned short irq = 0;
/* This holds the base addresses of the board */
static unsigned short base_port = 0;

/* Function prototypes for local functions */
static int get_buffered_int(
  unsigned long long *timestamp
);
static void init_io(unsigned short io_address);
static void clr_int(int bit_number);
static int get_int(void);

/* RTEMS Ids for Wait Queues */
rtems_id wq_a2d_1;
rtems_id wq_a2d_2;
rtems_id wq_dac_1;
rtems_id wq_dac_2;
rtems_id wq_dio;

/*
 *  Limits on number of buffered discrete input interrupts in
 *  the message queue.
 */
#define MAXIMUM_BUFFERED_DISCRETE_INTERRUPTS 1024

///////////////////////////////////////////////////////////////////////////////
typedef struct {
  unsigned long long timestamp;
  int                pin;
} din_message_t;

unsigned int pcmmio_dio_missed_interrupts;

int interruptible_sleep_on(
  rtems_id *id,
  int       milliseconds
);
void wake_up_interruptible(
  rtems_id *id
);

//
//    MIO_READ_IRQ_ASSIGNED
//
//////////////////////////////////////////////////////////////////////////////

int mio_read_irq_assigned(void)
{
  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  /* All of our programming of the hardware is handled at this level so that 
     all of the routines that need to shove and IRQ value into hardware will 
     use this call.
  */

  return (irq & 0xff);
}

///////////////////////////////////////////////////////////////////////////////
//
//    READ_DIO_BYTE
//
//////////////////////////////////////////////////////////////////////////////

unsigned char read_dio_byte(int offset)
{
  unsigned char byte_val;
  unsigned char offset_val;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  /* All bit operations are handled at this level so we need only
     read and write bytes from the actual hardware.
  */

  offset_val = offset & 0xff;
  byte_val = inb(base_port + 0x10 + offset_val);
  return (byte_val & 0xff);
}

///////////////////////////////////////////////////////////////////////////////
//
//    MIO_READ_REG
//
//////////////////////////////////////////////////////////////////////////////

unsigned char mio_read_reg(int offset)
{
  unsigned char byte_val;
  unsigned char offset_val;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;


  /* This is a catchall register read routine that allows reading of
     ANY of the registers on the PCM-MIO. It is used primarily for
     retreiving control and access values in the hardware.
   */

  offset_val = offset & 0xff;
  byte_val = inb(base_port + offset_val);
  return (byte_val & 0xff);
}

///////////////////////////////////////////////////////////////////////////////
//
//    MIO_WRITE_REG
//
//////////////////////////////////////////////////////////////////////////////

int mio_write_reg(int offset, unsigned char value)
{
  unsigned char byte_val;
  unsigned char offset_val;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  /* This function like the previous allow unlimited
     write access to ALL of the registers on the PCM-MIO
   */

  offset_val = offset & 0xff;
  byte_val = value;
  outb(byte_val, base_port + offset_val);
  
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//    WRITE_DIO_BYTE
//
//////////////////////////////////////////////////////////////////////////////

int write_dio_byte(int offset, unsigned char value)
{
  unsigned char byte_val;
  unsigned char offset_val;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  /* All bit operations for the DIO are handled at this level
     and we need the driver to allow access to the actual
     DIO registers to update the value.
  */

  offset_val = offset & 0xff;
  byte_val = value;
  outb(byte_val, base_port + 0x10 + offset_val);

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//    WRITE_DAC_COMMAND
//
//////////////////////////////////////////////////////////////////////////////

int write_dac_command(int dac_num,unsigned char value)
{
  unsigned char  byte_val;
  unsigned char  offset_val;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  byte_val = dac_num & 0xff;		/* This is the DAC number */
  offset_val = value;		        /* This is the data value */
  if (byte_val)
    outb(offset_val,base_port + 0x0e);
  else
    outb(offset_val,base_port + 0x0a);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//    WRITE_ADC_COMMAND
//
//////////////////////////////////////////////////////////////////////////////

int write_adc_command(int adc_num,unsigned char value)
{
  unsigned char byte_val;
  unsigned char offset_val;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  byte_val = adc_num & 0xff;		/* This is the ADC number */
  offset_val = value;		        /* This is the data value */

  if(byte_val)
    outb(offset_val,base_port + 0x06);
  else
    outb(offset_val,base_port + 0x02);
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//    WRITE_DAC_DATA
//
//////////////////////////////////////////////////////////////////////////////

int write_dac_data(int dac_num, unsigned short value)
{
  unsigned short word_val;
  unsigned char byte_val;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  byte_val = dac_num;
  word_val = value;

  if(byte_val)		/* DAC 1 */
    outw(word_val,base_port+0x0c);
  else
    outw(word_val,base_port+8);
  
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//    DAC_READ_STATUS
//
//////////////////////////////////////////////////////////////////////////////

unsigned char dac_read_status(int dac_num)
{
  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  if (dac_num)
    return inb(base_port + 0x0f);

  return inb(base_port + 0x0b);
}

///////////////////////////////////////////////////////////////////////////////
//
//    ADC_READ_STATUS
//
//////////////////////////////////////////////////////////////////////////////

unsigned char adc_read_status(int adc_num)
{
  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  if (adc_num)
    return inb(base_port + 7);
  return inb(base_port + 3);
}

///////////////////////////////////////////////////////////////////////////////
//
//    ADC_READ_CONVERSION_DATA
//
//////////////////////////////////////////////////////////////////////////////

unsigned short adc_read_conversion_data(int channel)
{
  int adc_num;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  if (channel > 7)
    adc_num = 1;
  else
    adc_num = 0;

  if (adc_num)
    return inw(base_port + 4);

  return inw(base_port);
}


int dio_get_int_with_timestamp(
  unsigned long long *timestamp
)
{
  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  return get_buffered_int(timestamp) & 0xff;
}

int dio_get_int(void)
{
  mio_error_code = MIO_SUCCESS;

  return dio_get_int_with_timestamp(NULL);
}

int wait_adc_int_with_timeout(int adc_num, int milliseconds)
{
  int sc;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  if (adc_num) {
    sc = interruptible_sleep_on(&wq_a2d_1, milliseconds);
  } else {
    sc = interruptible_sleep_on(&wq_a2d_2, milliseconds);
  }

  return sc;
}

int wait_adc_int(int adc_num)
{
  return wait_adc_int_with_timeout(adc_num, 0);
}

int wait_dac_int_with_timeout(int dac_num, int milliseconds)
{
  int sc;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  if (dac_num) {
    sc = interruptible_sleep_on(&wq_dac_1, milliseconds);
  } else {
    sc = interruptible_sleep_on(&wq_dac_2, milliseconds);
  }

  return sc;
}

int wait_dac_int(int dac_num)
{
  return wait_dac_int_with_timeout(dac_num, 0);
}

int wait_dio_int_with_timestamp(
  int                 milliseconds,
  unsigned long long *timestamp
)
{
  rtems_status_code  rc;
  din_message_t      din;
  size_t             received;

  mio_error_code = MIO_SUCCESS;

  if (check_handle())   /* Check for chip available */
    return -1;

  rc = rtems_message_queue_receive(
    wq_dio,
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
    printk( "wait_dio_int_with_timestamp - error %d\n", rc );
    exit( 0 );
  }

  if (timestamp)
    *timestamp = din.timestamp;
  return din.pin;
}

int wait_dio_int_with_timeout(int milliseconds)
{
  return wait_dio_int_with_timestamp(milliseconds, NULL);
}

int wait_dio_int(void)
{
  return wait_dio_int_with_timestamp(0, NULL);
}

static int handle = 0; /* XXX move to lower */

int check_handle(void)
{
  if (handle > 0)  /* If it's already a valid handle */
    return 0;

  if (handle == -1)  /* If it's already been tried */
  {
    mio_error_code = MIO_OPEN_ERROR;
    sprintf(mio_error_string,"MIO - Unable to open device PCMMIO");
    return -1;
  }

  /*
   * 0  ==> not initialized
   * 1+ ==> valid file handle, thus initialized
   * -1 ==> already attempted to open
   */
  handle = 1;
  return 0;

  /* if an error happens, go here */
  mio_error_code = MIO_OPEN_ERROR;
  sprintf(mio_error_string,"MIO - Unable to open device PCMMIO");
  handle = -1;
  return -1;
}

/*
 *  RTEMS barrier create helper
 */
void pcmmio_barrier_create(
  rtems_name  name,
  rtems_id   *id
)
{
  rtems_status_code rc;

  rc = rtems_barrier_create( name, RTEMS_BARRIER_MANUAL_RELEASE, 0, id );
  if ( rc == RTEMS_SUCCESSFUL )
    return;

 printk( "Unable to create PCMMIO Barrier\n" );
 exit(1);
}

/*
 *  RTEMS barrier create helper
 */
void pcmmio_din_queue_create(
  rtems_name  name,
  rtems_id   *id
)
{
  rtems_status_code rc;

  rc = rtems_message_queue_create(
    name,
    MAXIMUM_BUFFERED_DISCRETE_INTERRUPTS,
    sizeof(din_message_t),
    RTEMS_DEFAULT_ATTRIBUTES,
    id
  );
  if ( rc == RTEMS_SUCCESSFUL )
    return;

  printk( "Unable to create PCMMIO DIN IRQ Message Queue\n" );
  exit(1);
}

int interruptible_sleep_on(
  rtems_id *id,
  int       milliseconds
)
{
  rtems_status_code rc;

  rc = rtems_barrier_wait(*id, RTEMS_MILLISECONDS_TO_TICKS(milliseconds));
  if ( rc == RTEMS_SUCCESSFUL )
    return 0;

  mio_error_code = MIO_TIMEOUT_ERROR;
  return -1;
}

void wake_up_interruptible(
  rtems_id *id
)
{
  uint32_t  unblocked;

  (void) rtems_barrier_release(*id, &unblocked);
}

/*
 *  RTEMS specific interrupt handler
 */
#include <bsp/irq.h>

void common_handler(void);

void pcmmio_irq_handler(
  rtems_irq_hdl_param param
)
{
  common_handler();
}

static void pcmmio_irq_disable(const rtems_irq_connect_data *irq)
{
  BSP_irq_disable_at_i8259s(irq->name);
}
static void pcmmio_irq_enable(const rtems_irq_connect_data *irq)
{
  BSP_irq_enable_at_i8259s(irq->name);
}

static int pcmmio_irq_is_on(const rtems_irq_connect_data *irq)
{
  return BSP_irq_enabled_at_i8259s( irq->name );
}

rtems_irq_connect_data pcmmio_irq = {
  0,                            // name
  pcmmio_irq_handler,           // handler
  NULL,                         // parameter
  pcmmio_irq_enable,            // enable IRQ
  pcmmio_irq_disable,           // disable IRQ
  pcmmio_irq_is_on,             // is IRQ enabled
};

/* from pcmmio.c - GNU/Linux driver */
void init_io(unsigned short io_address)
{
  int x;
  unsigned short port;

  /* save the address for later use */
  port = io_address + 0X10;

  /* Clear all of the I/O ports. This also makes them inputs */
  for(x=0; x < 7; x++)
    outb(0,port+x);

  /* Set page 2 access, for interrupt enables */
  outb(0x80,port+7);

  /* Clear all interrupt enables */
  outb(0,port+8);
  outb(0,port+9);
  outb(0,port+0x0a);

  /* Restore page 0 register access */
  outb(0,port+7);
}

/*
 * RTEMS specific initialization routine
 */
void pcmmio_initialize(
  unsigned short _base_port,
  unsigned short _irq
)
{
  /* hardware configuration information */
  base_port                    = _base_port;
  irq                          = _irq;
  pcmmio_dio_missed_interrupts = 0;

  /* Create RTEMS Objects */
  pcmmio_barrier_create( rtems_build_name( 'a', '2', 'd', '1' ), &wq_a2d_1 );
  pcmmio_barrier_create( rtems_build_name( 'd', 'a', 'c', '1' ), &wq_dac_1 );
  pcmmio_barrier_create( rtems_build_name( 'd', 'a', 'c', '2' ), &wq_dac_2 );
  pcmmio_din_queue_create( rtems_build_name( 'd', 'i', 'o', ' ' ), &wq_dio );

  if ( base_port )
    init_io( base_port );

  /* install IRQ handler */
  if ( base_port && irq ) {
    int status = 0;
    pcmmio_irq.name = irq;
    #if defined(BSP_SHARED_HANDLER_SUPPORT)
      printk( "PCMMIO Installing IRQ handler as shared\n" );
      status = BSP_install_rtems_shared_irq_handler( &pcmmio_irq );
    #else
      printk( "PCMMIO Installing IRQ handler as non-shared\n" );
      status = BSP_install_rtems_irq_handler( &pcmmio_irq );
    #endif
    if ( !status ) {
      printk("Error installing PCMMIO interrupt handler! status=%d\n", status );
    }
  }
}

#include <libcpu/cpuModel.h> /* for rdtsc */

/*
 *  From this point down, we should be able to share easily with the Linux
 *  driver but I haven't gone to the trouble to do surgery on it.  I have
 *  no way to test it.
 */

/* real copy is in mio_io.c */
extern unsigned char adc2_port_image;

/* This is the common interrupt handler. It is called by the
 * actual hardware ISR.
 */

void common_handler(void)
{
  unsigned char status;
  unsigned char int_num;

  /* Read the interrupt ID register from ADC2 */

  adc2_port_image = adc2_port_image | 0x20;
  outb(adc2_port_image,base_port + 0x0f);

  status = inb(base_port + 0x0f);
  if (status & 1) {
    /* Clear ADC1 interrupt */
    inb(base_port+1);      /* Clear interrupt */

    /* Wake up any holding processes */
    wake_up_interruptible(&wq_a2d_1);
  }

  if (status & 2) {
    /* Clear ADC1 interrupt */
    inb(base_port+5);      /* Clear interrupt */

    /* Wake up anybody waiting for ADC1 */
    wake_up_interruptible(&wq_a2d_2);
  }

  if (status & 4) {
    /* Clear DAC1 interrupt */
    inb(base_port+9);    /* Clear interrupt */

    /* Wake up if you're waiting on DAC1 */
    wake_up_interruptible(&wq_dac_1);
  }

  if (status & 8) {

    /* DIO interrupt. Find out which bit */
    int_num = get_int();
    if (int_num) {
      rtems_status_code  rc;
      din_message_t      din;

      din.timestamp = rdtsc(); 
      din.pin       = int_num; 

      rc = rtems_message_queue_send( wq_dio, &din, sizeof(din_message_t) );
      if ( rc != RTEMS_SUCCESSFUL ) {
        pcmmio_dio_missed_interrupts++;
        #ifdef DEBUG
          printk("<1>Missed DIO interrupt\n" );
        #endif
     }
     #ifdef DEBUG
       printk("<1>Buffering DIO interrupt on bit %d\n",int_num);
     #endif

      /* Clear the interrupt */
      clr_int(int_num);
    }
  }

  if (status & 0x10) {
    /* Clear DAC2 Interrupt */
    inb(base_port+0x0d);    /* Clear interrupt */

    /* Wake up DAC2 holding processes */
    wake_up_interruptible(&wq_dac_2);
  }

  /* Reset the access to the interrupt ID register */
  adc2_port_image = adc2_port_image & 0xdf;
  outb(adc2_port_image,base_port+0x0f);
}


void clr_int(int bit_number)
{
  unsigned short port;
  unsigned short temp;
  unsigned short mask;
  unsigned short dio_port;

  dio_port = base_port + 0x10;

  /* Also adjust bit number */
  --bit_number;

  /* Calculate the I/O address based upon bit number */
  port = (bit_number / 8) + dio_port + 8;

  /* Calculate a bit mask based upon the specified bit number */
  mask = (1 << (bit_number % 8));

  /* Turn on page 2 access */
  outb(0x80,dio_port+7);

  /* Get the current state of the interrupt enable register */
  temp = inb(port);

  /* Temporarily clear only our enable. This clears the interrupt */
  temp = temp & ~mask;    /* Clear the enable for this bit */

  /* Now update the interrupt enable register */
  outb(temp,port);

  /* Re-enable our interrupt bit */
  temp = temp | mask;
  outb(temp,port);

  /* Set access back to page 0 */
  outb(0x00,dio_port+7);
}

int get_int(void)
{
  int temp;
  int x;
  unsigned short dio_port;

  dio_port = base_port + 0x10;

  /* Read the master interrupt pending register,
           mask off undefined bits */
  temp = inb(dio_port+6) & 0x07;

  /* If there are no pending interrupts, return 0 */
  if ((temp & 7) == 0)
    return 0;

  /* There is something pending, now we need to identify it */

  /* Set access to page 3 for interrupt id register */
  outb(0xc0, dio_port + 7);

  /* Read the interrupt ID register for port 0 */
  temp = inb(dio_port+8);

  /* See if any bit set, if so return the bit number */
  if (temp != 0) {
    for (x=0; x<=7; x++) {
      if (temp & (1 << x)) {
        outb(0,dio_port+7);
        return(x+1);
       }
    }
  }

  /* None in port 0, read port 1 interrupt ID register */
  temp = inb(dio_port+9);

  /* See if any bit set, if so return the bit number */
  if (temp != 0) {
    for (x=0; x<=7; x++) {
      if (temp & (1 << x)) {
        outb(0,dio_port+7);
        return(x+9);
      }
    }
  }

  /* Lastly, read the status of port 2 interrupt ID register */
  temp = inb(dio_port+0x0a);

  /* If any pending, return the appropriate bit number */
  if (temp != 0) {
    for (x=0; x<=7; x++) {
      if (temp & (1 << x)) {
         outb(0,dio_port+7);
         return(x+17);
      }
    }
  }

  /* We should never get here unless the hardware is seriously
     misbehaving, but just to be sure, we'll turn the page access
     back to 0 and return a 0 for no interrupt found
  */
  outb(0,dio_port+7);
  return 0;
}

void flush_buffered_ints(void)
{
  rtems_status_code  rc;
  size_t             flushed;

  rc = rtems_message_queue_flush( wq_dio, &flushed );
  if ( rc != RTEMS_SUCCESSFUL ) {
    printk( "flushed_buffered_int - error %d\n", rc );
    exit( 0 );
  }
}

int get_buffered_int(
  unsigned long long *timestamp
)
{
  rtems_status_code  rc;
  din_message_t      din;
  int                line;
  size_t             received;

  if (irq == 0) {
    line = get_int();
    if (line)
      clr_int(line);
    return line;
  }

  rc = rtems_message_queue_receive(
    wq_dio,
    &din,
    &received,
    RTEMS_NO_WAIT,
    0
  );
  if ( rc == RTEMS_UNSATISFIED ) {
    mio_error_code = MIO_READ_DATA_FAILURE;
    return 0;
  }

  if ( rc != RTEMS_SUCCESSFUL ) {
    printk( "get_buffered_int - error %d\n", rc );
    exit( 0 );
  }

  if (timestamp)
    *timestamp = din.timestamp;
  return din.pin;
}

int dio_get_missed_interrupts(void)
{
  int isrs;

  isrs = pcmmio_dio_missed_interrupts;

  pcmmio_dio_missed_interrupts = 0;

  return isrs;
}
