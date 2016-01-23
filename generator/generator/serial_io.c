#include "serial_io.h"

#include "atmel_start.h"

struct io_descriptor *io;

/** Writes the given number of characters from the buffer.*/
void serial_write(char * string, uint16_t length) {
	io_write(io, (uint8_t *)string, length);
}

/** Writes a null-terminated string (eg. a string literal) */
void serial_write_const(char * string) {
	serial_write(string, strlen(string));
}
/** Reads a line of text from the io stream, up to the given length.
	
	If echo is true, the characters will be echoed back as they are received.

	Returns the number of characters in the line, not including the newline character.
	Returns -1 if the buffer overflows
*/	
int serial_read_line(char * buffer, int length, int echo) {
	int count = 0;
	while (true) {
		int read = io_read(io, buffer, 1);
		if (read==1 && (*buffer == '\n') ) {
			*buffer = '\0';
			if (echo)
				io_write(io,"\n", 1);
			return count;
		}
		
		//ignore the carriage return character
		if (*buffer == '\r')
			continue;
			
		if (echo)
			io_write(io,buffer, 1);
			
		count++;
		
		if (count>=length)
			return -1;
			
		buffer++;
	}
}
int serial_read_char(char * buffer) {
	return io_read(io, buffer, 1);
}


void serial_configure() {
	usart_sync_get_io_descriptor(&USART_0, &io);
	usart_sync_enable(&USART_0);
}