#include "atmel_start.h"
#include "atmel_start_pins.h"
#include "stdio_start.h"

#include "modulation.h"

#include "systick.h"

#define SAMPLE_RATE 11025 
#define SAMPLE_BUFFER_SIZE 32


volatile q15_t samples[2][SAMPLE_BUFFER_SIZE];
volatile int sample_buffer = 0;
volatile int sample_index = 0;
volatile bool sample_buffer_request [2] = {true,true};

static struct timer_task sampleTimer_task1;

/**Write a signal to the dac.*/
void dac_write(q15_t value) {
	uint16_t value_q9_t = value >> 6;
	dac_sync_write(&DAC_0, 0, &value_q9_t, 1);
}

/** Configure the DAC for output.*/
void dac_configure() {
	dac_sync_enable_channel(&DAC_0, 0);
}

/** Sample timer. Outputs the current sample to the DAC. */
static void sampleTimer_cb(const struct timer_task *const timer_task) {
	//output current sample
	dac_write(samples[sample_buffer][sample_index]);
	
	//increment sample number
	sample_index++;
	if (sample_index>=SAMPLE_BUFFER_SIZE) {
		//increment the sample buffer and request that the old buffer be calculated
		sample_buffer_request[sample_buffer] = true;
		sample_buffer = 1-sample_buffer;
		sample_index=0;		
	}
}


/** Configure the sample timer.*/
void sampleTimer_configure(void)
{
	sampleTimer_task1.interval = 1;
	sampleTimer_task1.cb = sampleTimer_cb;
	sampleTimer_task1.mode = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_0, &sampleTimer_task1);
	timer_start(&TIMER_0);
}

int main(void)
 {
	system_init();
	systick_init();
	
	dac_configure();
	sampleTimer_configure();

	STDIO_REDIRECT_0_init();
	
	/*Enable system interrupt*/
	
	USART_0_example();
	STDIO_REDIRECT_0_example();

	phasor_state_t phasor_state = initialize_phasor_state();

	//pwm_set_parameters(&PWM_0,10000,5000);
	//pwm_enable(&PWM_0);

	uint32_t startTicks, endTicks;
	while(1) {
		for (int i=0;i<=1;i++) {
			if (sample_buffer_request[i] == true) {	
				startTicks = systick_read();
				phasor_model_t phasor_model = create_phasor_model(10, SAMPLE_RATE);
				endTicks = systick_read();
				phasor_q15(&phasor_model, &phasor_state, samples[i], SAMPLE_BUFFER_SIZE);
				sample_buffer_request[i] = false;

				//printf("phasor_model.phase_step=%d calculated in %u systicks\n", phasor_model.phaseStep, startTicks-endTicks);
				//printf("samples [0:3]: %d %d %d %d\n", samples[i][0], samples[i][1], samples[i][2], samples[i][3]);

				
			}
		}
		

	}
}
