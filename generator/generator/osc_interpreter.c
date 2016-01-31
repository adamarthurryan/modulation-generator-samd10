
#include <stdint.h>
#include <stdbool.h>
#include <arm_math.h>

#include "q16d15.h"
#include "synth.h"
#include "osc_parser.h"
#include "osc_interpreter.h"


/** Handler functions take an array of argumentValues and return a result.
	The signature of the argument array has already been checked, 
	so the handler can use the arguments without checking shape.
	Similarly, the instance number is guaranteed to be within bounds.*/
typedef osc_result_t(*osc_handler_t)(int, argument_value_t *);

#define OSC_HANDLER(fn) osc_result_t fn(int instance, argument_value_t * args)

OSC_HANDLER(lfoFreq) {
	synth_parameters.lfo[instance].frequency = args[0].qArg;
	return osc_result_success();
}
OSC_HANDLER(lfoShape) {
	//synth_parameters.lfo[instance].shape = ...
	if (strcmp(args[0].stringArg, "saw")==0) {
		synth_parameters.lfo[instance].shape = SAW;
		return osc_result_success();
	}
	else if (strcmp(args[0].stringArg, "sine")==0) {
		synth_parameters.lfo[instance].shape = SINE;
		return osc_result_success();
	}
	else if (strcmp(args[0].stringArg, "square")==0) {
		synth_parameters.lfo[instance].shape = SQUARE;
		return osc_result_success();
	}
	else if (strcmp(args[0].stringArg, "tri")==0) {
		synth_parameters.lfo[instance].shape = TRI;
		return osc_result_success();
	}
	
	return osc_result_error("Unknown wave shape - should be one of sine, tri, saw or square");
}
OSC_HANDLER(lfoDuty) {
	synth_parameters.lfo[instance].duty = args[0].qArg;
	return osc_result_success();
}

OSC_HANDLER(envADSR) {
	synth_parameters.env[instance].attack =  args[0].intArg;
	synth_parameters.env[instance].decay =   args[1].intArg;
	synth_parameters.env[instance].sustain = args[2].qArg;
	synth_parameters.env[instance].release = args[3].intArg;
	return osc_result_success();
}

OSC_HANDLER(envTrigger) {
	bool trigger = args[0].intArg > 0;
	synth_env_trigger(instance, trigger);
	return osc_result_success();
}

OSC_HANDLER(mixLFO) {
	synth_parameters.mix[instance].lfo[0] =   args[0].qArg;
	synth_parameters.mix[instance].lfo[1] =   args[1].qArg;
	return osc_result_success();
}
OSC_HANDLER(mixEnv) {
	synth_parameters.mix[instance].env[0] =   args[0].qArg;
	synth_parameters.mix[instance].env[1] =   args[1].qArg;
	return osc_result_success();
}

OSC_HANDLER(notImplemented) {
	return osc_result_error("Message handler not yet implemented");
}

#define OSC_MAX_NAME_LENGTH 10
#define OSC_MAX_SIGNATURE_LENGTH 10
#define OSC_MAX_NUM_MESSAGES 10

//this is kind of a bulky way to keep the data
//maybe it should be more pointer-y so it could be sparsely filled
typedef struct {
	char * name;
	char * signature;
	osc_handler_t handler;	
} osc_def_message_t;

typedef struct {
	char * name;
	uint8_t numInstances;
	uint8_t numMessages;
	osc_def_message_t messages[OSC_MAX_NUM_MESSAGES];
} osc_def_module_t;

const osc_def_module_t mod_lfo = { "lfo", 2, 3, {{"freq", "f", &lfoFreq}, {"shape", "s", &lfoShape}, {"duty", "f", &lfoDuty}}};
const osc_def_module_t mod_env = {"env", 2, 2, {{"adsr", "iifi", &envADSR}, {"trig", "i", &envTrigger}}};
const osc_def_module_t mod_mix = {"mix", 2, 2, {{"lfo", "ff", &mixLFO}, {"env", "ff", &mixEnv}}};
const osc_def_module_t mod_data = {"data", 2, 2, {{"save", "", &notImplemented}, {"load", "", &notImplemented}, {"reset", "", &notImplemented}}};

osc_def_module_t modules[4];

void osc_interpreter_configure() {
	memcpy(modules, (osc_def_module_t[]) {mod_lfo, mod_env, mod_mix, mod_data}, sizeof(modules));
}


int numModules = 3;


typedef enum {
	LFO, ENV, MIX
} module_select_t;

/** Returns the module index or -1 if no match.*/
int getModuleIndex(char * addr, int length) {
	for (int i=0; i<numModules; i++) {
		if (strncmp(addr, modules[i].name, length)==0)
		return i;
	}
	return -1;
}

/** Returns the message index or -1 if no match.*/
int getMessageIndex(int moduleIndex, char * addr, int length) {
	for (int i=0; i<modules[moduleIndex].numMessages; i++) {
		if (strncmp(addr, modules[moduleIndex].messages[i].name, length)==0)
			return i;
	}
	return -1;	
}

/** Returns true if the signature is correct.*/
int checkMessageSignature(const char * signature, osc_message_t * oscMessage) {
	int length  = strlen(signature);
	if (length != oscMessage->numArguments)
		return false;
	return (strncmp(signature, oscMessage->typeTag, length) == 0);
}

osc_result_t osc_message_interpret(osc_message_t * oscMessage) {
	if (oscMessage->numAddrParts != 3) {
		return osc_result_error("Expected 3 address parts: module/instance/message");
	}
	
	//pick a module
	int moduleIndex = getModuleIndex(oscMessage->addrParts[0], oscMessage->addrPartLengths[0]);
	if (moduleIndex<0)
		return osc_result_error("Unknown module address");
		
	//pick an instance
	if (strlen(oscMessage->addrParts[1]) != 1)
		return osc_result_error("Expect a single-digit instance address number");
		
	int instance = oscMessage->addrParts[1][0]-'0';
	//check instance bounds	
	if (instance>=modules[moduleIndex].numInstances || instance<0)
		return osc_result_error("Instance address out of range");
	
	//pick a message
	int messageIndex = getMessageIndex(moduleIndex, oscMessage->addrParts[2], oscMessage->addrPartLengths[2]);
	if (messageIndex<0)
		return osc_result_error("Unknown message address");
		
	//check message signature
	if (!checkMessageSignature(modules[moduleIndex].messages[messageIndex].signature, oscMessage))
		return osc_result_error("Incorrect message signature");
	
	//call handler and return result
	return (*(modules[moduleIndex].messages[messageIndex].handler))(instance, oscMessage->argumentValues);
}

