#include "atmel_start.h"
#include "atmel_start_pins.h"
#include "stdio_start.h"

#include "modulation.h"
#include "osc_parser.h"

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

struct io_descriptor *io;
/** Writes the given number of characters from the buffer.*/
void usart_write(char * string, uint16_t length) {
	io_write(io, (uint8_t *)string, length);
}

/** Writes a null-terminated string (eg. a string literal) */
void usart_write_0(char * string) {
	usart_write(string, strlen(string));
}
/** Reads a line of text from the io stream, up to the given length.
	
	If echo is true, the characters will be echoed back as they are received.

	Returns the number of characters in the line, not including the newline character.
	Returns -1 if the buffer overflows
*/	
int usart_read_line(char * buffer, int length, int echo) {
	int count = 0;
	while (true) {
		int read = io_read(io, buffer, 1);
		if (read==1 && (*buffer == '\n' || *buffer == '\r' ) ) {
			if (echo)
				io_write(io,"\n", 1);
			return count;
		}
		
		if (echo)
			io_write(io,buffer, 1);
			
		count++;
		
		if (count>=length)
			return -1;
			
		buffer++;
	}
}
int usart_read_char(char * buffer) {
	return io_read(io, buffer, 1);
}


void usart_configure() {
	usart_sync_get_io_descriptor(&USART_0, &io);
	usart_sync_enable(&USART_0);
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
	sine_q15(scratch2, scratch3, SAMPLE_BUFFER_SIZE);
	sine_q15(scratch2, scratch3, SAMPLE_BUFFER_SIZE);
	sine_q15(scratch2, scratch3, SAMPLE_BUFFER_SIZE);
	sine_q15(scratch2, scratch3, SAMPLE_BUFFER_SIZE);
					
	mix2_q15(scratch1, scratch1, samples[i], SAMPLE_BUFFER_SIZE);
	endTicks = systick_read();

	//printf("phasor_model.phase_step=%d calculated in %u systicks\n", phasor_model.phaseStep, startTicks-endTicks);
	//printf("samples [0:3]: %d %d %d %d\n", samples[i][0], samples[i][1], samples[i][2], samples[i][3]);

}


//parameters table
/**/
/*
char * parameter_names[] = {
"/lfo/1/frequency",
"/lfo/1/shape",
"/lfo/1/duty",
"/lfo/2/frequency",
"/lfo/2/shape",
"/lfo/2/duty",
"/env/1/attack",
"/env/1/decay",
"/env/1/release",
"/env/1/sustain",
"/env/2/attack",
"/env/2/decay",
"/env/2/release",
"/env/2/sustain",
"/mix/1/lfo/1",
"/mix/1/lfo/2",
"/mix/1/env/1",
"/mix/1/env/2",
"/mix/2/lfo/1",
"/mix/2/lfo/2",
"/mix/2/env/1"
"/mix/2/env/2"
};
*/
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

/*
void * [] parameter_ptrs {
	&(lfo1.frequency),
	&(lfo1.shape),
	&(lfo1.duty),
	&(lfo2.frequency),
	&(lfo2.shape),
	&(lfo2.duty),
	&(env1.attack),
	&(env1.decay),
	&(env1.release),
	&(env1.sustain),
	&(env2.attack),
	&(env2.decay),
	&(env2.release),
	&(env2.sustain),
	&(mix1.lfo1),
	&(mix1.lfo2),
	&(mix1.env1),
	&(mix1.env2),
	&(mix2.lfo1),
	&(mix2.lfo2),
	&(mix2.env1),
	&(mix2.env2),
};*/

// ======



int main(void)
 {
	system_init();
	systick_init();
	
	dac_configure();
	sampleTimer_configure();
	usart_configure();
	dsp_configure();
	
	//STDIO_REDIRECT_0_init();
	
	//STDIO_REDIRECT_0_example();
	
	usart_write("Modulation Generator\n", 21);
	


	//pwm_set_parameters(&PWM_0,10000,5000);
	//pwm_enable(&PWM_0);

	while(1) {
		osc_message_t oscCommand;
		
		char line[MAX_CMD_LINE_LENGTH];
		int lineLength = usart_read_line(line, MAX_CMD_LINE_LENGTH, true);

		osc_parser_result_t parserResult = osc_parse(line, lineLength, &oscCommand);
		
		if (parserResult.hasError){
			usart_write_0("Parse error: ");
			usart_write(parserResult.errorMessage, parserResult.errorMessageLength);
			usart_write("\n",1);
		}
		else {
			usart_write_0("Command received: ");
			for (int i=0;i<oscCommand.numAddrParts;i++) {
				usart_write_0("/");
				usart_write(oscCommand.addrParts[i], oscCommand.addrPartLengths[i]);
			}
			
			if (oscCommand.hasTypeTag) {
				usart_write_0(",");
				usart_write(oscCommand.typeTag, oscCommand.typeTagLength);
			}
			
			for (int i=0;i<oscCommand.numArguments;i++) {
				usart_write_0(" ");
				usart_write(oscCommand.arguments[i], oscCommand.argumentLengths[i]);
			}
			
			usart_write_0("\n");
		}
	}
}
