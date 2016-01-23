/*
 * simple_strnum.c
 *
 * Created: 1/22/2016 9:37:40 AM
 *  Author: Adam
 */ 

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "q16d15.h"

#include "simple_strnum.h"

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



/** Fast calculation of a non-negative power of 10. 
	* Returns -1 on overflow (power>9).*/
int32_t pow10i(int pow) {
	switch (pow) {
		case 0: return 1;
		case 1: return 10;
		case 2: return 100;
		case 3: return 1000;
		case 4: return 10000;
		case 5: return 100000;
		case 6: return 1000000;
		case 7: return 10000000;
		case 8: return 100000000;
		case 9: return 1000000000;
		default: return -1;
	}
}

/** Fast calculation of log10(n). 
 * From http://stackoverflow.com/questions/1068849/how-do-i-determine-the-number-of-digits-of-an-integer-in-c*/
int log10i (int32_t n) {
	//get abs of n (being carefull about minint/maxint
    if (n < 0) n = (n == INT32_MIN) ? INT32_MAX : -n;

    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    /*      2147483647 is 2^31-1 - add more ifs as needed
       and adjust this final return as well. */
    return 10;
}

int simple_qtos(char * buffer, int size, q16d15_t value, int precision) {
	//the integer part is everything above the last 15 digits	
	q16d15_t intPart = WHOLE_Q16D15(value);
	
	//the fractional part is the last 15 digits
	//the fractional part will always be positive
	q16d15_t fractPart = FRACT_Q16D15(value);
	
	
	//length starts with the final null character already counted
	int length = 1;
	
	int itosLength = simple_itos(buffer, size-length, intPart);
	if (itosLength==-1)
		return -1;
	
	//erase the last null character
	buffer +=itosLength-1;
	length +=itosLength-1;
	
	//add decimal point
	if (size-length<1)
		return -1;
	*buffer = '.';
	buffer++;
	length++;
	

	//output the fractional part
	if (precision>5)
		precision = 5;
	
	if (size-length<precision)
		return -1;
		
	int acc = fractPart;
	for (int i=0;i<precision;i++) {
		acc = acc*10;
		int digit = WHOLE_Q16D15(acc);
		acc = FRACT_Q16D15(acc);
		
		buffer[i]='0'+digit;
	}
	
	length+=precision;
	buffer+=precision;

	//null terminate
	*buffer = '\0';
	
	return length;
}




int simple_itos(char * buffer, int size, int32_t value) {
	int length = 0;
	int numDigits = log10i(value);
	
	//test buffer size
	if (size-numDigits-(value<0?1:0)-1 < 0)
		return -1;
	
	//add minus sign
	if (value<0) {
		value = -value;
		*buffer = '-';
		buffer++;
		length++;
	}

	//add digits
	for (int i=0;i<numDigits;i++) {
		int digit = value %10;
		value = value/10;
		buffer[numDigits-i-1]='0'+digit;
	}
	length += numDigits;
	
	//add null
	buffer[numDigits] = '\0';
	length++;	
	
	return length;
}

/** A simple string to int parser.*/
int32_t simple_strtol(char * buffer, int length) {
	
	//it is an error if a string with no length is passed
	if (length==0)
		return 0;

	//test if the number is negative
	int isNegative = (*buffer=='-')?true:false;
	
	//skip the minus character 
	if (isNegative) {
		buffer++;
		length++;
	}
		
	int num = 0;
	for (int i=0;i<length;i++) {
		//stop if we find a null
		if (buffer[i] == '\0')
			break;
			
		//any non-digit character is an error
		if (buffer[i] < '0' || buffer[i] > '9')
			return 0;
			
		//shift the number and add the next digit
		num*=10;
		num += buffer[i] - '0';
	}
	
	//apply the negative sign
	if (isNegative)
		num = -num;
		
	return num;
}

#define Q15_TENTH ((1<<15)/10)

//converts the input string to q16.15
q16d15_t simple_strtoq(char * buffer, int length) {
	q16d15_t value;
	char * parts[2];
	int  partLengths[2];
	int numParts = split_string(buffer, length, '.', parts, partLengths, 2);
	
	if (numParts != 1 && numParts != 2)
	return 0;
	
	value = simple_strtol(parts[0], partLengths[0]) << 15;
	if (numParts==2) {
		q16d15_t fractPart = 0;
		for (int i=partLengths[1]-1;i>=0;i--) {
			if (parts[1][i] <'0' || parts[1][i] > '9')
			return 0;
			int digit = (int)(parts[1][i]-'0')<<15;
			fractPart += digit;
			
			//fractPart = fractPart/10;
			//following is much faster, but less accurate
			//maybe with a q63 and a Q4x_TENTH it would be more accurate
			fractPart = (fractPart * Q15_TENTH)>>15;
		}
		value += fractPart;
		
	}
	
	return value;
}



#ifdef SIMPLE_STRNUM_ENABLE_FLOAT

int simple_ftos(char * buffer, int size, float value, int precision) {
	int intPart = (int)value;
	int fractPart = (int)((value-intPart)*pow10i(precision));
	if (fractPart < 0)
	fractPart = -fractPart;
	
	//length starts with the final null character already counted
	//!!! is it consistent to report the null character in length?
	int length = 1;
	
	//the integer buffer is the size of the largest representable integer
	char intBuffer[10];
	int itosLength = simple_itos(intBuffer, 10, intPart);
	if (itosLength==-1 || size-length < itosLength)
	return -1;
	
	for (int i=0;i<itosLength-1;i++) {
		buffer[i] = intBuffer[i];
	}
	
	//erase the last null character
	buffer +=itosLength-1;
	length +=itosLength-1;
	
	//add decimal point
	if (size-length<1)
	return -1;
	*buffer = '.';
	buffer++;
	length++;
	
	//render the fractional part to a string
	char fractBuffer[precision+1];
	itosLength = simple_itos(fractBuffer, precision+1, fractPart);
	
	if (itosLength == -1 || (size-length < itosLength-1))
	return -1;
	
	//output with leading zeros
	int numLeadingZeros = precision - (itosLength-1);
	for (int i=0; i<numLeadingZeros; i++) {
		buffer[i] = '0';
	}
	buffer += numLeadingZeros;
	length += numLeadingZeros;
	
	for (int i=0; i<itosLength-1; i++) {
		buffer[i] = fractBuffer[i];
	}
	buffer += itosLength-1;
	length += itosLength-1;
	
	//null terminate
	*buffer = '\0';
	
	return length;
}
/** A simple string to float parser. 
	Uses simple_strtol.
	Returns 0 on parse error.
	
	!!! Perhaps this should convert directly to q_31?*/

float simple_strtof(char * buffer, int length) {
	float value;
	char * parts[2];
	int  partLengths[2];
	int numParts = split_string(buffer, length, '.', parts, partLengths, 2);
	
	if (numParts != 1 && numParts != 2)
	return 0;
	
	value = simple_strtol(parts[0], partLengths[0]);
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


#endif

