
#include <stdint.h>
#include <arm_math.h>

#include "q16d15.h"

#include "modulation.h"

q15_t scratch1_q15[MAX_BLOCK_SIZE];
//q15_t scratch2_q15[MAX_BLOCK_SIZE];


//#define CLIPQ63_TO_Q31(x) ((q31_t) ((x >> 32) != ((q31_t) x >> 31)) ?	((0x7FFFFFFF ^ ((q31_t) (x >> 63)))) : (q31_t) x)

//!!! This could be made more efficient if we used the q15 reciprocal of the sample rate
//then:  step64 = (int64_t) frequency * sampleRateReciprocal
//this is a ... q1d30 value?
phasor_model_t create_phasor_model(q16d15_t frequency, q31_t samplePeriod) {
	int64_t freq64 = (int64_t) frequency;
	int64_t step64 = freq64*samplePeriod;
	
	phasor_model_t model;

	model.phaseStep = step64>>16;
	return model;
}

adsr_model_t create_adsr_model(q16d15_t attack, q16d15_t decay, q16d15_t release, q15_t sustain, q31_t samplePeriod) {
	adsr_model_t model;
	q31_t sustain_q31 = sustain<<16;
	model.sustain = sustain_q31;

	//this is q15d16 format
	int32_t nAttackSteps = (Q31_1 / attack);
	//this is q15d47 format
	int64_t astep_64 = ((int64_t) nAttackSteps)*samplePeriod;
	model.astep = (q31_t) (astep_64 >> 16);
	
	//this is q15d16 format
	int32_t nDecaySteps = ((Q31_1-sustain_q31) / decay);
	//this is q47d16 format
	int64_t dstep_64 = ((int64_t) nDecaySteps)*samplePeriod;
	model.dstep = (q31_t) (dstep_64 >> 16);

	//this is q15d16 format
	int32_t nReleaseSteps = ((sustain_q31) / release);
	//this is q47d16 format
	int64_t rstep_64 = ((int64_t) nReleaseSteps)*samplePeriod;
	model.rstep = (q31_t) (rstep_64 >> 16);
	
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
	arm_copy_q15(phaseIn, waveOut, blockSize);
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

//!!! Should be able to specify a duty for the tri wave
void tri_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize) {
	//the output signal goes from 0 to 1 as the phase goes from 0 to 0.5,
	//then the output goes back down to 0
	arm_offset_q15(phaseIn, -Q15_HALF, waveOut, blockSize);
	arm_abs_q15(waveOut, waveOut, blockSize);
	arm_scale_q15(waveOut, -Q15_1, 1, waveOut, blockSize);
	arm_offset_q15(waveOut, Q15_1, waveOut, blockSize);
}

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

void mix2_q15(q15_t *a, q15_t aLevel, q15_t *b, q15_t bLevel, q15_t * out, uint32_t blockSize) {
	arm_scale_q15(a, aLevel, 0, scratch1_q15, blockSize);
	arm_scale_q15(b, bLevel, 0, out, blockSize);
	arm_add_q15(scratch1_q15, out, out, blockSize);
}

void adsr_q15(adsr_model_t * model, uint8_t trigger, adsr_state_t * state, q15_t *envelopeOut, uint32_t blockSize) {
	if (state->mode == STATE_R && trigger!=0) {
		state->mode = STATE_A;
	}
	else if (state->mode != STATE_R && trigger == 0) {
		state->mode = STATE_R;
	}
	
	q31_t level = state->level;
	
	for (int i=0;i<blockSize;i++) {
		switch (state->mode) {
			case STATE_A:
				if (Q31_1 - model->astep < level) {
					level = Q31_1;
					state->mode = STATE_D;
				}
				else
					level += model->astep;
				
				break;
			case STATE_D:
				if ((model->sustain + model->dstep < 0) || model->sustain + model->dstep > level) {
					level = model->sustain;
					state->mode = STATE_S;
				}
				else
					level -= model->dstep;
				break;
			case STATE_R:
				if (0 + model->rstep > level) {
					level = 0;
				}
				else
					level -= model->rstep;
					
				break;
			case STATE_S:
				break;
		}
		*envelopeOut = level>>16;
		envelopeOut++;
	}
	state->level = level;
}

