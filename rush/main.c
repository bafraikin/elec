#include <avr/io.h>

#define ISR(vector, ...)                                                 \
	void vector(void) __attribute__((signal, __INTR_ATTRS)) __VA_ARGS__; \
	void vector(void)

#define sei() __asm__ __volatile__("sei" :: \
		: "memory")
#define cli() __asm__ __volatile__("cli" :: \
		: "memory")

#define F_CPU 16000000 // Clock Speed
#define UART_BAUDRATE 9600
#define MYUBRR F_CPU / 16 / UART_BAUDRATE


enum ENUM_GAME_STATE {
	WAITING_FOR_ROLE,
	CONFIRM_ROLE,
	START_GAME,
	BLINKING_LED,
	WAITING_INPUT,
	WHO_WON
};

enum ENUM_GAME_STATE game_state = WAITING_FOR_ROLE;

char g_my_time = 0;
char g_master = 0;
uint16_t opponent_time = 0;

void uart_init(unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	/* Set frame format: 7data, 2stop bit   20.10*/
	UCSR0C = (1<<USBS0)| (3<<UCSZ00) | (1 << UCSZ01) | (3 << UPM00);
}

void wait(uint32_t mhz)
{
	while(--mhz)
		;
}

void	get_my_time() {
	g_my_time = (TCNT1 % 255);
}

/* putchar*/
void uart_tx(unsigned char data)
{
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1 << UDRE0)))
		;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

/*putstr*/
void uart_printstr(const char *str)
{
	while (*str)
	{
		uart_tx(*str++);
	}
}

/* READ USART*/
char uart_rx(void)
{
	// UCSRnB  = (1 << RXENn)
	while (!(UCSR0A & (1 << RXC0)))
		;
	/* Get and return received data from buffer */
	return UDR0;
}

void check_master() {
	PORTB=0b1000;
	wait(22222);
	uart_tx(g_master);
}


ISR(USART_RX_vect) // should be TX instead of RX
{
	char char_received = uart_rx();
	//	uart_tx(uart_rx());

	if (game_state == WAITING_FOR_ROLE)
	{
		/* handshake*/
		// stop sending number temporaly
		if (g_my_time == char_received)
		{
			while(1)
				PORTB=15;
			get_my_time();
		}
		else
		{
			g_master = g_my_time > char_received;
			game_state++;
			check_master();
		}
		// send passe code
		// if pass code good next game state
		// otherwise restart sending 
	}
	else if (game_state == CONFIRM_ROLE)
	{
		char opponent_role = char_received;

		if ( g_master != opponent_role && (opponent_role == 0 || opponent_role ==  1))
			game_state++;
		else
			game_state--;
	}

}

/*
 *   sei() is not activated in this function.
 */
void set_interrupt_USART_RX()
{
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void init_timer1_interrupt(unsigned int int_time){

	TCCR1B |= (1 << WGM13) | (1 << WGM12); // mode CTC 12 TOP=ICR1
	TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 (From prescaler)

	TIMSK1 = (1 << OCIE1A); // Timer/Counter1, Output Compare A Match Interrupt Enable

	ICR1 = int_time;
	/* Interrupt every minute*/

}

void who_s_the_master() {

	while(game_state == WAITING_FOR_ROLE)
	{
		uart_tx(g_my_time);
	}
}


void start_game() {
	if (game_state == START_GAME)
		PORTB = g_master;
}


void main(void)
{
	uart_init(MYUBRR);
	init_timer1_interrupt(8449);
	wait(200);
	get_my_time();
	set_interrupt_USART_RX();
	sei();


	for (;;)
	{
		who_s_the_master();
		start_game();
	}
}
