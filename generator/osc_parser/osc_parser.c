/*
 * osc_parser.c
 *
 * Created: 1/19/2016 12:10:57 PM
 *  Author: Adam
 */ 

#include "osc_parser.h"


#define true 1
#define false 0

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
		
	char * parts[MAX_CMD_PARTS];
	int partLengths[MAX_CMD_PARTS];
	int numParts = split_string(line, lineLength, ' ', parts,  partLengths, MAX_CMD_PARTS);
		
	if (numParts<-1) {
		return osc_parse_error("Invalid command: too many parts");
	}
	else if (numParts==0)
		return osc_parse_error("No command");


	char * signatureParts[2];
	int * signaturePartLengths[2];
	int numSignatureParts = split_string(parts[0], partLengths[0], ',', signatureParts, signaturePartLengths, 2);
		
	if (numSignatureParts!=2 && numSignatureParts!=1) {
		return osc_parse_error("Invalid command: incorrect [address,type] format");
	}
		
	oscMessage->hasTypeTag = (numSignatureParts==2);
	oscMessage->typeTag = signatureParts[1];
	oscMessage->typeTagLength = signaturePartLengths[1];
		
		
	oscMessage->numAddrParts = split_string(signatureParts[0], signaturePartLengths[0], '/', oscMessage->addrParts, oscMessage->addrPartLengths, MAX_CMD_ADDR_PARTS);
		
	if (oscMessage->numAddrParts==0) {
		return osc_parse_error("Invalid command: no address");
	}
	else if (oscMessage->numAddrParts<-1) {
		return osc_parse_error("Invalid command: too many address parts");
	}

	memcpy(oscMessage->arguments, parts+1, (MAX_CMD_PARTS-1)*sizeof(char *));
	memcpy(oscMessage->argumentLengths, partLengths+1, (MAX_CMD_PARTS-1)*sizeof(int *));
		
	oscMessage->numArguments = numParts - 1;
		
	return osc_parse_success();
}
		
/** Break the string buffer into parts separated by the given character.
 * Returns the number of parts.
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
