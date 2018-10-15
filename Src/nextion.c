/*
 * nextion.c
 *
 *  Created on: 02-Aug-2018
 *      Author: medprime
 */

#include "nextion.h"

#define MAX_NEXTION_OBJECTS      (50)

#define NEXTION_LOOP_SCAN_TICK   (10) //ms dealay after first char is receiver

#define NEX_RET_CMD_FINISHED            (0x01)
#define NEX_RET_EVENT_LAUNCHED          (0x88)
#define NEX_RET_EVENT_UPGRADED          (0x89)
#define NEX_RET_EVENT_TOUCH_HEAD            (0x65)
#define NEX_RET_EVENT_POSITION_HEAD         (0x67)
#define NEX_RET_EVENT_SLEEP_POSITION_HEAD   (0x68)
#define NEX_RET_CURRENT_PAGE_ID_HEAD        (0x66)
#define NEX_RET_STRING_HEAD                 (0x70)
#define NEX_RET_NUMBER_HEAD                 (0x71)
#define NEX_RET_INVALID_CMD             (0x00)
#define NEX_RET_INVALID_COMPONENT_ID    (0x02)
#define NEX_RET_INVALID_PAGE_ID         (0x03)
#define NEX_RET_INVALID_PICTURE_ID      (0x04)
#define NEX_RET_INVALID_FONT_ID         (0x05)
#define NEX_RET_INVALID_BAUD            (0x11)
#define NEX_RET_INVALID_VARIABLE        (0x1A)
#define NEX_RET_INVALID_OPERATION       (0x1B)

#define NEX_EVENT_PUSH  (0x01)
#define NEX_EVENT_POP   (0x00)

#define false       (0)
#define true        (1)

static Nextion_Object_t* Nextion_Object_PTR_Array[MAX_NEXTION_OBJECTS];

static uint16_t Nextion_Object_Count = 0;

void Nextion_Add_Object(Nextion_Object_t* PTR) {

	Nextion_Object_PTR_Array[Nextion_Object_Count] = PTR;
	Nextion_Object_Count++;

}

uint8_t Nextion_Init() {

	uint8_t ret1 = NEXTION_ERR;
	uint8_t ret2 = NEXTION_ERR;

	/*Init Serial Port */

	/***** Cube ********/

	Ring_Buffer_Init(&huart1);

	Nextion_Send_Command("");

	Nextion_Send_Command("bkcmd=1");
	ret1 = Nextion_Command_Finished();
	Nextion_Send_Command("page 0");
	ret2 = Nextion_Command_Finished();
	return (ret1 && ret2) ? NEXTION_OK : NEXTION_ERR;
}

/*
 * Send command to Nextion.
 *
 * @param cmd - the string of command.
 */
void Nextion_Send_Command(const char* cmd) {

	char buf[30] = { 0 };
	char sps = 0xFF;

	sprintf(buf, "%s%c%c%c", cmd, sps, sps, sps);

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);

}

/*
 * Command is executed successfully.
 *
 * @param timeout - set timeout time.
 *
 * @retval true - success.
 * @retval false - failed.
 *
 */
uint8_t Nextion_Command_Finished() {
	uint32_t timeout = 0xFFFFF;

	uint8_t ret = NEXTION_ERR;

	uint8_t temp[4] = { 0 };

	while (Ring_Buffer_Get_Count() < 4 && --timeout) {
	}

	if (timeout > 0) {
		for (uint8_t i = 0; i < 4; i++) {
		    Ring_Buffer_Get_Char(&temp[i]);
		}

		if (temp[0] == NEX_RET_CMD_FINISHED && temp[1] == 0xFF
				&& temp[2] == 0xFF && temp[3] == 0xFF) {
			ret = NEXTION_OK;
		}
	}

	if (ret == NEXTION_OK) {
		//dbSerialPrintln("recvRetCommandFinished ok");
	} else {
		//dbSerialPrintln("recvRetCommandFinished err");
	}

	return ret;
}

/*
 * Receive uint32_t data.
 *
 * @param number - save uint32_t data.
 * @param timeout - set timeout time.
 *
 * @retval true - success.
 * @retval false - failed.
 *
 */
uint8_t Nextion_Receive_Number(uint32_t *number, uint32_t timeout) {
	uint8_t ret = NEXTION_ERR;
	uint8_t temp[8] = { 0 };

	if (!number) {
		return ret;
	}

	while (Ring_Buffer_Get_Count() < 8 && --timeout) {
	}

	if (timeout > 0) {

		for (uint8_t i = 0; i < 8; i++) {
		    Ring_Buffer_Get_Char(&temp[i]);
		}

		if (temp[0] == NEX_RET_NUMBER_HEAD && temp[5] == 0xFF && temp[6] == 0xFF
				&& temp[7] == 0xFF) {
			*number = ((uint32_t) temp[4] << 24) | ((uint32_t) temp[3] << 16)
					| (temp[2] << 8) | (temp[1]);

			ret = NEXTION_OK;
		}
	}

	return ret;
}

/*
 * Receive string data.
 *
 * @param buffer - save string data.
 * @param len - string buffer length.
 * @param timeout - set timeout time.
 *
 * @return the length of string buffer.
 *
 */
uint16_t Nextion_Receive_String(char *buffer, uint16_t len, uint32_t timeout) {
	uint16_t ret = 0;
	uint8_t str_start_flag = false;
	uint8_t cnt_0xff = 0;
	char* temp = "";
	uint8_t c = 0;
	uint32_t start;

	if (!buffer || len == 0) {
		goto __return;
	}

	start = HAL_GetTick();

	while (HAL_GetTick() - start <= timeout) {
		while (Ring_Buffer_Get_Count() > 0) {
			Ring_Buffer_Get_Char(&c);
			if (str_start_flag) {
				if (0xFF == c) {
					cnt_0xff++;
					if (cnt_0xff >= 3) {
						break;
					}
				} else {
					*temp = c;
					temp++;
				}
			} else if (NEX_RET_STRING_HEAD == c) {
				str_start_flag = true;
			}
		}

		if (cnt_0xff >= 3) {
			break;
		}
	}

	*temp = '\0';
	ret = strlen(temp);
	ret = ret > len ? len : ret;
	strncpy(buffer, temp, ret);

	__return:

	//dbSerialPrint("recvRetString[");
	//dbSerialPrint(temp.length());
	//dbSerialPrint(",");
	//dbSerialPrint(temp);
	//dbSerialPrintln("]");

	return ret;
}

void Nextion_Find_Object(uint8_t pid, uint8_t cid, uint8_t event) {
	Nextion_Object_t *e = NULL;

	uint16_t i = 0;

	for (i = 0; i < Nextion_Object_Count; i++) {
		e = Nextion_Object_PTR_Array[i];

		if (e->page_id == pid && e->component_id == cid) {
			if (NEX_EVENT_PUSH == event) {
				if (e->Push_Callback_PTR != NULL) {
					e->Push_Callback_PTR();
				}
			} else if (NEX_EVENT_POP == event) {
				if (e->Pop_Callback_PTR != NULL) {
					e->Pop_Callback_PTR();
				}
			}

			break;
		}
	}
}

/*********** called when Nextion send page id ***********
 *
 *
 *  defined externally
 */
extern void Nextion_Received_Page_ID(uint8_t Page_ID);
extern void Nextion_Received_String();
extern void Nextion_Received_Number(uint32_t Number);

void Nextion_Loop() {

	static uint32_t Nextion_Loop_Time_Stamp = 0;

	static uint8_t __buffer[10];

	uint16_t i;
	uint8_t c;

	static uint8_t delay_flag = 0;

	if (Ring_Buffer_Get_Count() > 0 && delay_flag == RESET) {
		delay_flag = SET;
		Nextion_Loop_Time_Stamp = HAL_GetTick();
	}
	if (HAL_GetTick() - Nextion_Loop_Time_Stamp > (NEXTION_LOOP_SCAN_TICK)
			&& delay_flag == SET) // excute this loop after NEXTION_LOOP_SCAN_TICK when first char is detected
					{

		delay_flag = RESET;  //delay complete after detecting first char

		while (Ring_Buffer_Get_Count() > 0) {
			//HAL_Delay(10);
			Ring_Buffer_Get_Char(&c);

			if (NEX_RET_EVENT_TOUCH_HEAD == c) {
				if (Ring_Buffer_Get_Count() >= 6) {
					__buffer[0] = c;
					for (i = 1; i < 7; i++) {
					Ring_Buffer_Get_Char(&__buffer[i]);
					}
					__buffer[i] = 0x00;

					if (0xFF == __buffer[4] && 0xFF == __buffer[5]
							&& 0xFF == __buffer[6]) {
						Nextion_Find_Object(__buffer[1], __buffer[2],
								__buffer[3]);
					}

				}
			}

			if (NEX_RET_CURRENT_PAGE_ID_HEAD == c) {
				if (Ring_Buffer_Get_Count() >= 4) {

					for (uint8_t i = 1; i < 5; i++) {
					    Ring_Buffer_Get_Char(&__buffer[i]);
					}

					if (__buffer[2] == 0xFF && __buffer[3] == 0xFF
							&& __buffer[4] == 0xFF) {

						Nextion_Received_Page_ID(__buffer[1]);
					}

				}
			}

			if (NEX_RET_STRING_HEAD == c) {
				uint8_t cnt_0xff = 0;
				uint8_t i = 0;
				while (Ring_Buffer_Get_Count() > 0) {
					Ring_Buffer_Get_Char(&c);
					if (0xFF == c) {
						cnt_0xff++;
						if (cnt_0xff >= 3) {
							break;
						}
					} else {
						Nextion_Received_Array[i] = c;
						i++;
						if (i == NEXTION_RECEIVED_STRING_SIZE-1) {
							break;
						}
					}
				}
				Nextion_Received_Array[i]='\0';
				Nextion_Received_String();
			}

			if (NEX_RET_NUMBER_HEAD == c) {

				uint32_t number = 0;
				for (uint8_t i = 1; i < 8; i++) {
					Ring_Buffer_Get_Char(&__buffer[i]);
				}

				if (__buffer[5] == 0xFF && __buffer[6] == 0xFF
						&& __buffer[7] == 0xFF) {
					number = ((uint32_t) __buffer[4] << 24)
							| ((uint32_t) __buffer[3] << 16)
							| (__buffer[2] << 8) | (__buffer[1]);

					Nextion_Received_Number(number);
				}

			}

		}
	}

}

/*********************
 * set value of object with .val property
 *
 *
 */

void Nextion_Set_Value(char *object_name, int16_t number) {
	char buf[30] = { 0 };

	char sps = 0xFF;

	sprintf(buf, "%s.val=%i%c%c%c", object_name, number, sps, sps, sps);

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);
}

/*********************
 * set value of object with .txt property
 *
 *
 */
void Nextion_Set_Value_Float(char *object_name, float number) {
	char buf[30] = { 0 };

	char sps = 0xFF;

	sprintf(buf, "%s.txt=\"%.2f\"%c%c%c", object_name, number, sps, sps, sps);

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);
}

/*********************
 * set text of object with .txt property
 *
 *
 */

void Nextion_Set_Text(char *object_name, char *buffer) {

	char buf[20] = { 0 };

	char sps = 0xFF;

	sprintf(buf, "%s.txt=\"%s\"%c%c%c", object_name, buffer, sps, sps, sps);

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);
}

/*********************
 * set background colour of object with .bco property
 *
 *
 */

void Nextion_Set_BCK_Colour(char *object_name, int colour) {

	char buf[20] = { 0 };

	char sps = 0xFF;

	sprintf(buf, "%s.bco=%i%c%c%c", object_name, colour, sps, sps, sps);

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);

	sprintf(buf, "ref %s%c%c%c", object_name, sps, sps, sps);

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);
}

/*********************
 * hide/show object
 *
 *
 */

void Nextion_Hide_Object(char *object_name, uint8_t hide) {

	char buf[20] = { 0 };

	char sps = 0xFF;

	if (hide > 0) {
		sprintf(buf, "vis %s,0%c%c%c", object_name, sps, sps, sps);
	} else {
		sprintf(buf, "vis %s,1%c%c%c", object_name, sps, sps, sps);
	}

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);
}


/*********************
 * set brightness
 * override -> true   save changes permanently
 * overide -> false   reset to default after power cycle
 */

void Nextion_Backlight_Brightness(uint8_t value, uint8_t overide) {

	char buf[10] = { 0 };

	char sps = 0xFF;

	if (overide > 0) {
		sprintf(buf, "dims=%i%c%c%c", value, sps, sps, sps);
	} else {
		sprintf(buf, "dim=%i%c%c%c", value, sps, sps, sps);
	}

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);
}

/*********************
 * set baud rate
 * override -> true   save changes permanently
 * overide -> false   reset to default after power cycle
 */
void Nextion_Baud_Rate(uint32_t baud, uint8_t overide) {

	char buf[10] = { 0 };

	char sps = 0xFF;

	if (overide > 0) {
		sprintf(buf, "bauds=%i%c%c%c", (int) baud, sps, sps, sps);
	} else {
		sprintf(buf, "baud=%i%c%c%c", (int) baud, sps, sps, sps);
	}

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);
}



void Nextion_Get_Text(char *object_name)
{

	char sps = 0xFF;
	char buf[20] = { 0 };

	sprintf(buf, "get %s.txt%c%c%c", object_name,sps, sps, sps);

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);
}



/*********************
 * get current page
 *
 *
 */

uint8_t Nextion_Get_Current_Page() {

	char buf[10] = { 0 };

	char sps = 0xFF;

	sprintf(buf, "sendme%c%c%c", sps, sps, sps);

	HAL_UART_Transmit(&huart1, (uint8_t*) &buf, strlen(buf), 50);

	return 0;
}
