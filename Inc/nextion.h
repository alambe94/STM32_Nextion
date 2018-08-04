/*
 * nextion.h
 *
 *  Created on: 02-Aug-2018
 *      Author: medprime
 */

#ifndef NEXTION_H_
#define NEXTION_H_

#include "stm32f1xx_hal.h"
#include "ring_buffer.h"
#include "string.h"

#define NEXTION_OK  1
#define NEXTION_ERR 0

#define  NEXTION_RECEIVED_STRING_SIZE 20

uint8_t Nextion_Received_Array[NEXTION_RECEIVED_STRING_SIZE];

typedef struct Nextion_Object_t
{
	uint8_t page_id;
	uint8_t component_id;
	char *name;

	void (*Push_Callback_PTR)(); //callback function ptr
	void (*Pop_Callback_PTR)();  //callback function ptr

} Nextion_Object_t;

extern UART_HandleTypeDef huart1;

uint8_t Nextion_Init();
void Nextion_Add_Object(Nextion_Object_t* PTR);
void Nextion_Send_Command(const char* cmd);
uint8_t Nextion_Command_Finished();
uint8_t Nextion_Get_Current_Page();


void Nextion_Loop();

void Nextion_Set_Text(char *object_name, char *buffer);
void Nextion_Set_Value_Float(char *object_name, float number);
void Nextion_Set_BCK_Colour(char *object_name, int colour);
void Nextion_Hide_Object(char *object_name, uint8_t hide);
void Nextion_Backlight_Brightness(uint8_t value, uint8_t overide);
void Nextion_Baud_Rate(uint32_t baud, uint8_t overide);

void Nextion_Get_Text(char *buffer);



#endif /* NEXTION_H_ */
