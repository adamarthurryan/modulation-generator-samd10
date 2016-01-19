/*
 * osc_parser.h
 *
 * Created: 1/19/2016 12:11:11 PM
 *  Author: Adam
 */ 


#ifndef OSC_PARSER_H_
#define OSC_PARSER_H_

#define MAX_CMD_PARTS 8
#define MAX_CMD_ADDR_PARTS 8

typedef struct {
	const char * errorMessage;
	int errorMessageLength;
	int hasError;
	
} osc_parser_result_t;

typedef struct {
	char * addrParts[MAX_CMD_ADDR_PARTS];
	int addrPartLengths[MAX_CMD_ADDR_PARTS];
	int numAddrParts;
	char * arguments[MAX_CMD_PARTS-1];
	int argumentLengths[MAX_CMD_PARTS-1];
	int numArguments;
	int hasTypeTag;
	char * typeTag;
	int typeTagLength;
} osc_message_t;

osc_parser_result_t osc_parse(char * line, int lineLength, osc_message_t * command);
int split_string(char * buffer, int length, char delimiter, char ** parts, int * partLengths, int maxParts);


#endif /* OSC_PARSER_H_ */