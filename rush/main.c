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
	WAITING_FOR_ROLE = 1,
	CONFIRM_ROLE = 2,
	START_GAME = 3,
	BLINKING_LED = 4,
	WAITING_INPUT = 5,
	WHO_WON = 6 
};

volatile enum ENUM_GAME_STATE game_state = WAITING_FOR_ROLE;

char g_my_time = 0;
char g_master = 0;
char g_time_adverse = 0;

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

void	get_my_time() {
	g_my_time = TCNT1 % 127;
}


void wait(uint32_t mhz)
{
	while(--mhz)
		;
}

void send_my_time() {
	uart_tx(g_my_time);
}


void check_master() {
	wait(22222);
	uart_tx(g_master);
}


/*
 *   sei() is not activated in this function.
 */
void set_interrupt_USART_RX()
{
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void init_timer1_interrupt(unsigned int int_time) 
{

	TCCR1B |= (1 << WGM13) | (1 << WGM12); // mode CTC 12 TOP=ICR1
	TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 (From prescaler)

	//	TIMSK1 = (1 << ICIE1); // Timer/Counter1, Output Compare A Match Interrupt Enable

	ICR1 = int_time;
	/* Interrupt every minute*/

}

void who_s_the_master() 
{
	if(game_state == WAITING_FOR_ROLE)
	{
		get_my_time();
		wait(40000);
		send_my_time();
	}
}


void start_game() {
	if (game_state == START_GAME)
	{
		DDRB=15;
		PORTB = 0b1111;
	}
}

ISR(USART_RX_vect) // should be TX instead of RX
{
	char char_received = uart_rx();

	if (game_state == WAITING_FOR_ROLE)
	{
		if (g_my_time == char_received)
		{
			wait(200);
			get_my_time();
			send_my_time();
		}
		else
		{
			g_master = g_my_time > char_received;
			game_state = CONFIRM_ROLE;
			PORTB = game_state;
			check_master();
		} 
	} 
	else if (game_state == CONFIRM_ROLE)
	{
		char opponent_role = char_received;
		if ( g_master != opponent_role && (opponent_role == 0 || opponent_role == 1))
		{
			game_state= START_GAME;
			while(1)
				PORTB=g_master;
		}
		else
			game_state--;
	}

}



void main(void)
{
	DDRB=0;
	uart_init(MYUBRR);
	init_timer1_interrupt(8449);

	set_interrupt_USART_RX();
	sei();
	PORTB = game_state;	

	for (;;)
	{
		wait(4000);
		who_s_the_master();
		start_game();
	}
}
