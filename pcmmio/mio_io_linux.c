/* mio_io.c
 * WinSystems support module file for the  PCM-MIO Linux driver
 *
 *
 *  $Id$
 *
 *
 *  This file implements all of the supported 'C' language functions.
 *  Where necessary ioctl calls are made into the kernel driver to access
 *  the actual hardware.
 */

/* #define DEBUG 1 */

#include "mio_io.h"    

#include <stdio.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */

/**************************************************************************
*
*		USER LIBRARY FUNCTIONS
*
***************************************************************************
*/

int handle = 0;
char *device_id ="pcmmio";
char *dev_id    = "/dev/pcmmio";

///////////////////////////////////////////////////////////////////////////////
//
//		MIO_READ_IRQ_ASSIGNED
//
//////////////////////////////////////////////////////////////////////////////

int mio_read_irq_assigned(void)
{
unsigned char val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;


	/* All of our programming of the hardware is handled at this level so that 
	   all of the routines that need to shove and IRQ value into hardware will 
	   use this call.
   */

    val = ioctl(handle,READ_IRQ_ASSIGNED,NULL);

	return val & 0xff;
}

///////////////////////////////////////////////////////////////////////////////
//
//		READ_DIO_BYTE
//
//////////////////////////////////////////////////////////////////////////////

unsigned char read_dio_byte(int offset)
{
int val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	/* All bit operations are handled at this level so we need only
	read and write bytes from the actual hardware using the driver
	to handle our ioctl call for it.
	*/

    val = ioctl(handle,READ_DIO_BYTE,offset);

	return (unsigned char) (val & 0xff);;
}

///////////////////////////////////////////////////////////////////////////////
//
//		MIO_READ_REG
//
//////////////////////////////////////////////////////////////////////////////

unsigned char mio_read_reg(int offset)
{
int val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;


	/* This is a catchall register read routine that allows reading of
	   ANY of the registers on the PCM-MIO. It is used primarily for
	   retreiving control and access values in the hardware.
   */

     val = ioctl(handle,MIO_READ_REG,offset);

	return (unsigned char) (val & 0xff);
}

///////////////////////////////////////////////////////////////////////////////
//
//		MIO_WRITE_REG
//
//////////////////////////////////////////////////////////////////////////////

int mio_write_reg(int offset, unsigned char value)
{
unsigned short param;
int val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	param = offset & 0xff;

	param = param | (value << 8);
    
	/* This function like the previous allow unlimited
	   write access to ALL of the registers on the PCM-MIO
   */

	val = ioctl(handle,MIO_WRITE_REG,param);
	
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//		WRITE_DIO_BYTE
//
//////////////////////////////////////////////////////////////////////////////

int write_dio_byte(int offset, unsigned char value)
{
unsigned short param;
int val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	param = offset & 0xff;

	param = param | (value << 8);
    
	/* All bit operations for the DIO are handled at this level
	   and we need the driver to allow access to the actual
	   DIO registers to update the value.
    */

	val = ioctl(handle,WRITE_DIO_BYTE,param);
	
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//		WRITE_DAC_COMMAND
//
//////////////////////////////////////////////////////////////////////////////

int write_dac_command(int dac_num,unsigned char value)
{
unsigned short param;
int val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	param = dac_num & 0xff;

	param = param | (value << 8);

	val = ioctl(handle,WRITE_DAC_COMMAND,param);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		WRITE_ADC_COMMAND
//
//////////////////////////////////////////////////////////////////////////////

int write_adc_command(int adc_num,unsigned char value)
{
unsigned short param;
int ret_val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	param = adc_num & 0xff;

	param = param | (value << 8);

    ret_val = ioctl(handle,WRITE_ADC_COMMAND,param);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		WRITE_DAC_DATA
//
//////////////////////////////////////////////////////////////////////////////

int write_dac_data(int dac_num, unsigned short value)
{
int ret_val;
unsigned char buffer[3];

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	buffer[0] = dac_num;
	buffer[1] = value & 0xff;
	buffer[2] = value >> 8;

    ret_val = ioctl(handle,WRITE_DAC_DATA,buffer);
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		DAC_READ_STATUS
//
//////////////////////////////////////////////////////////////////////////////

unsigned char dac_read_status(int dac_num)
{
int ret_val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

    ret_val = ioctl(handle,READ_DAC_STATUS,dac_num);

	return ret_val & 0xff;
}

///////////////////////////////////////////////////////////////////////////////
//
//		ADC_READ_STATUS
//
//////////////////////////////////////////////////////////////////////////////

unsigned char adc_read_status(int adc_num)
{
int ret_val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

    ret_val = ioctl(handle,READ_ADC_STATUS,adc_num);

	return (ret_val & 0xff);
}

///////////////////////////////////////////////////////////////////////////////
//
//		ADC_READ_CONVERSION_DATA
//
//////////////////////////////////////////////////////////////////////////////

unsigned short adc_read_conversion_data(int channel)
{
int ret_val;
int adc_num;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	if(channel > 7)
		adc_num = 1;
	else
		adc_num = 0;

    ret_val = ioctl(handle,READ_ADC_DATA,adc_num);
	
	return (ret_val & 0xffff);
}


int dio_get_int(void)
{
int c;

    c=ioctl(handle,DIO_GET_INT,NULL);

    return (c & 0xff);

}


int wait_adc_int(int adc_num)
{
int c;


    if(check_handle())   /* Check for chip available */
		return -1;

	if(adc_num)
	    c=ioctl(handle,WAIT_A2D_INT_1,NULL);
	else
	    c=ioctl(handle,WAIT_A2D_INT_2,NULL);


    return (c & 0xff);

}


int wait_dac_int(int dac_num)
{
int c;

    if(check_handle())   /* Check for chip available */
		return -1;

	if(dac_num)
	    c=ioctl(handle,WAIT_DAC_INT_1,NULL);
	else
	    c=ioctl(handle,WAIT_DAC_INT_2,NULL);

    return (c & 0xff);

}


int wait_dio_int(void)
{
int c;


    if(check_handle())   /* Check for chip available */
		return -1;

    c=ioctl(handle,WAIT_DIO_INT,NULL);

    return (c & 0xff);

}



int check_handle(void)
{
    if(handle > 0)	/* If it's already a valid handle */
		return 0;

    if(handle == -1)	/* If it's already been tried */
	{
		mio_error_code = MIO_OPEN_ERROR;
		sprintf(mio_error_string,"MIO - Unable to open device PCMMIO");
		return -1;
	}

    /* Try opening the device file, in case it hasn't been opened yet */

    handle = open(device_id,0);

	/* Try an alternate open at /dev */

	if(handle < 0)
		handle = open(dev_id,0);

    if(handle > 0)	/* If it's now a valid handle */
		return 0;

	mio_error_code = MIO_OPEN_ERROR;
	sprintf(mio_error_string,"MIO - Unable to open device PCMMIO");
    handle = -1;
	return -1;
}

