#include <stdint.h>

#define GSM_UART_BUFFER_SIZE 	512
#define INIT_VAL		0

typedef struct
{
	unsigned char buffer[GSM_UART_BUFFER_SIZE];
	volatile uint16_t head;
	volatile uint16_t tail;
}OtdCircularBufferApp;

void OtdCircularBufferApp_Init(void);

int8_t OtdCircularBufferApp_FindStr(char *substring, char *mainstring);

void OtdCircularBufferApp_BufferSendString(const char *s, uint8_t uart);

void OtdCircularBufferApp_BuffStoreChar(unsigned char c, OtdCircularBufferApp *buffer);

int8_t OtdCircularBufferApp_IsResponse(char *str);
