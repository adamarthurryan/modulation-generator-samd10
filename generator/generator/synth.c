#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <arm_math.h>

#include "q16d15.h"
#include "modulation.h"

#include "synth.h"

synth_parameters_t synth_parameters = {{{Q16D15_1, SAW, Q16D15_1_HALF}, {Q16D15_1, SAW, Q16D15_1_HALF}}, {{Q16D15_1,Q16D15_1,Q16D15_1, Q16D15_1}, {Q16D15_1,Q16D15_1,Q16D15_1, Q16D15_1}}, {{{Q16D15_1_HALF, Q16D15_1_HALF}, {0, 0}}, {{Q16D15_1_HALF, Q16D15_1_HALF}, {0, 0}}}};

phasor_state_t lfoPhasorState[NUM_LFOS];
adsr_state_t envState[NUM_ENVS];

bool envTriggers[NUM_ENVS];

void synth_env_trigger(int instance, bool value) {
	if (instance >= 0 && instance < NUM_ENVS) {
		envTriggers[instance] = value;
	}
}

void dsp_configure() {
	for (int i=0;i<NUM_LFOS;i++) {
		lfoPhasorState[i] = initialize_phasor_state();
	}
	for (int i=0;i<NUM_ENVS;i++) {
		envState[i] = initialize_adsr_state();
	}
}


void dsp_run_block (q15_t * block) {
	static q15_t lfoBuffer[NUM_LFOS][SAMPLE_BUFFER_SIZE];
	static q15_t envBuffer[NUM_ENVS][SAMPLE_BUFFER_SIZE];
	
	//render LFOs
	for (int i=0;i<NUM_LFOS;i++) {
		static phasor_model_t lfoPhasorModel;
		lfoPhasorModel = create_phasor_model(synth_parameters.lfo[i].frequency, SAMPLE_PERIOD_Q31);
		
		phasor_q15(&lfoPhasorModel, &lfoPhasorState[i], lfoBuffer[i], SAMPLE_BUFFER_SIZE);
		switch (synth_parameters.lfo[i].shape) {
			case SAW:
				saw_q15(lfoBuffer[i], lfoBuffer[i], SAMPLE_BUFFER_SIZE);
				break;
			case SINE:
				sine_q15(lfoBuffer[i], lfoBuffer[i], SAMPLE_BUFFER_SIZE);
				break;
			case SQUARE:
				square_q15(lfoBuffer[i], synth_parameters.lfo[i].duty, lfoBuffer[i], SAMPLE_BUFFER_SIZE);
				break;
			case TRI:
				tri_q15(lfoBuffer[i], lfoBuffer[i], SAMPLE_BUFFER_SIZE);
				break;
		}
	}
	
	//render envs
	for (int i=0;i<NUM_ENVS;i++) {
		static adsr_model_t envModel;
		envModel = create_adsr_model(synth_parameters.env[i].attack,\
					synth_parameters.env[i].decay,\
					synth_parameters.env[i].release,\
					synth_parameters.env[i].sustain,\
					SAMPLE_PERIOD_Q31);
		adsr_q15(&envModel, envTriggers[i], &envState[i], envBuffer[i], SAMPLE_BUFFER_SIZE);
	}
	
	//render mix outs
	static q15_t mixOut[NUM_MIXERS][SAMPLE_BUFFER_SIZE];
	for (int i=0;i<NUM_MIXERS;i++) {
		mix2_q15(lfoBuffer[0], synth_parameters.mix[i].lfo[0], lfoBuffer[1], synth_parameters.mix[i].lfo[1], mixOut[i], SAMPLE_BUFFER_SIZE);
		mix2_q15(envBuffer[0], synth_parameters.mix[i].env[0], mixOut[i], Q15_1, mixOut[i], SAMPLE_BUFFER_SIZE);
		mix2_q15(envBuffer[1], synth_parameters.mix[i].env[1], mixOut[i], Q15_1, mixOut[i], SAMPLE_BUFFER_SIZE);
	}
	
	arm_copy_q15(mixOut[0], block, SAMPLE_BUFFER_SIZE);
	//arm_copy_q15(envBuffer[0], block, SAMPLE_BUFFER_SIZE);
}
