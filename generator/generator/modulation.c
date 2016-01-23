
#include <stdint.h>
#include <arm_math.h>

#include "q16d15.h"

#include "modulation.h"

q15_t scratch1_q15[MAX_BLOCK_SIZE];
q15_t scratch2_q15[MAX_BLOCK_SIZE];


//#define CLIPQ63_TO_Q31(x) ((q31_t) ((x >> 32) != ((q31_t) x >> 31)) ?	((0x7FFFFFFF ^ ((q31_t) (x >> 63)))) : (q31_t) x)

phasor_model_t create_phasor_model(q16d15_t frequency, uint16_t sampleRate) {
	int64_t freq64 = ((int64_t) frequency)<<32;
	int64_t step64 = freq64/sampleRate;
	
	phasor_model_t model;
	
//wow, this brings in a lot of extra code! like 3 or 4 KB!
//	model.phaseStep=clip_q63_to_q31((q63_t) (period * 2147483648.0f));

	model.phaseStep = step64>>16;
//	arm_float_to_q31(&step, &(model.phaseStep), 1);
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
	int i;
	
	//the accumulator starts at the previous level
	q31_t acc=state->level;
	
	for (i=0;i<blockSize;i++) {
		//increment by the phase step
		acc += model->phaseStep;
		
		//the accumulator overflows to a negative value
		//trim it to 0 instead
		if (acc<0)
			acc+=Q31_MIN;
		  
		// this could be done afterwards with arm_q31_to_q15, but then we'd need an intermediate sample block
		*phaseOut = Q31_TO_Q15(acc);
		
		//increment the phase pointer
		phaseOut++;
	}
	
	state->level = acc;
}

void saw_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize) {
	//this should use arm_copy_q15, but it doesn't work for some reason
	arm_copy_q15(phaseIn, waveOut, blockSize);
	/*
	int i;
	for (i=0;i<blockSize;i++) {
		*waveOut = *phaseIn;
		waveOut++;
		phaseIn++;
	}
	*/
}

void sine_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize) {
	int i;
	for (i=0;i<blockSize;i++) {
		//find the value of the sine wave
		//note that the parameter to arm_sin_q15 is scaled so that [0..1] cooresponds to [0..2*pi]
		*waveOut = 	arm_sin_q15(*phaseIn);
		//scale down the wave and shift it to the range [0..1]
		*waveOut = (*waveOut>>1)+Q15_HALF;
		
		phaseIn++;
		waveOut++;
	}
}
//void tri_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize);

void square_q15(q15_t *phaseIn, q15_t duty,  q15_t * waveOut, uint32_t blockSize) {
	int i;
	for (i=0;i<blockSize;i++) {
		//the output signal is 0 if greater than the duty threshold
		//eg. a duty of 0.75 is 1 whenever the phase is over 0.75
		*waveOut = (*phaseIn<duty) ? Q15_1 : 0;
		waveOut++;
		phaseIn++;
	}	
}

void mix2_q15(q15_t *a, q15_t *b, q15_t * out, uint32_t blockSize) {
	arm_shift_q15(a, -1, scratch1_q15, blockSize);
	arm_shift_q15(b, -1, out, blockSize);
	arm_add_q15(scratch1_q15, out, out, blockSize);
}

//void adsr_q15(adsr_model_t * model, uint8_t trigger, adsr_state_t * state, q15_t *envelopeOut, uint32_t blockSize);

