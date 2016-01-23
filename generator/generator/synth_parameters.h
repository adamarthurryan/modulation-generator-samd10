/*
 * dsp_parameters.h
 *
 * Created: 1/22/2016 4:25:47 PM
 *  Author: Adam
 */ 


#ifndef SYNTH_PARAMETERS_H_
#define SYNTH_PARAMETERS_H_

typedef enum {
	SINE, SAW, SQUARE
} lfo_shape_t;

typedef struct {
	q16d15_t frequency;
	lfo_shape_t shape;
	q15_t duty;
} lfo_parameters_t;

typedef struct {
	int attack;
	int decay;
	int release;
	q15_t sustain;
} env_parameters_t;

typedef struct {
	q15_t lfo[2];
	q15_t env[2];
} mix_parameters_t;

typedef struct {
	lfo_parameters_t lfo[2];
	env_parameters_t env[2];
	mix_parameters_t mix[2];
} synth_parameters_t;

extern volatile synth_parameters_t synth_parameters;




#endif /* SYNTH_PARAMETERS_H_ */