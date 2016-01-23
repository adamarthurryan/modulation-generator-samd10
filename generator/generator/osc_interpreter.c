
#include <stdint.h>
#include <stdbool.h>
#include <arm_math.h>

#include "q16d15.h"
#include "synth_parameters.h"
#include "osc_parser.h"
#include "osc_interpreter.h"

typedef enum {
	LFO, ENV, MIX
} module_select_t;

typedef enum {
	FREQ, SHAPE, DUTY
} lfo_param_select_t;

typedef enum {
	ATTACK, DECAY, SUSTAIN, RELEASE
} env_param_select_t;

typedef enum {
	LFO0, LFO1, ENV0, ENV1
} mix_param_select_t;

typedef union {
	lfo_param_select_t lfoParam;
	env_param_select_t envParam;
	mix_param_select_t mixParam;
} param_select_t;

void osc_apply_message_lfo(int moduleInstance, lfo_param_select_t param, osc_message_t oscMessage) {
	lfo_shape_t shape;
	
	switch(param) {
		case FREQ:
			//!!! assert single q arg > 0
			synth_parameters.lfo[moduleInstance].frequency = oscMessage.argumentValues[0].qArg;
			break;
		case SHAPE:
			//!!! assert single string arg
			//!!! determine shape
			shape = SINE;
			synth_parameters.lfo[moduleInstance].shape = shape;
			break;
		case DUTY:
			//!!! assert single q arg between 0 and 1
			synth_parameters.lfo[moduleInstance].duty = (q15_t) (oscMessage.argumentValues[0].qArg);
			break;
	}
}
void osc_apply_message_env(int moduleInstance, env_param_select_t param, osc_message_t oscMessage) {
}
void osc_apply_message_mix(int moduleInstance, mix_param_select_t param, osc_message_t oscMessage) {
}

void osc_apply_message(module_select_t module, int moduleInstance, param_select_t param, osc_message_t oscMessage) {
	switch (module) {
		case LFO:
			osc_apply_message_lfo(moduleInstance, param.lfoParam, oscMessage);
			break;
		case ENV:
			osc_apply_message_lfo(moduleInstance, param.envParam, oscMessage);
			break;
		case MIX:
			osc_apply_message_lfo(moduleInstance, param.mixParam, oscMessage);
			break;
	}
}

void osc_message_interpret(osc_message_t oscMessage) {
	//!!! assert 3 addr parts
	
	//first addr is module
	//should be "lfo", "env" or "mix"
	//!!!
	module_select_t module = LFO;
	
	//instance should be "0" or "1"
	//!!! assert shape
	int moduleInstance = oscMessage.addrParts[1][0] - '0';
	
	//the allowed parameters depend on the module
	param_select_t param;
	param.lfoParam = FREQ;
	
	osc_apply_message(module, moduleInstance, param, oscMessage);
}

