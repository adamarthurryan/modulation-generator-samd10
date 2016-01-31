/*
 * synth.h
 *
 * Created: 1/22/2016 4:25:47 PM
 *  Author: Adam
 */ 


#ifndef SYNTH_PARAMETERS_H_
#define SYNTH_PARAMETERS_H_

#define SAMPLE_RATE 11025
#define SAMPLE_PERIOD_Q31 (Q31_1/11025)
#define SAMPLE_BUFFER_SIZE 32


#define NUM_LFOS 2
#define NUM_ENVS 2
#define NUM_MIXERS 2

typedef enum {
	SINE, SAW, SQUARE, TRI
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
	q15_t lfo[NUM_LFOS];
	q15_t env[NUM_ENVS];
} mix_parameters_t;


typedef struct {
	lfo_parameters_t lfo[NUM_LFOS];
	env_parameters_t env[NUM_ENVS];
	mix_parameters_t mix[NUM_MIXERS];
} synth_parameters_t;


synth_parameters_t synth_parameters;

void dsp_configure();

void dsp_run_block (q15_t *);

void synth_env_trigger(int, bool);


#endif /* SYNTH_PARAMETERS_H_ */