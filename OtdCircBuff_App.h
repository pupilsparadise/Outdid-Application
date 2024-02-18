#include <stdint.h>
#include "OtdGsm_App.h"

#define INIT_VAL		0

typedef enum
{
	OtdCircBuffApp_Empty,
	OtdCircBuffApp_Full,
	OtdCircBuffApp_Ok,
	OtdCircBuffApp_Nok
}OtdCircBuffApp_ErrorState_ten;

typedef struct
{
	unsigned char buffer[RX_BUFFER_LENGTH];
	volatile uint16_t head;
	volatile uint16_t tail;
}OtdCircBuffApp_CircBuff_tst;

/*Global Functions*/
void OtdCircBuffApp_Init(void);
void OtdCircBuffApp_CircBuffPush(volatile unsigned char *data);
uint16_t OtdCircBuffApp_IsData(void);
OtdCircBuffApp_ErrorState_ten OtdCircBuffApp_BufferPop(unsigned char *c);