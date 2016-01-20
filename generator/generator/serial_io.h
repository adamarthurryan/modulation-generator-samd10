/*
 * serial_io.h
 *
 * Created: 1/19/2016 11:16:06 PM
 *  Author: Adam
 */ 


#ifndef SERIAL_IO._H_
#define SERIAL_IO._H_

#include <stdint.h>


/** Writes the given number of characters from the buffer.*/
void serial_write(char * string, uint16_t length);

/** Writes a null-terminated string (eg. a string literal) */
void serial_write_const(char * string);

/** Reads a line of text from the io stream, up to the given length.
	
	If echo is true, the characters will be echoed back as they are received.

	Returns the number of characters in the line, not including the newline character.
	Returns -1 if the buffer overflows
*/	
int serial_read_line(char * buffer, int length, int echo);


int serial_read_char(char * buffer) ;


void serial_configure() ;

#endif /* SERIAL_IO._H_ */