#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <arm_math.h>

#include "atmel_start.h"
#include "atmel_start_pins.h"

#include "q16d15.h"
#include "synth_parameters.h"
#include "simple_strnum.h"
#include "modulation.h"
#include "osc_parser.h"
#include "osc_interpreter.h"
#include "serial_io.h"
#include "systick.h"

#define MAX_CMD_LINE_LENGTH 96

#define SAMPLE_RATE 11025 
#define SAMPLE_BUFFER_SIZE 32


volatile q15_t samples[2][SAMPLE_BUFFER_SIZE];
volatile int sample_buffer = 0;
volatile int sample_index = 0;

static struct timer_task sampleTimer_task1;
static struct timer_task sampleTimer_dsp_task1;

volatile synth_parameters_t synth_parameters;

void dsp_run_block (int i);

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
	dac_write_unsigned(samples[sample_buffer][sample_index]);
	
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
	dsp_run_block(1-sample_buffer);
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



// ================

phasor_state_t phasor_state_a;
phasor_state_t phasor_state_b;

q15_t scratch1[SAMPLE_BUFFER_SIZE];
q15_t scratch2[SAMPLE_BUFFER_SIZE];
q15_t scratch3[SAMPLE_BUFFER_SIZE];

uint32_t startTicks, endTicks;

void dsp_configure() {
	phasor_state_a = initialize_phasor_state();
	phasor_state_b = initialize_phasor_state();
}


void dsp_run_block (int i) {
	startTicks = systick_read();
	phasor_model_t phasor_model_a = create_phasor_model(synth_parameters.lfo[0].frequency, SAMPLE_RATE);
	phasor_q15(&phasor_model_a, &phasor_state_a, scratch1, SAMPLE_BUFFER_SIZE);
	sine_q15( scratch1, scratch1, SAMPLE_BUFFER_SIZE);

	phasor_model_t phasor_model_b = create_phasor_model(synth_parameters.lfo[1].frequency, SAMPLE_RATE);
	phasor_q15(&phasor_model_b, &phasor_state_b, scratch2, SAMPLE_BUFFER_SIZE);
	sine_q15(scratch2, scratch2, SAMPLE_BUFFER_SIZE);
					
	mix2_q15(scratch1, scratch2, samples[i], SAMPLE_BUFFER_SIZE);
	endTicks = systick_read();

	//printf("phasor_model.phase_step=%d calculated in %u systicks\n", phasor_model.phaseStep, startTicks-endTicks);
	//printf("samples [0:3]: %d %d %d %d\n", samples[i][0], samples[i][1], samples[i][2], samples[i][3]);

}




// ======


void osc_message_write(osc_message_t oscMessage) {
	for (int i=0;i<oscMessage.numAddrParts;i++) {
		serial_write_const("/");
		serial_write(oscMessage.addrParts[i], oscMessage.addrPartLengths[i]);
	}
	/*
	if (oscCommand.hasTypeTag) {
		serial_write_const(",");
		serial_write(oscCommand.typeTag, oscCommand.typeTagLength);
	}
	*/

	for (int i=0;i<oscMessage.numArguments;i++) {
		serial_write_const(" ");
		if (oscMessage.argumentTypes[i] == Q) {
			serial_write_const("f:");

			char buf[24];
			q16d15_t value = oscMessage.argumentValues[i].qArg;
			simple_qtos(buf, 13, value, 3);
			serial_write(buf, strlen(buf));
		}
		else if (oscMessage.argumentTypes[i] == INT) {
			serial_write_const("i:");

			char buf[13];
			simple_itos(buf, 13, oscMessage.argumentValues[i].intArg);
			serial_write(buf, strlen(buf));
		}
		else  {
			serial_write_const("s:");
			serial_write(oscMessage.argumentValues[i].stringArg, strlen(oscMessage.argumentValues[i].stringArg));					
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
	
	//STDIO_REDIRECT_0_init();
	
	//STDIO_REDIRECT_0_example();
	
	serial_write("Modulation Generator\n", 21);
	


	//pwm_set_parameters(&PWM_0,10000,5000);
	//pwm_enable(&PWM_0);

	while(1) {
		osc_message_t oscMessage;
		
		char line[MAX_CMD_LINE_LENGTH];
		int lineLength = serial_read_line(line, MAX_CMD_LINE_LENGTH, true);

		osc_parser_result_t parserResult = osc_parse(line, lineLength, &oscMessage);
		
		if (parserResult.hasError){
			serial_write_const("Parse error: ");
			serial_write(parserResult.errorMessage, parserResult.errorMessageLength);
			serial_write("\n",1);
		}
		else {
			serial_write_const("Command received: ");
			osc_message_write(oscMessage);			
			osc_message_interpret(oscMessage);

		}
	}
}
