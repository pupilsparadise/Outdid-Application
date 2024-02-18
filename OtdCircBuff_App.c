#include <string.h>
#include "OtdCircBuff_App.h"
#include "OtdUART.h"


/*Only RX is recieved in circular buffer*/
OtdCircBuffApp_CircBuff_tst GsmRx_CircBuff = {{INIT_VAL},INIT_VAL,INIT_VAL};

/*pointer for rx circular buffer for Gsm Uart*/
OtdCircBuffApp_CircBuff_tst *GsmRx_ptr;

/*Push Rx data to circular buffer*/
static void OtdCircBuffApp_BufferPushChar(unsigned char c, OtdCircBuffApp_CircBuff_tst *buffer)
{
	uint16_t loc = (buffer->head + 1) % RX_BUFFER_LENGTH;
	
	/*check if no overflow will occur*/
	if(loc != buffer->tail)
	{
		buffer->buffer[buffer->head] = c;
		/*update head*/
		buffer->head = loc;
	}	
}
/*Pop/readback the data recieved in circular buffer*/
OtdCircBuffApp_ErrorState_ten OtdCircBuffApp_BufferPop(unsigned char *c)
{

		if(GsmRx_ptr->head == GsmRx_ptr->tail)
		{
			return OtdCircBuffApp_Empty;
		}
		else
		{
			*c = GsmRx_ptr->buffer[GsmRx_ptr->tail];
			GsmRx_ptr->tail = (uint16_t)(GsmRx_ptr->tail + 1) % RX_BUFFER_LENGTH;
			return OtdCircBuffApp_Ok;
		}
}
static OtdCircBuffApp_ErrorState_ten OtdCircBuffApp_BufferPeek(unsigned char *c,uint8_t uart)
{
	if(GsmRx_ptr->head == GsmRx_ptr->tail)
	{
		return OtdCircBuffApp_Empty;
	}
	else
	{
		*c = GsmRx_ptr->buffer[GsmRx_ptr->tail];
		return OtdCircBuffApp_Ok;
	}
}


/*check if data is available in buffer*/
uint16_t OtdCircBuffApp_IsData(void)
{
	return (uint16_t)(RX_BUFFER_LENGTH + GsmRx_ptr->head - GsmRx_ptr->tail) % RX_BUFFER_LENGTH;
}

/*clear the buffer*/
static void OtdCircBuffApp_BufferClear(void)
{
	/*set buffer content to '\0'*/
	memset(GsmRx_ptr->buffer, '\0', RX_BUFFER_LENGTH);
	GsmRx_ptr->head = 0;
}

/*Initialize the Circular Buffer*/
void OtdCircBuffApp_Init(void)
{
	/*Initial buffers*/
	GsmRx_ptr = &GsmRx_CircBuff;
	/*initialize UART1*/
	OtdUart_Init();
}

void OtdCircBuffApp_CircBuffPush(volatile unsigned char *data)
{
	OtdCircBuffApp_BufferPushChar(*data,GsmRx_ptr);
}