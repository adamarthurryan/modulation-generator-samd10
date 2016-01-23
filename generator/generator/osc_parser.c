/*
 * osc_parser.c
 *
 * Created: 1/19/2016 12:10:57 PM
 *  Author: Adam
 */ 

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "q16d15.h"

#include "simple_strnum.h"

#include "osc_parser.h"

osc_parser_result_t osc_parse_error(char * errorMessage) {
	osc_parser_result_t result;
	result.errorMessage = errorMessage;
	result.errorMessageLength = strlen(errorMessage);
	result.hasError = true;
	return result;
}

osc_parser_result_t osc_parse_success() {
	osc_parser_result_t result;
	result.hasError = false;
	return result;
}

osc_parser_result_t osc_parse(char * line, int lineLength, osc_message_t * oscMessage) {
	
	
	//split the string into space-delimited parts
	char * parts[MAX_CMD_PARTS];
	int partLengths[MAX_CMD_PARTS];
	int numParts = split_string(line, lineLength, ' ', parts,  partLengths, MAX_CMD_PARTS);
		
	if (numParts<-1) {
		return osc_parse_error("Invalid command: too many parts");
	}
	else if (numParts==0)
		return osc_parse_error("No command");

	//break the type tag from the address
	char * signatureParts[2];
	int signaturePartLengths[2];
	int numSignatureParts = split_string(parts[0], partLengths[0], ',', signatureParts, signaturePartLengths, 2);
		
	if (numSignatureParts!=2 && numSignatureParts!=1) {
		return osc_parse_error("Invalid command: incorrect [address,type] format");
	}
		
	oscMessage->hasTypeTag = (numSignatureParts==2);
	oscMessage->typeTag = signatureParts[1];
	oscMessage->typeTagLength = signaturePartLengths[1];
		
	
	//break up the address parts
	oscMessage->numAddrParts = split_string(signatureParts[0], signaturePartLengths[0], '/', oscMessage->addrParts, oscMessage->addrPartLengths, MAX_CMD_ADDR_PARTS);
		
	if (oscMessage->numAddrParts==0) {
		return osc_parse_error("Invalid command: no address");
	}
	else if (oscMessage->numAddrParts<-1) {
		return osc_parse_error("Invalid command: too many address parts");
	}

	//copy the arguments parts
	memcpy(oscMessage->arguments, parts+1, (MAX_CMD_PARTS-1)*sizeof(char *));
	memcpy(oscMessage->argumentLengths, partLengths+1, (MAX_CMD_PARTS-1)*sizeof(int *));
		
	oscMessage->numArguments = numParts - 1;
		
	
	//if there is a typeTag part, ensure that there are as many types as arguments
	if (oscMessage->hasTypeTag && oscMessage->typeTagLength != oscMessage->numArguments)
		return osc_parse_error("Mismatch between type tag length and number of arguments");
	
	//finally iterate the arguments to create the argument values and number of arguments");
	for (int i=0;i<oscMessage->numArguments;i++) {
		//determine the type of argument from the tag
		if (oscMessage->hasTypeTag) {
			if (oscMessage->typeTag[i] == 'f') 
				oscMessage->argumentTypes[i] = Q;
			else if (oscMessage->typeTag[i] == 'i')
				oscMessage->argumentTypes[i] = INT;
			else if (oscMessage->typeTag[i] == 's')
				oscMessage->argumentTypes[i] = STRING;
			else
				return osc_parse_error("Unknown type - should be one of f, i or s");
		}
		else {
			return osc_parse_error("Type detection not yet implemented, supply a type tag string");
		}
		
		//parse the argument values
		
		//	if (oscMessage->argumentTypes[i] == FLOAT)
		//		oscMessage->argumentValues[i].floatArg = simple_strtof(oscMessage->arguments[i], oscMessage->argumentLengths[i]);
		
		char ** arguments = oscMessage->arguments;
		char * argument = arguments[i];
		int * argumentLengths = oscMessage->argumentLengths;
		int argumentLength = argumentLengths[i];
		
		if (oscMessage->argumentTypes[i] == Q)
			oscMessage->argumentValues[i].qArg = simple_strtoq(argument, argumentLength);
		else if (oscMessage->argumentTypes[i] == INT)
			oscMessage->argumentValues[i].intArg = simple_strtol(argument, argumentLength);
		else if (oscMessage->argumentTypes[i] == STRING)
			oscMessage->argumentValues[i].stringArg = argument;
			
	}
		
	return osc_parse_success();
}
		

