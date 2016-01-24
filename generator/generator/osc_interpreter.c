
#include <stdint.h>
#include <stdbool.h>
#include <arm_math.h>

#include "q16d15.h"
#include "synth_parameters.h"
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
	return osc_result_error("Message handler not yet implemented");
}
OSC_HANDLER(lfoDuty) {
	synth_parameters.lfo[instance].duty = args[0].qArg;
	return osc_result_success();
}

OSC_HANDLER(envADSR) {
	synth_parameters.env[instance].attack =  args[0].qArg;
	synth_parameters.env[instance].decay =   args[1].qArg;
	synth_parameters.env[instance].sustain = args[2].qArg;
	synth_parameters.env[instance].release = args[3].qArg;
	return osc_result_success();
}

OSC_HANDLER(envTrigger) {
	return osc_result_error("Message handler not yet implemented");	
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


int numModules = 3;
const char modules[3][4] = {{"lfo"}, {"env"}, {"mix"}};
int numInstances[3] = {2,2,2};
int numMessages[3] = {3,2,2};
const char messages[3][3][6] = {{{"freq"}, {"shape"}, {"duty"}}, {{"adsr"}, {"trig"}}, {{"lfo"}, {"env"}}};
const char signatures[3][3][5] = {{{"f"}, {"s"}, {"f"}}, {{"ffff"}, {"i"}}, {{"ff"}, {"ff"}}};
osc_handler_t handlers[3][3] = {{&lfoFreq,&lfoShape,&lfoDuty},{&envADSR,&envTrigger},{&mixLFO,&mixEnv}};


typedef enum {
	LFO, ENV, MIX
} module_select_t;

/** Returns the module index or -1 if no match.*/
int getModuleIndex(char * addr, int length) {
	for (int i=0; i<numModules; i++) {
		if (strncmp(addr, modules[i], length)==0)
		return i;
	}
	return -1;
}

/** Returns the message index or -1 if no match.*/
int getMessageIndex(int moduleIndex, char * addr, int length) {
	for (int i=0; i<numMessages[moduleIndex]; i++) {
		if (strncmp(addr, messages[moduleIndex][i], length)==0)
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
	if (oscMessage->addrParts[1] != 1)
		return osc_result_error("Expect a single-digit instance address number");
		
	int instance = oscMessage->addrParts[1][0]-'0';
	//check instance bounds	
	if (instance>=numInstances[moduleIndex] || instance<0)
		return osc_result_error("Instance address out of range");
	
	//pick a message
	int messageIndex = getMessageIndex(moduleIndex, oscMessage->addrParts[2], oscMessage->addrPartLengths[2]);
	if (messageIndex<0)
		return osc_result_error("Unknown message address");
		
	//check message signature
	if (!checkMessageSignature(signatures[moduleIndex][messageIndex], oscMessage))
		return osc_result_error("Incorrect message signature");
	
	//call handler and return result
	return (*handlers[moduleIndex][messageIndex])(instance, oscMessage->argumentValues);
}

