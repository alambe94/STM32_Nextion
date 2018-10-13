/*
 * ring_buffer.h
 *
 *  Created on: 01-Aug-2018
 *      Author: medprime
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include "main.h"
#include "stm32f4xx_hal.h"


void Ring_Buffer_Init(UART_HandleTypeDef *_huart);
uint8_t Ring_Buffer_Get_Char(void);
uint8_t Ring_Buffer_Get_Count(void);
void Ring_Buffer_Flush();

uint8_t Ring_Buffer_Check_Char(void);



#endif /* RING_BUFFER_H_ */
