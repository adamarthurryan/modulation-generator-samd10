#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <arm_math.h>

#include "atmel_start.h"
#include "atmel_start_pins.h"

#include "q16d15.h"
#include "synth.h"
#include "simple_strnum.h"
#include "modulation.h"
#include "osc_parser.h"
#include "osc_interpreter.h"
#include "serial_io.h"
#include "systick.h"


#include "synth.h"

#define MAX_CMD_LINE_LENGTH 96



q15_t samples[2][NUM_MIXERS][SAMPLE_BUFFER_SIZE];
int sample_buffer = 0;
int sample_index = 0;

static struct timer_task sampleTimer_task1;
static struct timer_task sampleTimer_dsp_task1;



/**Write an unsigned signal to the dac.
	* Negative values will not render correctly.*/
void dac_write_unsigned(q15_t value) {
	uint16_t value_q9 = value >> 5;
	dac_sync_write(&DAC_0, 0, &value_q9, 1);
}

/** Configure the DAC for output.*/
void dac_configure() {
	dac_sync_enable_channel(&DAC_0, 0);
}


/** Sample timer. Outputs the current sample to the DAC. */
static void sampleTimer_cb(const struct timer_task *const timer_task) {
	//output current sample
	//the dac gets mixer 0 output
	dac_write_unsigned(samples[sample_buffer][0][sample_index]);
	
	//the pwm gets mixer 1 output
	pwm_set_parameters(&PWM_1, 1<<12, samples[sample_buffer][1][sample_index]>>3);
	
	//increment sample number
	sample_index++;
	if (sample_index>=SAMPLE_BUFFER_SIZE) {
		//increment the sample buffer
		sample_buffer = 1-sample_buffer;
		sample_index=0;		
	}
}

/** Run the DSP block on the off sample.*/
static void sampleTimer_dsp_cb(const struct timer_task *const timer_task) {
	dsp_run_block((q15_t *) samples[1-sample_buffer]);

}

/** Configure the sample timer.*/
void sampleTimer_configure(void)
{

	sampleTimer_dsp_task1.interval = SAMPLE_BUFFER_SIZE;
	sampleTimer_dsp_task1.cb = sampleTimer_dsp_cb;
	sampleTimer_dsp_task1.mode = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_0, &sampleTimer_dsp_task1);

	sampleTimer_task1.interval = 1;
	sampleTimer_task1.cb = sampleTimer_cb;
	sampleTimer_task1.mode = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_0, &sampleTimer_task1);


	timer_start(&TIMER_0);
}

/** Configure PWM 1.*/
void pwm_configure(void) {
	pwm_enable(&PWM_1);
}

// ================


/** Button handler.*/
static void trigger_button_pressed(void)
{
	int triggerLevel = !gpio_get_pin_level(PA25);
	synth_env_trigger(0,triggerLevel);
	synth_env_trigger(1,triggerLevel);
}


/**
 * Example of using EXTERNAL_IRQ_0
 */
void trigger_button_configure(void)
{
	ext_irq_register(PIN_PA25, trigger_button_pressed);
}





// ======

// Refactor to osc_tostring and move to osc_parser
void osc_message_write(osc_message_t * oscMessage) {
	for (int i=0;i<oscMessage->numAddrParts;i++) {
		serial_write_const("/");
		serial_write(oscMessage->addrParts[i], oscMessage->addrPartLengths[i]);
	}
	/*
	if (oscCommand.hasTypeTag) {
		serial_write_const(",");
		serial_write(oscCommand.typeTag, oscCommand.typeTagLength);
	}
	*/

	for (int i=0;i<oscMessage->numArguments;i++) {
		serial_write_const(" ");
		if (oscMessage->argumentTypes[i] == Q) {
			serial_write_const("f:");

			static char buf[24];
			q16d15_t value = oscMessage->argumentValues[i].qArg;
			simple_qtos(buf, 13, value, 3);
			serial_write(buf, strlen(buf));
		}
		else if (oscMessage->argumentTypes[i] == INT) {
			serial_write_const("i:");

			static char buf[13];
			simple_itos(buf, 13, oscMessage->argumentValues[i].intArg);
			serial_write(buf, strlen(buf));
		}
		else  {
			serial_write_const("s:");
			serial_write_const(oscMessage->argumentValues[i].stringArg);					
		}
	}
	serial_write_const("\n");
}



int main(void)
 {
	system_init();
	systick_init();
	
	dac_configure();
	sampleTimer_configure();
	serial_configure();
	dsp_configure();
	
	osc_interpreter_configure();
	trigger_button_configure();
	
	//test pwm
	pwm_configure();
	
	
	serial_write_const("\nModulation Generator\n");
	

	//test flash size
	char buffer[10];
	uint32_t page_size = flash_get_page_size(&FLASH_0);
	size_t params_size = sizeof(synth_parameters);
	uint32_t num_pages = flash_get_total_pages(&FLASH_0);
	
	//this is set with NVRAM_EEPROM_SIZE = 0x5 (2 rows = 512 bytes)
	uint32_t num_flash_pages = 2;
	//the address of the first flash page?
	uint32_t ptr_flash_start = page_size*(num_pages-num_flash_pages);
	
	serial_write_const("Flash page size: ");
	simple_itos(buffer, 10, page_size);
	serial_write_const((const char *)buffer);
	serial_write_const("\n");
	
	serial_write_const("Parameters size: ");
	simple_itos(buffer, 10, params_size);
	serial_write_const((const char *)buffer);
	serial_write_const("\n");



	//pwm_set_parameters(&PWM_0,10000,5000);
	//pwm_enable(&PWM_0);

	while(1) {
		static osc_message_t oscMessage;
		
		static char line[MAX_CMD_LINE_LENGTH];
		int lineLength = serial_read_line(line, MAX_CMD_LINE_LENGTH, true);

		osc_result_t parserResult = osc_parse(line, lineLength, &oscMessage);
		
		if (parserResult.hasError){
			serial_write_const("Parse error: ");
			serial_write_const(parserResult.errorMessage);
			serial_write_const("\n");
		}
		else {
			serial_write_const("Command received: ");
			osc_message_write(&oscMessage);			
			osc_result_t interpretResult = osc_message_interpret(&oscMessage);
			
			if (interpretResult.hasError) {
				serial_write_const("Interpret error: ");
				serial_write_const(interpretResult.errorMessage);
				serial_write_const("\n");
			}
		}
	}
}
