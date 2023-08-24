#include <string.h>
#include "OtdCircularBuffer_App.h"
#include "OtdUART.h"

#define INVALID_UART  2

extern uint8_t tx_pending;//flag to confirm all bytes have transmitted
extern uint8_t result;
portType debug_port = 0;

/*Only RX is recieved in circular buffer*/
Circular_Buffer debug_rx = {{INIT_VAL},INIT_VAL,INIT_VAL};

/*pointer for rx circular buffer for debug port*/
Circular_Buffer *debug_rx_ptr;

void CircularBuffer_init(void)
{
	/*Initial buffers*/
	debug_rx_ptr = &debug_rx;
	/*initialize UART1*/
	OtdUart_Init();
}

void buff_store_char(unsigned char c, Circular_Buffer *buffer)
{
	uint16_t loc = (buffer->head + 1) % UART_BUFFER_SIZE;
	
	/*check if no overflow will occur*/
	if(loc != buffer->tail)
	{
		buffer->buffer[buffer->head] = c;
		/*update head*/
		buffer->head = loc;
	}	
}

/*find a substring within a string*/
int8_t find_str(char *substring, char *mainstring)
{
	int l,i,j;
	
	/*count substring i.e. substring*/
	for(l=0 ; substring[l] != '\0' ; l++){}
	
	for(i=0 , j=0 ; mainstring[i] != '\0' && substring[j] != '\0'; i++)
	{
		if(mainstring[i] == substring[j])
		{
			j++;
		}
		else
		{
			j=0;
		}
	}
	if(j == l)
	{
		return 1;
	}
	else
	{
		return 2;
	}
	
}

uint16_t buffer_clear(portType uart)
{
	if(uart == debug_port)
	{
		/*set buffer content to '\0'*/
		memset(debug_rx_ptr->buffer, '\0', UART_BUFFER_SIZE);
		debug_rx_ptr->head = 0;
	}
	else
	{
		return INVALID_UART;
	}
}

uint16_t buffer_peek(portType uart)
{
	if(uart == debug_port)
	{
		if(debug_rx_ptr->head == debug_rx_ptr->tail)
		{
			return 2;
		}
		else
		{
			return debug_rx_ptr->buffer[debug_rx_ptr->tail];
		}
	}
	else
	{
		return INVALID_UART;
	}
}

uint16_t buffer_read(portType uart)
{
	unsigned char c ;
	if(uart == debug_port)
	{
		if(debug_rx_ptr->head == debug_rx_ptr->tail)
		{
			return 2;
		}
		else
		{
			c = debug_rx_ptr->buffer[debug_rx_ptr->tail];
			debug_rx_ptr->tail = (uint16_t)(debug_rx_ptr->tail + 1) % UART_BUFFER_SIZE;
			return c;
		}
	}
	else
	{
		return INVALID_UART;
	}
}

void buffer_send_string(const char *s, portType uart)
{
	if(uart == debug_port)
	{
		while(*s != '\0')
		{
			//R_UART1_Send(s++, 1);
			//OtdUart_Send(s++, 1);
			OtdUart_Send((uint8_t *__near)s++, 1);
			while(!tx_pending);
			tx_pending = 0;
		}

	}	
}

uint16_t is_data(portType uart)
{
	if(uart == debug_port)
	{
		return (uint16_t)(UART_BUFFER_SIZE + debug_rx_ptr->head - debug_rx_ptr->tail) % UART_BUFFER_SIZE;
	}
	else
	{
		return INVALID_UART;
	}	
}



void debug_rx_uart_callback(void)
{
	unsigned char c = result;
	buff_store_char(c, debug_rx_ptr);
	//OtdUart_Recieve(&result,1);
}

/*Function to get the position for the first character of a string in buffer*/

void get_first_char(char *str)
{
	/*make sure there is data in the buffer*/
	while(!is_data(debug_port)){}
	
	while(buffer_peek(debug_port)!= str[0])
	{
		debug_rx_ptr->tail = (uint16_t)(debug_rx_ptr->tail + 1) % UART_BUFFER_SIZE;
		while(!is_data(debug_port)){}
	}
}

int8_t is_response(char *str)
{
	int curr_pos = 0;
	int len = strlen(str);

	while( curr_pos != len)
	{
		curr_pos = 0;
		get_first_char(str);

		while(buffer_peek(debug_port) == str[curr_pos])
		{
			curr_pos++;

			buffer_read(debug_port);

			if(curr_pos == len)
			{
				return 1;
			}

			while(!is_data(debug_port)){}
		}
	}
	if(curr_pos == len)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}
