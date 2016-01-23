/*
 * IncFile1.h
 *
 * Created: 1/22/2016 9:36:38 AM
 *  Author: Adam
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

//#define SIMPLE_STRNUM_ENABLE_FLOAT


int split_string(char * buffer, int length, char delimiter, char ** parts, int * partLengths, int maxParts);


int simple_qtos(char * buffer, int size, q16d15_t value, int precision);
int simple_itos(char * buffer, int size, int32_t value);


q16d15_t simple_strtoq(char * buffer, int length) ;
int32_t simple_strtol(char * buffer, int length) ;

#ifdef SIMPLE_STRNUM_ENABLE_FLOAT
	int simple_ftos(char * buffer, int size, float value, int precision);
	float simple_strtof(char * buffer, int length) ;
#endif 

#endif /* INCFILE1_H_ */