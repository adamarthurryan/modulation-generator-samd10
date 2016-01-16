#include "modulation.h"

#include <stdint.h>
#include <arm_math.h>

#define Q31_TO_Q15(a) a>>16
#define Q31_1 (1<<31) - 1
#define Q15_1 (1<<15) - 1
#define F32_TO_Q31(f32) (q31_t) ((f32-(int)f32)*2147483648.0f) //?
//#define CLIPQ63_TO_Q31(x) ((q31_t) ((x >> 32) != ((q31_t) x >> 31)) ?	((0x7FFFFFFF ^ ((q31_t) (x >> 63)))) : (q31_t) x)


phasor_model_t create_phasor_model(float frequency, uint16_t sampleRate) {
	float step = frequency/sampleRate;
	phasor_model_t model;
	
//wow, this brings in a lot of extra code! like 3 or 4 KB!
//	model.phaseStep=clip_q63_to_q31((q63_t) (period * 2147483648.0f));

	model.phaseStep = F32_TO_Q31(step);
	return model;
}
adsr_model_t create_adsr_model(uint16_t attackMs, uint16_t decayMs, uint16_t releaseMs, float sustain, uint16_t sampleRate) {
	adsr_model_t model;
	return model;
}

phasor_state_t initialize_phasor_state() {
	phasor_state_t state;
	state.level = 0;
	return state;
}

adsr_state_t initialize_adsr_state() {
	adsr_state_t state;
	state.level = 0;
	state.mode = STATE_R;
	return state;
}

void phasor_q15(phasor_model_t * model, phasor_state_t * state, q15_t *phaseOut, uint32_t blockSize) {
	uint8_t i;
	
	//the accumulator starts at the previous level
	q31_t acc=state->level;
	
	for (i=0;i<blockSize;i++) {
		//increment by the phase step
		acc += model->phaseStep;
		
		//overflow the phase
		//the accumulator overflows to a negative value, which should be truncated to zero
		if (acc<0)
			acc = 0;
		  
		// maybe this should be done for the whole block at once?
		*phaseOut = Q31_TO_Q15(acc);
		
		//increment the phase pointer
		phaseOut++;
	}
	
	state->level = acc;
}

void saw_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize) {
	arm_copy_q15(phaseIn, waveOut, blockSize);
}

//void sine_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize);
//void tri_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize);

void square_q15(q15_t duty, q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize) {
	uint8_t i;
	for (i=0;i<blockSize;i++) {
		//the output signal is 0 if greater than the duty threshold
		//eg. a duty of 0.75 is of whenever the phase is over 0.75
		*waveOut = (*phaseIn<duty) ? Q15_1 : 0;
		waveOut++;
		phaseIn++;
	}	
}

//void adsr_q15(adsr_model_t * model, uint8_t trigger, adsr_state_t * state, q15_t *envelopeOut, uint32_t blockSize);

