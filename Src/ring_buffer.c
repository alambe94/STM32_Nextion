/*
 * ring_buffer.c
 *
 *  Created on: 01-Aug-2018
 *      Author: medprime
 */



/** Implementation of read only ring buffer for uart
 *
 * data is written to buffer via uart DMA in background
 *
 *
 */


#include "ring_buffer.h"

#define BUFFER_SIZE       128  /* must be power of two */

static uint8_t RX_DMA_Buffer[BUFFER_SIZE]={0};
static UART_HandleTypeDef *huart;
static uint32_t Read_PTR;

#define WRITE_PTR ( BUFFER_SIZE - (huart->hdmarx->Instance->CNDTR))

void Ring_Buffer_Init(UART_HandleTypeDef *_huart)
{
	huart = _huart;
	huart->hdmarx->Instance->CNDTR = 0;
	Read_PTR = 0;
	HAL_UART_Receive_DMA(huart, RX_DMA_Buffer, BUFFER_SIZE);

}

uint8_t Ring_Buffer_Is_Full(void)
{

	return (Read_PTR == WRITE_PTR)?1:0;
}

uint8_t Ring_Buffer_Get_Char(void)
{
	if (WRITE_PTR == Read_PTR)
	{
		return 0x00;
	}
	else
	{
		uint8_t data = RX_DMA_Buffer[Read_PTR];

		Read_PTR++;

		if (Read_PTR == BUFFER_SIZE)
		{
			Read_PTR = 0;
		}

		return data;
	}
}

uint8_t Ring_Buffer_Get_Count(void)
{
	if (WRITE_PTR >= Read_PTR)
	{
		return (WRITE_PTR - Read_PTR);
	}
	return (BUFFER_SIZE - (Read_PTR - WRITE_PTR));
}


void Ring_Buffer_Flush()
{
	Read_PTR = WRITE_PTR;
}


