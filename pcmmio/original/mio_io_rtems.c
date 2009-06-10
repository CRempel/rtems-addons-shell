/* mio_io.c WinSystems support module file for the  PCM-MIO RTEMS driver */
/*
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

#include <i386_io.h>

/*
 *  These have to be configured SOMEHOW
 */

/* IRQ source or 0 ==> polled */
static unsigned short irq = 0;
/* This holds the base addresses of the board */
static unsigned short base_port = 0;

/* for RTEMS */
void pcmmio_initialize(
  unsigned short _base_port,
  unsigned short _irq
)
{
  base_port = _base_port;
  irq       = _irq;
}

/* Function prototypes for local functions */
int get_buffered_int(void);
void init_io(unsigned short io_address);
void clr_int(int bit_number);
int get_int(void);

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


int dio_get_int(void)
{
  if (check_handle())   /* Check for chip available */
    return -1;

  return get_buffered_int() & 0xff;

}

static int handle = 0; /* XXX move to lower */


int wait_adc_int(int adc_num)
{
  int c;

  if (check_handle())   /* Check for chip available */
    return -1;

  if (adc_num)
      c=ioctl(handle,WAIT_A2D_INT_1,NULL);
  else
      c=ioctl(handle,WAIT_A2D_INT_2,NULL);


  return (c & 0xff);

}


int wait_dac_int(int dac_num)
{
  int c;

  if (check_handle())   /* Check for chip available */
    return -1;

  if (dac_num)
      c=ioctl(handle,WAIT_DAC_INT_1,NULL);
  else
      c=ioctl(handle,WAIT_DAC_INT_2,NULL);

  return (c & 0xff);

}


int wait_dio_int(void)
{
  int c;

  if (check_handle())   /* Check for chip available */
    return -1;

  c=ioctl(handle,WAIT_DIO_INT,NULL);

  return (c & 0xff);

}


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
 *  From this point down, we should be able to share easily with the Linux
 *  driver but I haven't gone to the trouble to do surgery on it.  I have
 *  no way to test it.
 */

/* We will buffer up the transition interrupts and will pass them on
   to waiting applications
*/

#define MAX_INTS 1024

static unsigned char int_buffer[MAX_INTS];
static int inptr = 0;
static int outptr = 0;

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

  /* Lastly, read the statur of port 2 interrupt ID register */
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


int get_buffered_int(void)
{
  int temp;

  if (irq == 0) {
    temp = get_int();
    if (temp)
      clr_int(temp);
    return temp;
  }

  if (outptr != inptr) {
    temp = int_buffer[outptr++];
    if (outptr == MAX_INTS)
      outptr = 0;
    return temp;
  }

  return 0;
}





