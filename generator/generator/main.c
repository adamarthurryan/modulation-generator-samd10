#include "atmel_start.h"
#include "atmel_start_pins.h"

#include "modulation.h"
#include "osc_parser.h"

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
	phasor_model_t phasor_model_a = create_phasor_model(10, SAMPLE_RATE);
	phasor_q15(&phasor_model_a, &phasor_state_a, scratch1, SAMPLE_BUFFER_SIZE);
	sine_q15( scratch1, scratch1, SAMPLE_BUFFER_SIZE);

	phasor_model_t phasor_model_b = create_phasor_model(33, SAMPLE_RATE);
	phasor_q15(&phasor_model_b, &phasor_state_b, scratch2, SAMPLE_BUFFER_SIZE);
	sine_q15(scratch2, scratch2, SAMPLE_BUFFER_SIZE);
					
	mix2_q15(scratch1, scratch2, samples[i], SAMPLE_BUFFER_SIZE);
	endTicks = systick_read();

	//printf("phasor_model.phase_step=%d calculated in %u systicks\n", phasor_model.phaseStep, startTicks-endTicks);
	//printf("samples [0:3]: %d %d %d %d\n", samples[i][0], samples[i][1], samples[i][2], samples[i][3]);

}


typedef struct {
	float frequency;
	int shape;
	float duty;
} lfo_parameters_t;

typedef struct {
	int attack;
	int decay;
	int release;
	float sustain;
} env_parameters_t;

typedef struct {
	float lfo1;
	float lfo2;
	float env1;	
	float env2;
} mix_parameters_t;

struct {
	lfo_parameters_t lfo1;
	lfo_parameters_t lfo2;
	env_parameters_t env1;
	env_parameters_t env2;
	mix_parameters_t mix1;
	mix_parameters_t mix2;
} parameters;


// ======

void simple_ftos(char * buffer, int size, float value) {
	//do simple_itos on the integer part
	//then multiply the fractional part by, say 1000000 and do it again
	
}

void simple_itos(char * buffer, int size, int value) {
	if (value<0) {
		value = -value;
		*buffer = '-';
		buffer++;
		size--;
	}
	while (value > 0 && size) {
		...
	}
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
		osc_message_t oscCommand;
		
		char line[MAX_CMD_LINE_LENGTH];
		int lineLength = serial_read_line(line, MAX_CMD_LINE_LENGTH, true);

		osc_parser_result_t parserResult = osc_parse(line, lineLength, &oscCommand);
		
		if (parserResult.hasError){
			serial_write_const("Parse error: ");
			serial_write(parserResult.errorMessage, parserResult.errorMessageLength);
			serial_write("\n",1);
		}
		else {
			serial_write_const("Command received: ");
			for (int i=0;i<oscCommand.numAddrParts;i++) {
				serial_write_const("/");
				serial_write(oscCommand.addrParts[i], oscCommand.addrPartLengths[i]);
			}
			/*
			if (oscCommand.hasTypeTag) {
				serial_write_const(",");
				serial_write(oscCommand.typeTag, oscCommand.typeTagLength);
			}
			*/
			
			for (int i=0;i<oscCommand.numArguments;i++) {
				serial_write_const(" ");
				if (oscCommand.argumentTypes[i] == FLOAT) {
					serial_write_const("f:");

					char buf[24];
					simple_ftos(buf, 13, oscCommand.argumentValues[i].floatArg);
					serial_write(buf, strlen(buf));
				}
				else if (oscCommand.argumentTypes[i] == INT) {
					serial_write_const("i:");

					char buf[13];
					simple_itos(buf, 13, oscCommand.argumentValues[i].intArg);
					serial_write(buf, strlen(buf));
				}
				else  {
					serial_write_const("s:");
					serial_write(oscCommand.arguments[i], oscCommand.argumentLengths[i]);					
				}
			}
			
			serial_write_const("\n");
		}
	}
}
