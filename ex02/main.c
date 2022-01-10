
#include <avr/io.h>


int main(int argc, char **argv)
{

	PORTB = 1<<PORTB3; // select the bit of portb to set to Write value 
	DDRB = 1<<DDB3; // put the ddb3 bit to 1 
	return (0);
}
