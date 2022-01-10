
#include <avr/io.h>

void	wait(long mhz)
{
	long i;

	i = -1;
	while (++i < mhz)
		;
}


int main(int argc, char **argv)
{
	while(1)
	{
		wait(400000);
		PORTB ^= 1<<PORTB3;
		DDRB ^= 1<<DDB3; // put the ddb3 bit to 1 
		wait(400000);
	}
}
