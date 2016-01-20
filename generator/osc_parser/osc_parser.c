/*
 * osc_parser.c
 *
 * Created: 1/19/2016 12:10:57 PM
 *  Author: Adam
 */ 

#include <stdbool.h>
#include <stdlib.h>
#include "osc_parser.h"

osc_parser_result_t osc_parse_error(const char * errorMessage) {
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
	int * signaturePartLengths[2];
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
				oscMessage->argumentTypes[i] = FLOAT;
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
		
		if (oscMessage->argumentTypes[i] == FLOAT)
			oscMessage->argumentValues[i].floatArg = simple_strtof(oscMessage->arguments[i], oscMessage->argumentLengths[i]);
		else if (oscMessage->argumentTypes[i] == INT)
			oscMessage->argumentValues[i].intArg = strtol(oscMessage->arguments[i], NULL, 10);
	}
		
	return osc_parse_success();
}
		
/** Break the string buffer into parts separated by the given character.
 * Returns the number of parts.
 * All instances of the delimiter in the given buffer will be replaced with null characters so that the returned part strings are null-terminated.
 * Returns -1 on overflow (too many parts).*/
int split_string(char * buffer, int length, char delimiter, char ** parts, int * partLengths, int maxParts) {
	
	if (length == 0)
		return 0;
	
	int isDelimiter = false;
	int isStart = true;
	
	int partCount = 0;
	* partLengths = 0;
	while (length>0) {
		
		//the end of a part
		if (*buffer == delimiter && !isDelimiter && !isStart) {
			//set the character to null
			*buffer =  '\0';
			
			//update the state
			isDelimiter = true;
			
			//move to the next part and partLength index
			parts++;
			partLengths++;
		}
		//the beginning of a part
		else if (*buffer != delimiter && (isDelimiter || isStart)) {
			//increment the part count and return on overflow
			partCount++;
			if (partCount>maxParts)
				return -1;
	
			//initialize the part (to the current buffer position) and partLength (to 1)
			*parts = buffer;
			*partLengths = 1;

			//update the state
			isDelimiter = false;
			isStart = false;
		}
		//ordinary characters within a part
		else if (*buffer != delimiter && !isDelimiter && !isStart) {
			(*partLengths)++;
		}
		
		//move to next character in buffer
		length--;
		buffer++;
	}
	return partCount;
}

/** A simple string to float parser. 
	Returns 0 on parse error.*/
float simple_strtof(char * buffer, int length) {
	float value;
	char * parts[2];
	int  partLengths[2];
	int numParts = split_string(buffer, length, '.', parts, partLengths, 2);
	
	if (numParts != 1 && numParts != 2)
		return 0;
	
	value = strtol(parts[0], NULL, 10);
	if (numParts==2) {
		float fractPart = 0;
		for (int i=partLengths[1]-1;i>=0;i--) {
			if (parts[1][i] <'0' || parts[1][i] > '9')
				return 0;
			int digit = (int)(parts[1][i]-'0');
			fractPart += digit;
			fractPart = fractPart * 0.1;
		}
		value += fractPart;
	}
	
	return value;
}