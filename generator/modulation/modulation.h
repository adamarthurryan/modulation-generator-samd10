
#ifndef MODULATION_H_
#define MODULATION_H_

#define ARM_MATH_CM0PLUS true

#include <stdint.h>
#include <arm_math.h>

//with a sample rate of 44100 and q31 bits of resolution on the signals
//we can track from 0.0001 Hz to 22050 Hz with good accuracy

//at q15 and 1102 Hz sample rate, the frequency range is limited to 0.02 Hz to 501 Hz

//for modulation generating, we should be able to use a sample rate of 11025

typedef struct {
	q31_t phaseStep;
} phasor_model_t;

typedef struct {
	q31_t level;
} phasor_state_t;

typedef struct {
	q31_t astep;
	q31_t dstep;
	q31_t rstep;
	q31_t sustain;
} adsr_model_t;

typedef struct {
	q31_t level;
	enum {STATE_A,STATE_D,STATE_R} mode;
} adsr_state_t;

phasor_model_t create_phasor_model(float frequency, uint16_t sampleRate);
adsr_model_t create_adsr_model(uint16_t attackMs, uint16_t decayMs, uint16_t releaseMs, float sustain, uint16_t sampleRate);

phasor_state_t initialize_phasor_state();
adsr_state_t initialize_adsr_state();

void phasor_q15(phasor_model_t * model, phasor_state_t * state, q15_t *phaseOut, uint32_t blockSize);

void saw_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize);
void sine_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize);
void tri_q15(q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize);
void square_q15(q15_t duty, q15_t *phaseIn, q15_t * waveOut, uint32_t blockSize);

void adsr_q15(adsr_model_t * model, uint8_t trigger, adsr_state_t * state, q15_t *envelopeOut, uint32_t blockSize);



#endif /* MODULATION_H_ */