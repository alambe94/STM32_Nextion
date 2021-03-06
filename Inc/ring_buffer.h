#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include "main.h"
#include "stm32f1xx_hal.h"

typedef struct Ring_Buffer_t
{
	char *Buffer;
	uint16_t Write_Index;
	uint16_t Read_Index;
	uint16_t Size;
} Ring_Buffer_t;

void Ring_Buffer_Init(Ring_Buffer_t *handle, char *buffer, uint16_t size);
uint8_t Ring_Buffer_Put_Char(Ring_Buffer_t *handle, char data);
uint8_t Ring_Buffer_Get_Char(Ring_Buffer_t *handle, char *data);
uint8_t Ring_Buffer_Peek_Char(Ring_Buffer_t *handle, char *data, uint16_t position);
uint8_t Ring_Buffer_Is_Full(Ring_Buffer_t *handle);
void Ring_Buffer_Flush(Ring_Buffer_t *handle);
uint8_t Ring_Buffer_Get_Count(Ring_Buffer_t *handle);

#endif /* RING_BUFFER_H_ */
