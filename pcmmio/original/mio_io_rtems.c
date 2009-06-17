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

/*
 *  These are configured by the initialization call.
 */

/* IRQ source or 0 ==> polled */
static unsigned short irq = 0;
/* This holds the base addresses of the board */
static unsigned short base_port = 0;

/* Function prototypes for local functions */
int get_buffered_int(
  unsigned long long *timestamp
);
void init_io(unsigned short io_address);
void clr_int(int bit_number);
int get_int(void);

/* RTEMS Ids for Wait Queues */
rtems_id wq_a2d_1;
rtems_id wq_a2d_2;
rtems_id wq_dac_1;
rtems_id wq_dac_2;
rtems_id wq_dio;

void interruptible_sleep_on(
  rtems_id *id
);
void wake_up_interruptible(
  rtems_id *id
);

///////////////////////////////////////////////////////////////////////////////
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
  if (check_handle())   /* Check for chip available */
    return -1;

  return get_buffered_int(timestamp) & 0xff;
}

int dio_get_int(void)
{
  return dio_get_int_with_timestamp(NULL);
}

int wait_adc_int(int adc_num)
{
  if (check_handle())   /* Check for chip available */
    return -1;

  if (adc_num) {
    interruptible_sleep_on(&wq_a2d_1);
  } else {
    interruptible_sleep_on(&wq_a2d_2);
  }

  return 0;
}


int wait_dac_int(int dac_num)
{
  if (check_handle())   /* Check for chip available */
    return -1;

  if (dac_num) {
    interruptible_sleep_on(&wq_dac_1);
  } else {
    interruptible_sleep_on(&wq_dac_2);
  }

  return 0;
}


int wait_dio_int(void)
{
  int i;

  if (check_handle())   /* Check for chip available */
    return -1;

  if((i = get_buffered_int(NULL)))
    return i;

  interruptible_sleep_on(&wq_dio);

  i = get_buffered_int(NULL);

  return i;
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

void interruptible_sleep_on(
  rtems_id *id
)
{
  rtems_status_code rc;

  rc = rtems_barrier_wait(*id, 0);
}

void wake_up_interruptible(
  rtems_id *id
)
{
  rtems_status_code rc;
  uint32_t          unblocked;

  rc = rtems_barrier_release(*id, &unblocked);
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
  BSP_irq_disable_at_i8259s(irq->name - BSP_IRQ_VECTOR_BASE);
}
static void pcmmio_irq_enable(const rtems_irq_connect_data *irq)
{
  BSP_irq_enable_at_i8259s(irq->name - BSP_IRQ_VECTOR_BASE);
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

/*
 * RTEMS specific initialization routine
 */
void pcmmio_initialize(
  unsigned short _base_port,
  unsigned short _irq
)
{
  /* hardware configuration information */
  base_port = _base_port;
  irq       = _irq;

  /* Create RTEMS Objects */
  pcmmio_barrier_create( rtems_build_name( 'a', '2', 'd', '1' ), &wq_a2d_1 );
  pcmmio_barrier_create( rtems_build_name( 'd', 'a', 'c', '1' ), &wq_dac_1 );
  pcmmio_barrier_create( rtems_build_name( 'd', 'a', 'c', '2' ), &wq_dac_2 );
  pcmmio_barrier_create( rtems_build_name( 'd', 'i', 'o', ' ' ), &wq_dio );

  /* install IRQ handler */
  if ( irq ) {
    int status = 0;
    pcmmio_irq.name = irq;
    #if defined(BSP_SHARED_HANDLER_SUPPORT)
      BSP_install_rtems_shared_irq_handler( &pcmmio_irq );
    #else
      printk( "PCMMIO Installing IRQ handler as non-shared\n" );
      BSP_install_rtems_irq_handler( &pcmmio_irq );
    #endif
    if ( !status ) {
      printk("Error installing PCMMIO interrupt handler!\n" );
      rtems_fatal_error_occurred( status );
    }
  }
}

#include <libcpu/cpuModel.h> /* for rdtsc */

/*
 *  From this point down, we should be able to share easily with the Linux
 *  driver but I haven't gone to the trouble to do surgery on it.  I have
 *  no way to test it.
 */

/* We will buffer up the transition interrupts and will pass them on
   to waiting applications
*/

#define MAX_INTS 1024

typedef struct {
  unsigned char      line;
  unsigned long long timestamp;
} DIO_Int_t;

static DIO_Int_t int_buffer[MAX_INTS];
static int       inptr = 0;
static int       outptr = 0;

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
      #ifdef DEBUG
        printk("<1>Buffering DIO interrupt on bit %d\n",int_num);
      #endif

      /*
       * Buffer the interrupt
       *
       * NOTE: No need to worry about disabling interrupts,
       *       we are in interrupts.
       */

      int_buffer[inptr].timestamp = rdtsc();
      int_buffer[inptr].line = int_num;
      inptr++;
      if (inptr == MAX_INTS)
        inptr = 0;

        /* Clear the interrupt */
        clr_int(int_num);
    }

    /* Wake up anybody waiting for a DIO interrupt */
    wake_up_interruptible(&wq_dio);
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

int get_buffered_int(
  unsigned long long *timestamp
)
{
  rtems_interrupt_level level;
  int                   line;

  if (irq == 0) {
    line = get_int();
    if (line)
      clr_int(line);
    return line;
  }

  line = 0;

  rtems_interrupt_disable( level );
    if (outptr != inptr) {
      if ( timestamp )
        *timestamp = int_buffer[outptr].timestamp;
      line = int_buffer[outptr].line;
      outptr++;
      if (outptr == MAX_INTS)
        outptr = 0;
    }
  rtems_interrupt_enable( level );
  
  return line;
}
