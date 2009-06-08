/* PCMMIO.C    WinSystems PCM-MIO Linux Driver */

/* 
 * $Header$
 * $Id$
 *
 * Filename : $RCSfile$
 * 
 * $Log$
 *
 *
 * WinSystems PCM-MIO Linux Device Driver
 */

 static char *RCSInfo = "$Id$";

/* Portions of original code Copyright (C) 1998-99 by Ori Pomerantz */


/* #define DEBUG 1 */

#ifndef __KERNEL__
#  define __KERNEL__
#endif

#ifndef MODULE
#  define MODULE
#endif

#include <linux/config.h>
#include <linux/module.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/uaccess.h>


MODULE_LICENSE("GPL");

/* Our own ioctl numbers */

#include "mio_io.h"

#define SUCCESS 0

#define MAX_INTS 1024

/* Function prototypes for local functions */

int get_buffered_int(void);
void init_io(unsigned short io_address);
void clr_int(int bit_number);
int get_int(void);


/* Interrupt handler */

irqreturn_t mio_handler(int, void *, struct pt_regs *);

void common_handler(void);


/* This holds the base addresses of the board */

unsigned short base_port = 0;

/* The name for our device, as it will appear in 
 * /proc/devices */

#define MAJOR_NUM 115
#define DEVICE_NAME "pcmmio"

/* The maximum length of the message for the device */

#define BUF_LEN 40

/* The message the device will give when asked */

static char Message[BUF_LEN];

/* A message pointer for the device */

int Message_Ptr = 0;

/* Our insmod command line arguments */

MODULE_PARM(io,"1-6i");
MODULE_PARM(irq,"1-6i");

static unsigned short io = 0;
static unsigned short irq = 0;


/* We will buffer up the transition interrupts and will pass them on
   to waiting applications
*/

unsigned char int_buffer[MAX_INTS];
int inptr = 0;
int outptr = 0;


/* These declarations create the wait queues. One for each supported
   device.
*/

DECLARE_WAIT_QUEUE_HEAD(wq_a2d_1);
DECLARE_WAIT_QUEUE_HEAD(wq_a2d_2);
DECLARE_WAIT_QUEUE_HEAD(wq_dac_1);
DECLARE_WAIT_QUEUE_HEAD(wq_dac_2);
DECLARE_WAIT_QUEUE_HEAD(wq_dio);

unsigned char adc2_port_image = 0;

/* This is the common interrupt handler. It is called by the hctual hardware ISR.
*/

void common_handler()
{
unsigned char status;
unsigned char int_num;


			/* Read the interrupt ID register from ADC2 */

			adc2_port_image = adc2_port_image | 0x20;
			outb(adc2_port_image,base_port + 0x0f);	

			status = inb(base_port + 0x0f);

			if(status & 1)
			{
				/* Clear ADC1 interrupt */
				inb(base_port+1);			/* Clear interrupt */

				/* Wake up any holding processes */

				wake_up_interruptible(&wq_a2d_1);
			}

			if(status & 2)
			{			
				/* Clear ADC1 interrupt */

				inb(base_port+5);			/* Clear interrupt */

				/* Wake up anybody waiting for ADC1 */

				wake_up_interruptible(&wq_a2d_2);
			}

			if(status & 4)
			{
				/* Clear DAC1 interrupt */

				inb(base_port+9);		/* Clear interrupt */

				/* Wake up if you're waiting on DAC1 */

				wake_up_interruptible(&wq_dac_1);
			}

			if(status & 8)
			{

				/* DIO interrupt. Find out which bit */

				int_num = get_int();
				if(int_num)
				{
#ifdef DEBUG
					printk("<1>Buffering DIO interrupt on bit %d\n",int_num);
#endif

					/* Buffer the interrupt */

					int_buffer[inptr++] = int_num;
					if(inptr == MAX_INTS)
						inptr = 0;
				
					/* Clear the interrupt */

					clr_int(int_num);
				}

				/* Wake up anybody waiting for a DIO interrupt */

				wake_up_interruptible(&wq_dio);
			}

			if(status & 0x10)
			{
				/* Clear DAC2 Interrupt */

				inb(base_port+0x0d);		/* Clear interrupt */

				/* Wake up DAC2 holding processes */

				wake_up_interruptible(&wq_dac_2);
			}


			/* Reset the access to the interrupt ID register */

			adc2_port_image = adc2_port_image & 0xdf;
			outb(adc2_port_image,base_port+0x0f);	

}



/* This is the actual hadrware interrupt handler. It defers to another
   functon for handling.
*/

irqreturn_t mio_handler(int pirq, void *dev_id, struct pt_regs *regs)
{

#ifdef DEBUG
	printk("<1>MIO Interrupt received\n");
#endif

	common_handler();
	return IRQ_HANDLED;
}



/***********************************************************************
*
*
*			DEVICE OPEN
*
*
************************************************************************
*/


/* This function is called whenever a process attempts 
 * to open the device file */

static int device_open(struct inode *inode, struct file *file)
{

  if(base_port == 0)
  {
	printk("<1>**** OPEN ATTEMPT on uninitialized port *****\n");
	return -1;
  }

#ifdef DEBUG
  printk ("device_open(%p)\n", file);
#endif

  /* Rewind the message pointer for this device on a new open.

     This could cause problems for applications that use file I/O
     for talking to the device as we do not limit the number of
     applications that can open the device. The solution is to use
     IOCTL functions to control the device rather than file I/O
     calls.
  */

  Message_Ptr = 0;


  return SUCCESS;
}


/*************************************************************************
*
*
*			DEVICE CLOSE
*
*
**************************************************************************
*/


int device_release(struct inode *inode, struct file *file)
{

#ifdef DEBUG
  printk ("device_release(%p,%p)\n", inode, file);
#endif 


  return 0;

}


/****************************************************************************
*
*			DEVICE READ
*
*
*
*****************************************************************************
*/

ssize_t device_read(struct file *file,char *buffer,size_t length,loff_t *offset)
{
unsigned short port;
int bytes_read;
int x;

   /* We only allow reading of the 32 valid ports in the device. Beyond
      that we return EOF.
   */

   if(Message_Ptr == 32)
 	return 0;	/* EOF Condition */

   port = base_port;

   // Do a fresh read of the ports */

   for(x=0; x<32; x++)
       Message[x] = inb(port+x);


  /* Number of bytes actually written to the buffer */

   bytes_read = 0;

#ifdef DEBUG
  printk("device_read(%p,%p,%d)\n",file, buffer, length);
#endif


  /* Actually put the data into the buffer */

  while (length && (Message_Ptr < 32))
  {
    /* Because the buffer is in the user data segment, 
     * not the kernel data segment, assignment wouldn't 
     * work. Instead, we have to use put_user which 
     * copies data from the kernel data segment to the 
     * user data segment. */

    put_user((Message[Message_Ptr++]), buffer++);
    length --;
    bytes_read ++;
  }

#ifdef DEBUG
   printk ("Read %d bytes, %d left\n",bytes_read, length);
#endif

   /* Read functions are supposed to return the number 
    * of bytes actually inserted into the buffer */


  /* By updating file->f_pos we can allow seeking of our Messge_Ptr
     which will allow random access to the ports using file I/O
  */

  file->f_pos = (loff_t)Message_Ptr;

  return bytes_read;
}


/****************************************************************************
*
*
*				DEVICE WRITE
*
*****************************************************************************
*/

ssize_t device_write(struct file *file,const char *buffer,size_t length,loff_t *offset)
{
int i;
unsigned short port;

    port = base_port;

#ifdef DEBUG
  printk ("device_write(%p,%s,%d)",file, buffer, length);
#endif

  /* Read in the bytes to write, one at a time */

  for(i=0; i<length && i<BUF_LEN; i++)
  {
    get_user(Message[i], buffer+i);

    /* We output each byte to it's appropriate port as it comes in */

	if(Message_Ptr == 32)
	{
         file->f_pos = (loff_t) Message_Ptr;
	     return i;
	}


    outb(Message[i],port + Message_Ptr++);

  }

  /* As with device_read if we update the file position. Seeking will
     work as expected.
  */

  file->f_pos = (loff_t) Message_Ptr;


  /* Again, return the number of input characters used */

    return i;
}


/****************************************************************************
*
*
*
*			DEVICE IOCTL
*
*
*****************************************************************************
*/


int device_ioctl(struct inode *inode,struct file *file,
     unsigned int ioctl_num, unsigned long ioctl_param)
{
int i;
unsigned short word_val;
unsigned char byte_val;
unsigned char offset_val;
unsigned char *arguments;

#ifdef DEBUG
//  printk("PCMMIO - IOCTL call IOCTL CODE %04X\n",ioctl_num);
#endif


  /* Switch according to the ioctl called */

  switch (ioctl_num)
	{

	case WRITE_DAC_DATA:

		arguments = (char *) ioctl_param;

		if(!access_ok(VERIFY_WRITE,arguments,3))
		{
		    printk("<1>pcmmio : Unable to access IOCTL argument memory\n");
		    return -EFAULT;
		}

		byte_val = arguments[0];

		word_val = arguments[2];
		word_val = word_val << 8;
		word_val = word_val | arguments[1];

		if(byte_val)		/* DAC 1 */
			outw(word_val,base_port+0x0c);
		else
			outw(word_val,base_port+8);

		break;


	case	READ_DAC_STATUS:

		byte_val = ioctl_param & 0xff;		/* This is the dac number */

		if(byte_val)
			i = inb(base_port + 0x0f);
		else
			i = inb(base_port + 0x0b);

		return i;

		break;


	case	WRITE_DAC_COMMAND:

		byte_val = ioctl_param & 0xff;		/* This is the DAC number */
		offset_val = ioctl_param >> 8;		/* This is the data value */

		if(byte_val)
			outb(offset_val,base_port + 0x0e);
		else
			outb(offset_val,base_port + 0x0a);

		break;


	case	WRITE_ADC_COMMAND:

		byte_val = ioctl_param & 0xff;		/* This is the ADC number */
		offset_val = ioctl_param >> 8;		/* This is the data value */

		if(byte_val)
			outb(offset_val,base_port + 0x06);
		else
			outb(offset_val,base_port + 0x02);

		break;



	case	READ_ADC_DATA:

		byte_val = ioctl_param & 0xff;	/* This is the ADC number */

		if(byte_val)
			word_val = inw(base_port + 4);
		else
			word_val = inw(base_port);

		return word_val;

		break;


	case	READ_ADC_STATUS:

		byte_val = ioctl_param & 0xff;		/* This is the ADC number */
        
		if(byte_val)
			i = inb(base_port + 7);
		else
			i = inb(base_port + 3);

		return i;

		break;


	case	WRITE_DIO_BYTE:

		offset_val = ioctl_param & 0xff;
		byte_val = ioctl_param >> 8;
		outb(byte_val, base_port + 0x10 + offset_val);

		break;


	case	READ_DIO_BYTE:

		offset_val = ioctl_param & 0xff;
		byte_val = inb(base_port + 0x10 + offset_val);
		return (byte_val & 0xff);

		break;


	case	MIO_WRITE_REG:

		offset_val = ioctl_param & 0xff;
		byte_val = ioctl_param >> 8;
		outb(byte_val, base_port + offset_val);

		break;


	case	MIO_READ_REG:

		offset_val = ioctl_param & 0xff;
		byte_val = inb(base_port + offset_val);
		return (byte_val & 0xff);

		break;


	case	WAIT_A2D_INT_1:

#ifdef DEBUG
		printk("<1>PCMMIO - IOCTL wait_ad1_int \n");
		printk("<1>PCMMIO : current process %i (%s) going to sleep\n",
		current->pid,current->comm);
#endif

		interruptible_sleep_on(&wq_a2d_1);
		break;

	case	WAIT_A2D_INT_2:


#ifdef DEBUG
		printk("<1>PCMMIO - IOCTL wait_ad2_int \n");
		printk("<1>PCMMIO : current process %i (%s) going to sleep\n",
		current->pid,current->comm);
#endif

		interruptible_sleep_on(&wq_a2d_2);
		break;

	case	WAIT_DAC_INT_1:

#ifdef DEBUG
		printk("<1>PCMMIO - IOCTL wait_dac1_int \n");
		printk("<1>PCMMIO : current process %i (%s) going to sleep\n",
		current->pid,current->comm);
#endif
		interruptible_sleep_on(&wq_dac_1);
		break;

	case	WAIT_DAC_INT_2:

#ifdef DEBUG
		printk("<1>PCMMIO - IOCTL wait_dac2_int \n");
		printk("<1>PCMMIO : current process %i (%s) going to sleep\n",
		current->pid,current->comm);
#endif
		interruptible_sleep_on(&wq_dac_2);
		break;

	case	WAIT_DIO_INT:

		if((i = get_buffered_int()))
			return i;

#ifdef DEBUG
		printk("<1>PCMMIO - IOCTL wait_dio_int \n");
		printk("<1>PCMMIO : current process %i (%s) going to sleep\n",
		current->pid,current->comm);
#endif
		interruptible_sleep_on(&wq_dio);

		i = get_buffered_int();

		return i;
		break;

	case READ_IRQ_ASSIGNED:

		return(irq & 0xff);
		break;

	case DIO_GET_INT:

		i = get_buffered_int();
		return(i & 0xff);
		break;


	 }

  return SUCCESS;
}

/***************************** DEVICE_LSEEK ***************************/

loff_t device_lseek(struct file *filp, loff_t off, int whence)
{
unsigned short offset;

    /* Convert the offset value to a more familiar size. After all
       our device is a fixed size of 10 bytes.
    */

    offset = (unsigned short)off;

#ifdef DEBUG
    printk("<1>PCMMIO:lseek - offset %d mode %d\n",offset,whence);
#endif

    switch(whence)
    {
	case 0:   /* Seek set */
	    Message_Ptr = offset;
		if(Message_Ptr > 32)
			Message_Ptr = 32;
		break;

	case 1:   /* Seek from Current */
	    Message_Ptr += offset;
		if(Message_Ptr > 32)
			Message_Ptr = 32;

		break;
	
	default:	  /* Seek from End */
	    return -EINVAL;
	    break;
    }

	if(Message_Ptr < 0)
	{
	    Message_Ptr = 0;
	    filp->f_pos = (loff_t)Message_Ptr;
	    return -EINVAL;
	}

	if(Message_Ptr > 32)
	{
	    Message_Ptr = 32;
	    filp->f_pos = (loff_t)Message_Ptr;
	    return -EINVAL;
	}

	filp->f_pos = (loff_t)Message_Ptr;
	return (loff_t)Message_Ptr;
}

/* ******************* Module Declarations *************************** */


/* This structure will hold the functions to be called 
 * when a process does something to the our device 
*/

struct file_operations Fops = {

  owner:	THIS_MODULE,
  llseek:	device_lseek,
  read:		device_read, 
  write:	device_write,
  ioctl:	device_ioctl,
  open:		device_open,
  release:	device_release,
};

/****************************************************************************
*
*			INIT_MODULE
*
*
*****************************************************************************
*/

/* Initialize the module - Register the character device */

int init_module()
{
int ret_val;

  /* Sign-on */

   printk("<1>\nWinSystems PCM-MIO Driver. Copyright 2006. All rights reserved\n");
   printk("<1>%s\n",RCSInfo);


  /* Check and Map our I/O region requests */

	if(io != 0)
    {
		if(request_region(io,0x20,"PCMMIO") == NULL)
		{
			printk("<1>PCMMIO - Unable to use I/O Address %04X\n",io);
           	io = 0;
		}
  	/* Register the character device */

  		ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

  		/* Negative values signify an error */
 
 		if (ret_val < 0)
		{
    		printk (" %s failed with %d\n",",registering the character device PCMMIO ", ret_val);
    		return ret_val;
  		}
	
		printk ("\nPCMMIO : The Base I/O Address = %04X\n",io);
		base_port = io;
		init_io(io);
	}

      if((io != 0) && (irq != 0))
      {
			if(request_irq(irq,mio_handler,SA_SHIRQ,"PCMMIO",RCSInfo))
			printk("<1>PCMMIO - Unable to register IRQ %d\n",irq);
		else
			printk("<1>PCMMIO - IRQ %d registered ok Chip 1\n",irq);

	  }
 
	return 0;
}

/***************************************************************************
*
*			CLEANUP_MODULE
*
****************************************************************************
*/

/* Cleanup - unregister the appropriate file from /proc */

void cleanup_module()
{
  /* Unregister I/O Port usage */

   	if(irq)
		free_irq(irq,RCSInfo);
     
	if(io)
      	release_region(io,0x20);

  /* Unregister the device */

	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
 
}  


/* start of device subroutines */



/* This array holds the image values of the last write to each I/O
   port. This allows bit manipulation routines to work without having to actually do
   a read-modify-write to the I/O port.
*/

unsigned char port_images[6];


void init_io(unsigned short io_address)
{
int x;
unsigned short port;

	/* save the address for later use */

	port = io_address + 0X10;

	/* Clear all of the I/O ports. This also makes them inputs */

	for(x=0; x < 7; x++)
		outb(0,port+x);

	/* Clear the image values as well */

	for(x=0; x < 6; x++)
		port_images[x] = 0;

	/* Set page 2 access, for interrupt enables */

	outb(0x80,port+7);

	/* Clear all interrupt enables */

	outb(0,port+8);
	outb(0,port+9);
	outb(0,port+0x0a);

	/* Restore page 0 register access */

	outb(0,port+7);
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

	if((temp & 7) == 0)
	    return 0;

	/* There is something pending, now we need to identify it */

	/* Set access to page 3 for interrupt id register */

	outb(0xc0, dio_port + 7);

	/* Read the interrupt ID register for port 0 */

	temp = inb(dio_port+8);

	/* See if any bit set, if so return the bit number */

	if(temp != 0)
	{
	    for(x=0; x<=7; x++)
	    {
            if(temp & (1 << x))
	     	{
		    outb(0,dio_port+7);
		    return(x+1);
            }
         }
     }

	/* None in port 0, read port 1 interrupt ID register */

	temp = inb(dio_port+9);

	/* See if any bit set, if so return the bit number */

	if(temp != 0)
	{
	    for(x=0; x<=7; x++)
	    {
		if(temp & (1 << x))
		{
		    outb(0,dio_port+7);
		    return(x+9);
		}
	    }
	}

	/* Lastly, read the statur of port 2 interrupt ID register */

	temp = inb(dio_port+0x0a);

	/* If any pending, return the appropriate bit number */

	if(temp != 0)
	{
	    for(x=0; x<=7; x++)
	    {
			if(temp & (1 << x))
			{
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

	if(irq == 0)
	{
	    temp = get_int();
	    if(temp)
			clr_int(temp);
	    return temp;
	}

	if(outptr != inptr)
	{
	    temp = int_buffer[outptr++];
	    if(outptr == MAX_INTS)
			outptr = 0;
	    return temp;
	}

	return 0;
}

	
