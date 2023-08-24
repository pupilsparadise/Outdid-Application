#include <stdint.h>

#define UART_BUFFER_SIZE 	512
#define INIT_VAL		0

typedef uint8_t portType;

typedef struct
{
	unsigned char buffer[UART_BUFFER_SIZE];
	volatile uint16_t head;
	volatile uint16_t tail;
}Circular_Buffer;

void CircularBuffer_init(void);

int8_t find_str(char *substring, char *mainstring);

void buffer_send_string(const char *s, portType uart);

void buff_store_char(unsigned char c, Circular_Buffer *buffer);

int8_t is_response(char *str);
