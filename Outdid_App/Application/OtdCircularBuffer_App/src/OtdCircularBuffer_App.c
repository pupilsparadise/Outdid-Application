#include <string.h>
#include "OtdCircularBuffer_App.h"
#include "OtdUART.h"

extern uint8_t gsm_tx_pending;//flag to confirm all bytes have transmitted
extern uint8_t debug_tx_pending;
extern uint8_t gsm_rx_data;

/*Only RX is recieved in circular buffer*/
OtdCircularBufferApp gsm_rx = {{INIT_VAL},INIT_VAL,INIT_VAL};

/*pointer for rx circular buffer for debug port*/
OtdCircularBufferApp *gsm_rx_ptr;

void OtdCircularBufferApp_Init(void)
{
	/*Initial buffers*/
	gsm_rx_ptr = &gsm_rx;
	/*initialize UART1*/
	OtdUart_Init();
}

void OtdCircularBufferApp_BuffStoreChar(unsigned char c, OtdCircularBufferApp *buffer)
{
	uint16_t loc = (buffer->head + 1) % GSM_UART_BUFFER_SIZE;
	
	/*check if no overflow will occur*/
	if(loc != buffer->tail)
	{
		buffer->buffer[buffer->head] = c;
		/*update head*/
		buffer->head = loc;
	}	
}

/*find a substring within a string*/
int8_t OtdCircularBufferApp_FindStr(char *substring, char *mainstring)
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
		return OtdCircularBufferApp_Pass;
	}
	else
	{
		return OtdCircularBufferApp_Failed;
	}
	
}

uint16_t OtdCircularBufferApp_BufferClear(uint8_t uart)
{
	if(uart == GSM_UART)
	{
		/*set buffer content to '\0'*/
		memset(gsm_rx_ptr->buffer, '\0', GSM_UART_BUFFER_SIZE);
		gsm_rx_ptr->head = 0;
	}
	else
	{
		return INVALID_UART;
	}
}

uint16_t OtdCircularBufferApp_BufferPeek(unsigned char *c,uint8_t uart)
{
	if(uart == GSM_UART)
	{
		if(gsm_rx_ptr->head == gsm_rx_ptr->tail)
		{
			return OtdCircularBufferApp_Failed;
		}
		else
		{
			*c = gsm_rx_ptr->buffer[gsm_rx_ptr->tail];
			return OtdCircularBufferApp_Pass;
		}
	}
	else
	{
		return INVALID_UART;
	}
}

static uint16_t OtdCircularBufferApp_BufferRead(unsigned char *c,uint8_t uart)
{
	if(uart == GSM_UART)
	{
		if(gsm_rx_ptr->head == gsm_rx_ptr->tail)
		{
			return OtdCircularBufferApp_Failed;
		}
		else
		{
			*c = gsm_rx_ptr->buffer[gsm_rx_ptr->tail];
			gsm_rx_ptr->tail = (uint16_t)(gsm_rx_ptr->tail + 1) % GSM_UART_BUFFER_SIZE;
			return OtdCircularBufferApp_Pass;
		}
	}
	else
	{
		return INVALID_UART;
	}
}
 
void OtdCircularBufferApp_BufferSendString(const char *s, uint8_t uart)
{
	/*if(uart == GSM_UART)
	{
		while(*s != '\0')
		{
			OtdUart_Send((uint8_t *__near)s++, 1,uart);
			while(!gsm_tx_pending);
			gsm_tx_pending = 0;
		}

	}
	if(uart == DEBUG_UART)
	{
		while(*s != '\0')
		{
			OtdUart_Send((uint8_t *__near)s++, 1,uart);
			while(!debug_tx_pending);
			debug_tx_pending = 0;
		}
	}*/
}

uint16_t OtdCircularBufferApp_IsData(uint8_t uart)
{
	if(uart == GSM_UART)
	{
		return (uint16_t)(GSM_UART_BUFFER_SIZE + gsm_rx_ptr->head - gsm_rx_ptr->tail) % GSM_UART_BUFFER_SIZE;
	}
	else
	{
		return INVALID_UART;
	}	
}

void gsm_rx_uart_callback(void)
{
	unsigned char c = gsm_rx_data;
	OtdCircularBufferApp_BuffStoreChar(c, gsm_rx_ptr);
	//OtdUart_Recieve(&gsm_rx_data,1);
}

/*Function to get the position for the first character of a string in buffer*/
void OtdCircularBufferApp_GetFirstChar(char *str)
{
	volatile unsigned char c;
	volatile OtdCircularBufferApp_ErrorState status_err = OtdCircularBufferApp_Failed;
	/*make sure there is data in the buffer*/
	//while(!OtdCircularBufferApp_IsData(GSM_UART)){} //this will be checked before calling 
	
	//while(OtdCircularBufferApp_BufferPeek(GSM_UART)!= str[0])
	//if(OtdCircularBufferApp_BufferPeek(GSM_UART)!= str[0])
	status_err = OtdCircularBufferApp_BufferPeek(&c,GSM_UART);
	if((c != str[0]) && (status_err == OtdCircularBufferApp_Pass))
	{
		gsm_rx_ptr->tail = (uint16_t)(gsm_rx_ptr->tail + 1) % GSM_UART_BUFFER_SIZE;
		//while(!OtdCircularBufferApp_IsData(GSM_UART)){}
	}
}
volatile OtdCircularBufferApp_ErrorState status;
OtdCircularBufferApp_ErrorState OtdCircularBufferApp_IsResponse(char *str)
{
	uint16_t curr_pos = 0;
	uint16_t len = strlen(str);
	volatile unsigned char c = 0;
	status = OtdCircularBufferApp_Pass;

	

	while((curr_pos != len) && (status == OtdCircularBufferApp_Pass))
	{
		curr_pos = 0;
		OtdCircularBufferApp_GetFirstChar(str);
		
		status = OtdCircularBufferApp_BufferPeek(&c,GSM_UART);
		
		if(status == OtdCircularBufferApp_Pass)
		{
			//while((OtdCircularBufferApp_BufferPeek(GSM_UART) == str[curr_pos]) && (status == OtdCircularBufferApp_Pass))
			while((c == str[curr_pos]) && (status == OtdCircularBufferApp_Pass))
			{
				curr_pos++;

				status = OtdCircularBufferApp_BufferRead(&c,GSM_UART);

				if(curr_pos == len)
				{
					return OtdCircularBufferApp_Pass;
				}

				//while(!OtdCircularBufferApp_IsData(GSM_UART)){}
				status = OtdCircularBufferApp_BufferPeek(&c,GSM_UART);
			}		
		}

		
	}
	if(curr_pos == len)
	{
		return OtdCircularBufferApp_Pass;
	}
	else
	{
		return OtdCircularBufferApp_Failed;
	}
}
