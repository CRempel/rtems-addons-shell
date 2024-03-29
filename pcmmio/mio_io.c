/* mio_io.c
 * WinSystems support module file for the PCM-MIO Linux and RTEMS drivers
 *
 *  $Id$
 *
 *  This file implements all of the supported 'C' language functions.
 *  Where necessary calls are made into a porting layer to access the
 *  actual hardware.
 */

#define LIB_DEFINED

/* #define DEBUG 1 */

#include "mio_io.h"    

#include <stdio.h>

/* These image variable help out where a register is not
   capable of a read/modify/write operation
*/

unsigned char dio_port_images[8];
unsigned char adc1_port_image =0;
unsigned char adc2_port_image =0;
unsigned char dac1_port_image =0;
unsigned char dac2_port_image =0;


/* The channel selects on the ADC are non contigous. In order to avoid shifting and such
   with each channel select, we simple bild an array for selection.
*/

unsigned char adc_channel_select[16] = {ADC_CH0_SELECT, ADC_CH1_SELECT, ADC_CH2_SELECT,
	ADC_CH3_SELECT, ADC_CH4_SELECT, ADC_CH5_SELECT, ADC_CH6_SELECT, ADC_CH7_SELECT,
	ADC_CH0_SELECT, ADC_CH1_SELECT, ADC_CH2_SELECT, ADC_CH3_SELECT, ADC_CH4_SELECT,
	ADC_CH5_SELECT, ADC_CH6_SELECT, ADC_CH7_SELECT };

/* Mode selection can also be time consuming and we'd also like the mode to "Stick" from
   call to call. This array will eventually hold the actual command byte to send to the
   ADC controller for each channel according to the mode set with adc_set_channel_mode
*/

unsigned char adc_channel_mode[16] = {ADC_CH0_SELECT, ADC_CH1_SELECT, ADC_CH2_SELECT,
	ADC_CH3_SELECT, ADC_CH4_SELECT, ADC_CH5_SELECT, ADC_CH6_SELECT, ADC_CH7_SELECT,
	ADC_CH0_SELECT, ADC_CH1_SELECT, ADC_CH2_SELECT, ADC_CH3_SELECT, ADC_CH4_SELECT,
	ADC_CH5_SELECT, ADC_CH6_SELECT, ADC_CH7_SELECT };


/* This array and the index value are used internally for the adc_convert_all_channels
  and for the adc_buffered_conversion routines.
*/
unsigned char adc_channel_buff[18] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,15,0xff,0xff};
unsigned char *adc_input_buffer;
unsigned short *adc_user_buffer;

/* Becaues of the nature of the ADC beast. It's necessary to keep track of the last
  channel and the previous channel in order to retrieve the data amd know who it belongs to
*/

int adc_last_channel, adc_current_channel;

/* Various index pointers for the above arrays and misc globals */

int adc_ch_index =0;
int adc_out_index=0;
int adc_repeat_channel;
int adc_repeat_count;





///////////////////////////////////////////////////////////////////////////////
//
//		DISABLE_DIO_INTERRUPT
//
//////////////////////////////////////////////////////////////////////////////

int disable_dio_interrupt(void)
{
	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	adc1_port_image = adc1_port_image | 0x10;
	mio_write_reg(0x03, adc1_port_image);	// Access the int enable register
	mio_write_reg(0x02, 0);
	adc1_port_image = adc1_port_image & 0xef;
	mio_write_reg(0x03,adc1_port_image );	// Disable the interrupt
	
	return(0);
}


///////////////////////////////////////////////////////////////////////////////
//
//		ENABLE_DIO_INTERRUPT
//
//////////////////////////////////////////////////////////////////////////////

int enable_dio_interrupt(void)
{
unsigned char vector;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	/* We read the assign IRQ from the driver, so we can program the hardware to
	   match,  but only if an aaplication desires such.
    */

	vector = mio_read_irq_assigned();

	if(vector == 0)
	{
		mio_error_code = MIO_MISSING_IRQ;
		sprintf(mio_error_string,"MIO(DIO) : enable_dio_interrupt - No IRQ assigned");
		return(1);
	}

	adc1_port_image = adc1_port_image | 0x10;
	mio_write_reg(0x03, adc1_port_image);	// Access the int enable register
	mio_write_reg(0x02, vector);
	adc1_port_image = adc1_port_image & 0xef;
	mio_write_reg(0x03, adc1_port_image);	// Enable the interrupt

	return(0);
}


///////////////////////////////////////////////////////////////////////////////
//
//		DISABLE_DAC_INTERRUPT
//
//////////////////////////////////////////////////////////////////////////////

int disable_dac_interrupt(int dac_num)
{
	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	if(dac_num)
	{
		dac2_port_image = dac2_port_image & 0xfe;
		dac2_port_image = dac2_port_image | 0x08;
		mio_write_reg(0x0f, dac2_port_image);	// Access the int enable register
		mio_write_reg(0x0e, 0);
		dac2_port_image = dac2_port_image & 0xf7;
		mio_write_reg(0x0f, dac2_port_image);	// Disable the interrupt
	}
	else
	{
		dac1_port_image = dac1_port_image & 0xfe;
		dac1_port_image = dac1_port_image | 0x08;
		mio_write_reg(0x0b, dac1_port_image);	// Access the int enable register
	
		mio_write_reg(0x0a, 0);

		dac1_port_image = dac1_port_image & 0xf7;
		mio_write_reg(0x0b, dac1_port_image);	// Disable the interrupt
	}
	return(0);
}

///////////////////////////////////////////////////////////////////////////////
//
//		ENABLE_DAC_INTERRUPT
//
//////////////////////////////////////////////////////////////////////////////

int enable_dac_interrupt(int dac_num)
{
unsigned char vector;


	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	/* We read the assign IRQ from the driver, so we can program the hardware to
	   match,  but only if an aaplication desires such.
    */

	vector = mio_read_irq_assigned();

	if(vector == 0)
	{
		mio_error_code = MIO_MISSING_IRQ;
		sprintf(mio_error_string,"MIO(DAC) : enable_dac_interrupt - No IRQ assigned");
		return(1);
	}


	if(dac_num)
	{
		dac2_port_image = dac2_port_image & 0xfe;
		dac2_port_image = dac2_port_image | 0x08;
		mio_write_reg(0x0f, dac2_port_image);	// Access the int enable register

		mio_write_reg(0x0e, vector);

		dac2_port_image = dac2_port_image & 0xf7;
		dac2_port_image = dac2_port_image | 0x01;
		mio_write_reg(0x0f, dac2_port_image);	// Enable the interrupt
	}
	else
	{
		dac1_port_image = dac1_port_image & 0xfe;
		dac1_port_image = dac1_port_image | 0x08;
		mio_write_reg(0x0b, dac1_port_image);	// Access the int enable register

		mio_write_reg(0x0a, vector);

		dac1_port_image = dac1_port_image & 0xf7;
		dac1_port_image = dac1_port_image | 0x01;
		mio_write_reg(0x0b, dac1_port_image);	// Enable the interrupt
	}
	return(0);
}



///////////////////////////////////////////////////////////////////////////////
//
//		DISABLE_ADC_INTERRUPT
//
//////////////////////////////////////////////////////////////////////////////

int disable_adc_interrupt(int adc_num)
{

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	if(adc_num)
	{
		adc2_port_image = adc2_port_image & 0xfe;
		adc2_port_image = adc2_port_image | 0x08;
		mio_write_reg(0x07, adc2_port_image);	// Access the int enable register

		mio_write_reg(0x06, 0);

		adc2_port_image = adc2_port_image & 0xf7;
		mio_write_reg(0x07, adc2_port_image);	// Disable the interrupt
	}
	else
	{
		adc1_port_image = adc1_port_image & 0xfe;
		adc1_port_image = adc1_port_image | 0x08;
		mio_write_reg(0x03, adc1_port_image);	// Access the int enable register

		mio_write_reg(0x02, 0);

		adc1_port_image = adc1_port_image & 0xf7;
		mio_write_reg(0x03, adc1_port_image);	// Disable the interrupt
	}
	return(0);
}


///////////////////////////////////////////////////////////////////////////////
//
//		ENABLE_ADC_INTERRUPT
//
//////////////////////////////////////////////////////////////////////////////

int enable_adc_interrupt(int adc_num)
{
unsigned char vector;


	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	/* We read the assign IRQ from the driver, so we can program the hardware to
	   match,  but only if an aaplication desires such.
    */

	vector = mio_read_irq_assigned();

	if(vector == 0)
	{
		mio_error_code = MIO_MISSING_IRQ;
		sprintf(mio_error_string,"MIO(ADC) : enable_adc_interrupt - No IRQ assigned");

		return(1);
	}

	if(adc_num)
	{
		adc2_port_image = adc2_port_image & 0xfe;
		adc2_port_image = adc2_port_image | 0x08;
		mio_write_reg(0x07, adc2_port_image);	// Access the int enable register

		mio_write_reg(0x06, vector);

		adc2_port_image = adc2_port_image & 0xf7;
		adc2_port_image = adc2_port_image | 0x01;
		mio_write_reg(0x07, adc2_port_image);	// Enable the interrupt
	}
	else
	{
		adc1_port_image = adc1_port_image & 0xfe;
		adc1_port_image = adc1_port_image | 0x08;
		mio_write_reg(0x03, adc1_port_image);	// Access the int enable register

		mio_write_reg(0x02, vector);

		adc1_port_image = adc1_port_image & 0xf7;
		adc1_port_image = adc1_port_image | 0x01;
		mio_write_reg(0x03, adc1_port_image);	// Enable the interrupt
	}
	return(0);
}


///////////////////////////////////////////////////////////////////////////////
//
//		SET_DAC_SPAN
//
//////////////////////////////////////////////////////////////////////////////

int set_dac_span(int channel, unsigned char span_value)
{
unsigned char select_val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	if((channel < 0) || (channel > 7))
	{
		mio_error_code = MIO_BAD_CHANNEL_NUMBER;
		sprintf(mio_error_string,"MIO(DAC) : Set_dac_span - bad channel number %d",channel);
		return(1);
	}


	/* This function sets up the output range for the DAC channel */

	select_val = (channel % 4) << 1;

	write_dac_data(channel / 4, span_value);
	if(mio_error_code)
		return 1;

	write_dac_command(channel / 4, 0x60 | select_val);
	if(mio_error_code)
		return(1);

	if(wait_dac_ready(channel))
		return 1;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		WAIT_DAC_READY
//
//////////////////////////////////////////////////////////////////////////////

int wait_dac_ready(int channel)
{
unsigned long retry;

	retry = 100000L;

	/* This may seem like an absurd way to handle waiting and violates the
	"no busy waiting" policy. The fact is that the hardware is normally so fast that we
	usually only need one time through the loop anyway. The longer timeout is for rare
	occasions and for detecting non-existant hardware.
	*/

	while(retry--)
	{
		if(dac_read_status(channel / 4) & DAC_BUSY)
			return 0;

	}
	return 1;

}


///////////////////////////////////////////////////////////////////////////////
//
//		SET_DAC_OUTPUT
//
//////////////////////////////////////////////////////////////////////////////

int set_dac_output(int channel, unsigned short dac_value)
{
unsigned char select_val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	select_val = (channel % 4) << 1;
	write_dac_data(channel / 4, dac_value);
	if(mio_error_code)
		return(1);
	write_dac_command(channel / 4, 0x70 | select_val);
	if(mio_error_code)
		return(1);

	if(wait_dac_ready(channel))
		return 1;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		SET_DAC_VOLTAGE
//
//////////////////////////////////////////////////////////////////////////////

int set_dac_voltage(int channel, float voltage)
{
unsigned short  value = 0;
float bit_val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	/* This output function is auto-ranging in that it picks the span that will
	give the most precision for the voltage specified. This has one side-effect that
	may be objectionable to some applications. When call to set_dac_span completes the
	new range is set and the output will respond immediately using whatever value was last
	in the output registers. This may cause a spike (up or down) in the DAC output until the
	new output value is sent to the controller.
	*/

	if((voltage < -10.0) || (voltage > 10.0))
	{
		mio_error_code = MIO_ILLEGAL_VOLTAGE;
		sprintf(mio_error_string,"MIO(DAC) :Set DAC Voltage - Illegal Voltage %9.5f",voltage);
		return 1;
	}

	if((voltage >= 0.0) && (voltage < 5.0))
	{
		set_dac_span(channel,DAC_SPAN_UNI5);
		if(mio_error_code)
			return(1);
		bit_val = 5.0 / 65536;
		value = (unsigned short) (voltage / bit_val);
	}

	if(voltage >= 5.0)
	{
		set_dac_span(channel,DAC_SPAN_UNI10);
		if(mio_error_code)
			return(1);
		bit_val = 10.0 / 65536;
		value = (unsigned short) (voltage / bit_val);
	}

	if((voltage < 0.0) && (voltage > -5.0))
	{
		set_dac_span(channel, DAC_SPAN_BI5);
		if(mio_error_code)
			return(1);
		bit_val = 10.0 / 65536;
		value = (unsigned short) ((voltage + 5.0) / bit_val);
	}

	if(voltage <= -5.0)
	{
		set_dac_span(channel, DAC_SPAN_BI10);
		if(mio_error_code)
			return(1);
		bit_val = 20.0 / 65536;
		value  = (unsigned short) ((voltage + 10.0) / bit_val);
	}

	if(wait_dac_ready(channel))
		return 1;

	set_dac_output(channel,value);
	if(mio_error_code)
		return(1);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		ADC_START_CONVERSION
//
//////////////////////////////////////////////////////////////////////////////

int adc_start_conversion(int channel)
{
	mio_error_code = MIO_SUCCESS;

	if((channel <0) || (channel > 15))
	{
		mio_error_code = MIO_BAD_CHANNEL_NUMBER;
		sprintf(mio_error_string,"MIO(ADC) : Start conversion bad channel number %d",channel);
		return(1);
	}

	adc_last_channel = adc_current_channel;
	adc_current_channel = channel;

	write_adc_command(channel / 8,adc_channel_mode[channel]);
	
	if(mio_error_code)
		return(1);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		ADC_GET_CHANNEL_VOLTAGE
//
//////////////////////////////////////////////////////////////////////////////

float adc_get_channel_voltage(int channel)
{
unsigned short value;
float result;

	mio_error_code = MIO_SUCCESS;

	// Start two conversions so that we can have current data

	adc_start_conversion(channel);
	if(mio_error_code)
		return(0.0);

	adc_wait_ready(channel);
	if(mio_error_code)
		return(0.0);

	adc_start_conversion(channel);
	if(mio_error_code)
		return(0.0);

	adc_wait_ready(channel);
	if(mio_error_code)
		return(0.0);

	// Read out the conversion's raw data

	value = adc_read_conversion_data(channel);
	if(mio_error_code)
		return(0.0);

	// Convert the raw data to a voltage 

	value = value + adc_adjust[channel];
	result = value * adc_bitval[channel];
	result = result + adc_offset[channel];

	return(result);
}

///////////////////////////////////////////////////////////////////////////////
//
//		ADC_CONVERT_ALL_CHANNELS
//
//////////////////////////////////////////////////////////////////////////////

int adc_convert_all_channels(unsigned short *buffer)
{
int x;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	// Initialize global variables including transferinng the
	// address of the user's ouput buffer to an internal buffer pointer

	adc_user_buffer = buffer;
	adc_input_buffer = adc_channel_buff;
	adc_ch_index =0;
	adc_out_index = 0;

	adc_start_conversion(0);
	if(mio_error_code)
		return(1);

	adc_wait_ready(0);
	if(mio_error_code)
		return(1);

	// This is old data throw it out

	adc_read_conversion_data(0);
	if(mio_error_code)
		return(1);

	// Finish the rest of the channels

	for(x=1; x<8; x++)
	{
		adc_start_conversion(x);
		if(mio_error_code)
			return(1);

		adc_wait_ready(x);
		if(mio_error_code)
			return(1);

		// Store the results in the user's buffer

		adc_user_buffer[adc_out_index++] = adc_read_conversion_data(x);
		if(mio_error_code)
			return(1);
	}

	// A final dummy conversion is required to get out the last data

	adc_start_conversion(7);
	if(mio_error_code)
		return(1);

	adc_wait_ready(7);
	if(mio_error_code)
		return(1);

	adc_user_buffer[adc_out_index++] = adc_read_conversion_data(7);
	if(mio_error_code)
		return(1);

	// Now start on the second controller

	adc_start_conversion(8);
	if(mio_error_code)
		return(1);

	adc_wait_ready(8);
	if(mio_error_code)
		return(1);

	// This data is old - Throw it out

	adc_read_conversion_data(8);
	if(mio_error_code)
		return(1);

	for(x=9; x<16; x++)
	{
		adc_start_conversion(x);
		if(mio_error_code)
			return(1);

		adc_wait_ready(x);
		if(mio_error_code)
			return(1);

		adc_user_buffer[adc_out_index++] = adc_read_conversion_data(x);
		if(mio_error_code)
			return(1);
	}

	// A final dummy conversion is required to get the last data

	adc_start_conversion(15);
	if(mio_error_code)
		return(1);

	adc_wait_ready(15);
	if(mio_error_code)
		return(1);

	adc_user_buffer[adc_out_index++] = adc_read_conversion_data(15);
	if(mio_error_code)
		return(1);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		ADC_CONVERT_TO_VOLTS
//
//////////////////////////////////////////////////////////////////////////////

float adc_convert_to_volts(int channel, unsigned short value)
{
float result;

		if((channel < 0) || (channel > 15))
			return(0.0);

		value = value + adc_adjust[channel];
		result = value * adc_bitval[channel];
		result = result + adc_offset[channel];
		return result;
}


///////////////////////////////////////////////////////////////////////////////
//
//		ADC_CONVERT_SINGLE_REPEATED
//
//////////////////////////////////////////////////////////////////////////////

int adc_convert_single_repeated(int channel, unsigned short count, unsigned short *buffer)
{
	int x;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	// Setup global variables including transferring the address of the
	// user's output buffer to a global variable the ISR knows about.

	adc_user_buffer = buffer;
	adc_out_index = 0;
	adc_repeat_channel = channel;
	adc_repeat_count = count;

	adc_start_conversion(adc_repeat_channel);
	if(mio_error_code)
		return(1);

	adc_wait_ready(adc_repeat_channel);
	if(mio_error_code)
		return(1);

	// This data is old, we don't want it

	adc_read_conversion_data(adc_repeat_channel);
	if(mio_error_code)
		return(1);

	// Perform the requested number of conversions. Place the results into
	// the user's buffer.

	for(x=0; x<=adc_repeat_count; x++)
	{
		adc_start_conversion(adc_repeat_channel);
		if(mio_error_code)
			return(1);

		adc_wait_ready(adc_repeat_channel);
		if(mio_error_code)
			return(1);

		adc_user_buffer[adc_out_index++] = adc_read_conversion_data(adc_repeat_channel);
		if(mio_error_code)
			return(1);
	}

	// One last dummy conversion to retrieve our last data

	adc_start_conversion(adc_repeat_channel);
	if(mio_error_code)
		return(1);

	adc_wait_ready(adc_repeat_channel);
	if(mio_error_code)
		return(1);

	adc_user_buffer[adc_out_index++] = adc_read_conversion_data(adc_repeat_channel);
	if(mio_error_code)
		return(1);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		ADC_BUFFERED_CHANNEL_CONVERSIONS
//
//////////////////////////////////////////////////////////////////////////////

int adc_buffered_channel_conversions(unsigned char *input_channel_buffer,unsigned short *buffer)
{
int adc_next_channel;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;


	adc_ch_index = 0;
	adc_out_index = 0;
	
	adc_user_buffer = buffer;
	adc_input_buffer = input_channel_buffer;

	// Reset all of the array index pointers

	adc_start_conversion(adc_input_buffer[adc_ch_index]);

	if(mio_error_code)
		return(1);

	adc_wait_ready(adc_input_buffer[adc_ch_index++]);

	if(mio_error_code)
		return(1);

	// While there are channel numbers in the buffer (1= 0xff)
	// convert the requested channel and place the result in the
	// user's output buffer

	while(adc_input_buffer[adc_ch_index] != 0xff)
	{
		adc_next_channel = adc_input_buffer[adc_ch_index];


		/* This function is particularly tricky because of the
		fact that the data is delayed by one conversion and if
		we switch back and forth between the two controllers
		we'll need to run an extra conversion in order to get the
		last data offering from the previous controller. The
		conditional code in the next several lines handles the
		switches from one controller to the other.
		*/

		if(adc_current_channel < 8 && adc_next_channel > 7)
		{
			adc_start_conversion(adc_current_channel);
			if(mio_error_code)
				return(1);

			adc_wait_ready(adc_current_channel);
			if(mio_error_code)
				return(1);

			adc_user_buffer[adc_out_index++] = adc_read_conversion_data(adc_current_channel);
			if(mio_error_code)
				return(1);

			adc_start_conversion(adc_input_buffer[adc_ch_index]);
			if(mio_error_code)
				return(1);

			adc_wait_ready(adc_input_buffer[adc_ch_index++]);
			if(mio_error_code)
				return(1);
		}
		else if(adc_current_channel > 7 && adc_next_channel < 8)
		{
			adc_start_conversion(adc_current_channel);
			if(mio_error_code)
				return(1);

			adc_wait_ready(adc_current_channel);
			if(mio_error_code)
				return(1);

			adc_user_buffer[adc_out_index++] = adc_read_conversion_data(adc_current_channel);
			if(mio_error_code)
				return(1);

			adc_start_conversion(adc_input_buffer[adc_ch_index]);
			if(mio_error_code)
				return(1);

			adc_wait_ready(adc_input_buffer[adc_ch_index++]);
			if(mio_error_code)
				return(1);
		}
		adc_start_conversion(adc_input_buffer[adc_ch_index]);
		if(mio_error_code)
			return(1);

		adc_wait_ready(adc_input_buffer[adc_ch_index++]);
		if(mio_error_code)
			return(1);

		adc_user_buffer[adc_out_index++] = adc_read_conversion_data(adc_current_channel);
		if(mio_error_code)
			return(1);
	}

	// One last conversion allows us to retrieve our real last data

	adc_start_conversion(adc_input_buffer[--adc_ch_index]);
	if(mio_error_code)
		return(1);
	adc_wait_ready(adc_input_buffer[adc_ch_index]);
	if(mio_error_code)
		return(1);

	adc_user_buffer[adc_out_index++] = adc_read_conversion_data(adc_last_channel);
	if(mio_error_code)
		return(1);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//		ADC_WAIT_READY
//
//////////////////////////////////////////////////////////////////////////////

int adc_wait_ready(int channel)
{
long retry;
	
	mio_error_code = MIO_SUCCESS;
	retry = 100000l;

	/* Like with the DAC timeout routine, under normal circumstances we'll
	barely make it through the loop one time beacuse the hadrware is plenty
	fast. We have the delay for the rare occasion and when the hadrware is not
	responding properly.
	*/

	while(retry--)
	{
		if(adc_read_status(channel / 8) & 0x80)
			return 0;
	}

	mio_error_code = MIO_TIMEOUT_ERROR;
	sprintf(mio_error_string,"MIO(ADC) : Wait ready - Device timeout error");
	return(1);
}

///////////////////////////////////////////////////////////////////////////////
//
//		BUFFERED_DAC_OUTPUT
//
//////////////////////////////////////////////////////////////////////////////

int buffered_dac_output(unsigned char *cmd_buff,unsigned short *data_buff)
{
int x= 0;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	while(1)
	{
		if(cmd_buff[x] == 0xff)
			return 0;

		if(set_dac_output(cmd_buff[x], data_buff[x]))
			return 1;

		x++;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//		ADC_SET_CHANNEL_MODE
//
//////////////////////////////////////////////////////////////////////////////

int adc_set_channel_mode(int channel, int input_mode,int duplex,int range)
{
unsigned char command_byte;

	mio_error_code = MIO_SUCCESS;

	if(channel < 0 || channel > 15)
	{
		mio_error_code = MIO_BAD_CHANNEL_NUMBER;
		sprintf(mio_error_string,"MIO(ADC) : Set Channel Mode - Bad Channel Number %d",channel);
		return(1);
	}

	// Check for illegal modes

	if((input_mode != ADC_SINGLE_ENDED) && (input_mode != ADC_DIFFERENTIAL))
	{
		mio_error_code = MIO_BAD_MODE_NUMBER;
		sprintf(mio_error_string,"MIO(ADC) : Set Channel Mode - Bad Mode Number");
		return(1);
	}

	if((duplex != ADC_UNIPOLAR) && (duplex != ADC_BIPOLAR))
	{
		mio_error_code = MIO_BAD_MODE_NUMBER;
		sprintf(mio_error_string,"MIO(ADC) : Set Channel Mode - Bad Mode Number");
		return(1);
	}

	if((range != ADC_TOP_5V) && (range != ADC_TOP_10V))
	{
		mio_error_code = MIO_BAD_RANGE;
		sprintf(mio_error_string,"MIO(ADC) : Set Channel Mode - Bad Range Value");
		return(1);
	}

	command_byte = adc_channel_select[channel];
	command_byte = command_byte | input_mode | duplex | range;

	/* Building these four arrays at mode set time is critical for speed
	   as we don't need to calculate anything when we want to start an ADC
	   conversion. WE simply retrieve the command byte from the array
	   and send it to the controller.

	   Likewise, when doing conversion from raw 16-bit values to a voltage
	   the mode controls the worth of each individual bit as well as binary
	   bias and offset values.
   */

	adc_channel_mode[channel] = command_byte;

	/* Calculate bit values, offset, and adjustment values */

	if((range == ADC_TOP_5V) && (duplex == ADC_UNIPOLAR))
	{
		adc_bitval[channel] = 5.00 / 65536.0;
		adc_adjust[channel] = 0;
		adc_offset[channel] = 0.0;
	}

	if((range == ADC_TOP_5V) && (duplex == ADC_BIPOLAR))
	{
		adc_bitval[channel] = 10.0 / 65536.0;
		adc_adjust[channel] = 0x8000;
		adc_offset[channel] = -5.000;
	}

	if((range == ADC_TOP_10V) && (duplex == ADC_UNIPOLAR))
	{
		adc_bitval[channel] = 10.0 / 65536.0;
		adc_adjust[channel] = 0;
		adc_offset[channel] = 0.0;
	}

	if((range == ADC_TOP_10V) && (duplex == ADC_BIPOLAR))
	{
		adc_bitval[channel] = 20.0 / 65536.0;
		adc_adjust[channel] = 0x8000;
		adc_offset[channel] = -10.0;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//
//		ADC_AUTO_GET_CHANNEL_VOLTAGE
//
//
/////////////////////////////////////////////////////////////////////////////////

float adc_auto_get_channel_voltage(int channel)
{
unsigned short value;
float result;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	// Start out on a +/-10 Volt scale

	adc_set_channel_mode(channel,ADC_SINGLE_ENDED,ADC_BIPOLAR,ADC_TOP_10V);
	if(mio_error_code)
		return(0.0);

	adc_start_conversion(channel);
	if(mio_error_code)
		return(0.0);

	adc_wait_ready(channel);
	if(mio_error_code)
		return(0.0);

	adc_start_conversion(channel);
	if(mio_error_code)
		return(0.0);

	adc_wait_ready(channel);
	if(mio_error_code)
		return(0.0);

	value = adc_read_conversion_data(channel);
	if(mio_error_code)
		return(0.0);

	// Convert the raw data to voltage

	value = value + adc_adjust[channel];
	result = value * adc_bitval[channel];
	result = result + adc_offset[channel];

#ifdef DEBUG
	printf("auto_get_channel_voltage : Raw = %04x, adjust = %d, bitval = %9.5f, offset = %9.5f,  result = %9.5f\n",
		                                value - adc_adjust[channel],adc_adjust[channel],adc_bitval[channel],adc_offset[channel],result);
#endif

	// If the voltage is less than -5.00 volts, we're as precise as we can get

	if(result <= -5.00)
		return(result);

	// If the result is between -4.99 and 0.0 we can  to the +/- 5V scale.

	if(result < 0.0)
		adc_set_channel_mode(channel,ADC_SINGLE_ENDED,ADC_BIPOLAR,ADC_TOP_5V);

	if(mio_error_code)
		return(0.0);

	// If the result is above 5 volts a 0 - 10V range will work best

	if(result >= 5.00)
		adc_set_channel_mode(channel,ADC_SINGLE_ENDED,ADC_UNIPOLAR,ADC_TOP_10V);

	if(mio_error_code)
		return(0.0);

	// Lastly if we're greater than 0 and less than 5 volts the 0-5V scale is best

	if((result >= 0.0) && (result < 5.00))
		adc_set_channel_mode(channel, ADC_SINGLE_ENDED, ADC_UNIPOLAR,ADC_TOP_5V);

	if(mio_error_code)
		return(0.0);

	// Now that the values is properly ranged, we take two more samples
	// to get a current reading at the new scale.

	adc_start_conversion(channel);

	if(mio_error_code)
		return(0.0);

	adc_wait_ready(channel);

	if(mio_error_code)
		return(0.0);

	adc_start_conversion(channel);

	if(mio_error_code)
		return(0.0);

	adc_wait_ready(channel);

	if(mio_error_code)
		return(0.0);

	value = adc_read_conversion_data(channel);
	if(mio_error_code)
		return(0.0);

	// Convert the raw data to voltage

	value = value + adc_adjust[channel];
	result = value * adc_bitval[channel];
	result = result + adc_offset[channel];

#ifdef DEBUG
	printf("auto_get_channel_voltage : Raw = %04x, adjust = %d, bitval = %9.5f, offset = %9.5f,  result = %9.5f\n",
		                                value - adc_adjust[channel],adc_adjust[channel],adc_bitval[channel],adc_offset[channel],result);
#endif

	return(result);
}

////////////////////////////////////////////////////////////////////////////////
//
//
//		DIO_READ_BIT
//
//
/////////////////////////////////////////////////////////////////////////////////

int dio_read_bit(int bit_number)
{
unsigned char port;
int val;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	// Adjust for 0 - 47 bit numbering

	--bit_number;

	port = bit_number / 8;

	val = read_dio_byte(port);

	// Get just the bit we specified

	val = val & (1 << (bit_number % 8));

	// adjust the return for a 0 or 1 value

	if(val)
		return 1;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//
//		DIO_WRITE_BIT
//
//
/////////////////////////////////////////////////////////////////////////////////

int dio_write_bit(int bit_number, int val)
{
unsigned char port;
unsigned char temp;
unsigned char mask;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	// Adjust bit numbering for 0 based numbering

	--bit_number;

	// Calculate the address of the port based on bit number

	port = bit_number / 8;

	// Use the image value to avoid having to read from the port first

	temp = dio_port_images[bit_number / 8];

	// Calculate the bit mask for the specifed bit

	mask = (1 << (bit_number %8));

	// Check whether the request was to set or clear the bit

	if(val)
		temp = temp | mask;
	else
		temp = temp & ~mask;

	// Update the image value with the value we're about to write

	dio_port_images[bit_number / 8] = temp;

	write_dio_byte(port, temp);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
//
//		DIO_SET_BIT
//
//
/////////////////////////////////////////////////////////////////////////////////

int dio_set_bit(int bit_number)
{
	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	dio_write_bit(bit_number,1);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//
//		DIO_CLR_BIT
//
//
/////////////////////////////////////////////////////////////////////////////////

int dio_clr_bit(int bit_number)
{
	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	dio_write_bit(bit_number,0);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//
//		DIO_ENAB_BIT_INT
//
//
/////////////////////////////////////////////////////////////////////////////////

int dio_enab_bit_int(int bit_number, int polarity)
{
unsigned char port;
unsigned char temp;
unsigned char mask;


	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	// Adjust the bit number for 0 based numbering

	--bit_number;

	// Calculate the offset for the enable port

	port = (bit_number / 8) + 8;

	// Calculate the proper bit mask for this bit number

	mask = (1 << (bit_number % 8));

	// Turn on access to page 2 registers

	write_dio_byte(0x07,0x80);

	// Get the current state of the enable register

	temp = read_dio_byte(port);

        // Temporarily clear only our enable. This clears the interrupt

        temp = temp & ~mask;

        // Write out the temporary value

        write_dio_byte(port, temp);

	// Set the enable bit for our bit number

	temp = temp | mask;

	// Now update the interrupt enable register

	write_dio_byte(port, temp);

	// Turn on access to page 1 for polarity control

	write_dio_byte(0x07,0x40);

	temp = read_dio_byte(port);

	// Set the polarity according to the argument value

	if(polarity)
		temp = temp | mask;
	else
		temp = temp & ~mask;

	write_dio_byte(port, temp);

	// Set access back to page 0

	write_dio_byte(0x07,0);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
//
//		DIO_DISAB_BIT_INT
//
//
/////////////////////////////////////////////////////////////////////////////////

int dio_disab_bit_int(int bit_number)
{
unsigned char port;
unsigned char temp;
unsigned char mask;

	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	// Adjust the bit number for 0 based numbering

	--bit_number;

	// Calculate the offset for the enable port

	port = (bit_number / 8) + 8;

	// Calculate the proper bit mask for this bit number

	mask = (1 << (bit_number % 8));

	// Turn on access to page 2 registers

	write_dio_byte(0x07,0x80);

	// Get the current state of the enable register

	temp = read_dio_byte(port);

	// Clear the enable bit for the our bit

	temp = temp & ~mask;

	// Update the enable register with the new data

	write_dio_byte(port,temp);

	// Set access back to page 0

	write_dio_byte(0x07,0);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
//
//		DIO_CLR_INT
//
//
/////////////////////////////////////////////////////////////////////////////////

int dio_clr_int(int bit_number)
{
unsigned short port;
unsigned short temp;
unsigned short mask;

	// Adjust for 0 based numbering
	mio_error_code = MIO_SUCCESS;

	if(check_handle())   /* Check for chip available */
		return -1;

	--bit_number;

	// Calculate the correct offset for our enable register

	port = (bit_number / 8) + 8;

	// Calculate the bit mask for this bit

	mask = (1 << (bit_number % 8));

	// Set access to page 2 for the enable register

	write_dio_byte(0x07,0x80);

	// Get the current state of the register

	temp = read_dio_byte(port);

	// Temporarily clear only our enable. This clears the interrupt

	temp = temp & ~mask;

	// Write out the temporary value

	write_dio_byte(port, temp);

	temp = temp | mask;

	write_dio_byte(port, temp);

	// Set access back to page 0

	write_dio_byte(0x07,0);

	return 0;
}

