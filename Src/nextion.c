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

static Nextion_Object_t* Nextion_Object_List[MAX_NEXTION_OBJECTS];

static uint16_t Nextion_Object_Count = 0;


#define  NEXTION_IN_BUFF_SIZE 20
char     Nextion_In_Buffer[NEXTION_IN_BUFF_SIZE];

volatile uint8_t  Idle_Interrupt_Flag = 0;
volatile uint8_t  CMD_Finished_Flag = 0;

/*********************ring buffer stuff start*******************/
#define       UART_RING_BUFFER_SIZE   128
static char   UART_DMA_RX_Buffer[UART_RING_BUFFER_SIZE];
static Ring_Buffer_t UART_Ring_Buffer_Handle;

extern UART_HandleTypeDef huart1;
UART_HandleTypeDef* Nextion_UART = &huart1;

/*data is written to buffer via uart DMA in background*/
/* need to update Write_Index manually */
#define     UPDATE_RING_BUFFER() (UART_Ring_Buffer_Handle.Write_Index = ( UART_RING_BUFFER_SIZE - (Nextion_UART->hdmarx->Instance->CNDTR)))

/*********************ring buffer stuff stop*******************/

uint8_t Nextion_Add_Object(Nextion_Object_t* PTR)
    {
    if (Nextion_Object_Count < MAX_NEXTION_OBJECTS)
	{
	Nextion_Object_List[Nextion_Object_Count] = PTR;
	Nextion_Object_Count++;
	return 1;
	}
    return 0;
    }

uint8_t Nextion_Wait_CMD()
    {
    uint32_t timeout = 0xFFFF;
    CMD_Finished_Flag = 0;
    uint8_t xreturn = NEXTION_ERR;

    while (!Idle_Interrupt_Flag && --timeout)
	{
	}
    if (timeout)
	{

	Nextion_Loop();

	if (CMD_Finished_Flag)
	    {
	    CMD_Finished_Flag = 0;
	    xreturn = NEXTION_OK;
	    }
	else
	    {
	    xreturn = NEXTION_ERR;
	    }
	}

    return xreturn;

    }

uint8_t Nextion_Init()
    {



    /*Init Serial Port */

    /***** Cube ********/

    Ring_Buffer_Init(&UART_Ring_Buffer_Handle, UART_DMA_RX_Buffer, UART_RING_BUFFER_SIZE);

    HAL_UART_Receive_DMA(Nextion_UART, (uint8_t*) UART_DMA_RX_Buffer, UART_RING_BUFFER_SIZE);

    __HAL_UART_ENABLE_IT(Nextion_UART, UART_IT_IDLE);

    Nextion_Send_Command("");
    HAL_Delay(1);

    CMD_Finished_Flag = 0;
    Nextion_Send_Command("bkcmd=1");

    if (Nextion_Wait_CMD())
	{

	Nextion_Send_Command("page 0");

	if (Nextion_Wait_CMD())
	    {
	    return NEXTION_OK;
	    }
	}

    return NEXTION_ERR;

    }

/*
 * Send command to Nextion.
 *
 * @param cmd - the string of command.
 */
void Nextion_Send_Command(const char* cmd)
    {

    char buf[30] =
	{
	0
	};
    char sps = 0xFF;

    sprintf(buf, "%s%c%c%c", cmd, sps, sps, sps);

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);

    }

void Nextion_Find_Object(uint8_t pid, uint8_t cid, uint8_t event)
    {
    Nextion_Object_t *Nextion_Object_t_PTR = NULL;

    uint16_t i = 0;

    for (i = 0; i < Nextion_Object_Count; i++)
	{

	Nextion_Object_t_PTR = Nextion_Object_List[i];

	if (Nextion_Object_t_PTR->Page_ID == pid && Nextion_Object_t_PTR->Component_ID == cid)
	    {
	    if (NEX_EVENT_PUSH == event)
		{
		if (Nextion_Object_t_PTR->Push_Callback != NULL)
		    {
		    Nextion_Object_t_PTR->Push_Callback();
		    }
		}
	    else if (NEX_EVENT_POP == event)
		{
		if (Nextion_Object_t_PTR->Pop_Callback != NULL)
		    {
		    Nextion_Object_t_PTR->Pop_Callback();
		    }
		}

	    break;//object found break the loop
	    }
	}
    }

/*********** called when Nextion send page id ***********
 *
 *
 *  defined externally
 */
extern void Nextion_RX_Page_ID_Callback(uint8_t Page_ID);



/*********** called when Nextion send string ***********
 *
 *
 *  defined externally
 */
extern void Nextion_RX_String_Callback(const char* str);



/*********** called when Nextion send number ***********
 *
 *
 *  defined externally
 */
extern void Nextion_RX_Number_Callback(uint32_t Number);

/*********** called when Command Exe is finished ***********
 *
 *
 *  defined externally
 */
extern void Nextion_CMD_Finished_Callback();



void Nextion_UART_RX_ISR()
    {

    if (__HAL_UART_GET_FLAG(Nextion_UART, UART_FLAG_IDLE))
	{

	Idle_Interrupt_Flag = 1;

	/*clear idle interrupt*/
	Nextion_UART->Instance->SR;
	Nextion_UART->Instance->DR;

	}
    }



void Nextion_Loop()
    {

    static uint8_t rx_char_count = 0;
    char rx_char = 0;
    uint8_t data_received = 0;
    uint8_t rx_number = 0;

    if (Idle_Interrupt_Flag)
	{

	Idle_Interrupt_Flag = 0;

	/*data is written to buffer via uart DMA in background*/
	/* need to update Write_Index manually */
	UPDATE_RING_BUFFER();

	while (Ring_Buffer_Get_Count(&UART_Ring_Buffer_Handle))
	    {

	    Ring_Buffer_Get_Char(&UART_Ring_Buffer_Handle, &rx_char);

	    if (rx_char == 0xFF)
		{
		Ring_Buffer_Get_Char(&UART_Ring_Buffer_Handle, &rx_char);
		    {
		    if (rx_char == 0xFF)
			{
			Ring_Buffer_Get_Char(&UART_Ring_Buffer_Handle,
				&rx_char);
			if (rx_char == 0xFF)
			    {
			    rx_char_count = 0;
			    data_received = 1;
			    }
			}
		    }
		}
	    else
		{
		Nextion_In_Buffer[rx_char_count] = rx_char;
		rx_char_count++;
		}

	    if (data_received)
		{

		data_received = 0;

		switch (Nextion_In_Buffer[0])
		    {
		case NEX_RET_EVENT_TOUCH_HEAD:
		    Nextion_Find_Object(Nextion_In_Buffer[1],
			    Nextion_In_Buffer[2], Nextion_In_Buffer[3]);
		    break;

		case NEX_RET_CURRENT_PAGE_ID_HEAD:
		    Nextion_RX_Page_ID_Callback(Nextion_In_Buffer[1]);
		    break;

		case NEX_RET_STRING_HEAD:
		    Nextion_RX_String_Callback(&Nextion_In_Buffer[1]);
		    break;

		case NEX_RET_NUMBER_HEAD:
		    rx_number = (Nextion_In_Buffer[4] << 24)
			    | (Nextion_In_Buffer[3] << 16)
			    | (Nextion_In_Buffer[2] << 8)
			    | (Nextion_In_Buffer[1] << 0);
		    Nextion_RX_Number_Callback(rx_number);
		    break;

		case NEX_RET_CMD_FINISHED:
		    CMD_Finished_Flag = 1;
		    Nextion_CMD_Finished_Callback();
		    break;
		default:
		    break;

		    }
		memset(Nextion_In_Buffer, 0x00, NEXTION_IN_BUFF_SIZE); //reset buffer
		}
	    }
	}
    }

/*********************
 * set value of object with .val property
 *
 *
 */

void Nextion_Set_Value(char *object_name, int16_t number)
    {
    char buf[30] =
	{
	0
	};

    char sps = 0xFF;

    sprintf(buf, "%s.val=%i%c%c%c", object_name, number, sps, sps, sps);

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);
    }

/*********************
 * set value of object with .txt property
 *
 *
 */
void Nextion_Set_Value_Float(char *object_name, float number)
    {
    char buf[30] =
	{
	0
	};

    char sps = 0xFF;

    sprintf(buf, "%s.txt=\"%.2f\"%c%c%c", object_name, number, sps, sps, sps);

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);
    }

/*********************
 * set text of object with .txt property
 *
 *
 */

void Nextion_Set_Text(char *object_name, char *buffer)
    {

    char buf[20] =
	{
	0
	};

    char sps = 0xFF;

    sprintf(buf, "%s.txt=\"%s\"%c%c%c", object_name, buffer, sps, sps, sps);

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);
    }

/*********************
 * set background colour of object with .bco property
 *
 *
 */

void Nextion_Set_BCK_Colour(char *object_name, int colour)
    {

    char buf[20] =
	{
	0
	};

    char sps = 0xFF;

    sprintf(buf, "%s.bco=%i%c%c%c", object_name, colour, sps, sps, sps);

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);

    sprintf(buf, "ref %s%c%c%c", object_name, sps, sps, sps);

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);
    }

/*********************
 * hide/show object
 *
 *
 */

void Nextion_Hide_Object(char *object_name, uint8_t hide)
    {

    char buf[20] =
	{
	0
	};

    char sps = 0xFF;

    if (hide > 0)
	{
	sprintf(buf, "vis %s,0%c%c%c", object_name, sps, sps, sps);
	}
    else
	{
	sprintf(buf, "vis %s,1%c%c%c", object_name, sps, sps, sps);
	}

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);
    }

/*********************
 * set brightness
 * override -> true   save changes permanently
 * overide -> false   reset to default after power cycle
 */

void Nextion_Backlight_Brightness(uint8_t value, uint8_t overide)
    {

    char buf[10] =
	{
	0
	};

    char sps = 0xFF;

    if (overide > 0)
	{
	sprintf(buf, "dims=%i%c%c%c", value, sps, sps, sps);
	}
    else
	{
	sprintf(buf, "dim=%i%c%c%c", value, sps, sps, sps);
	}

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);
    }

/*********************
 * set baud rate
 * override -> true   save changes permanently
 * overide -> false   reset to default after power cycle
 */
void Nextion_Baud_Rate(uint32_t baud, uint8_t overide)
    {

    char buf[20] =
	{
	0
	};

    char sps = 0xFF;

    if (overide > 0)
	{
	sprintf(buf, "bauds=%i%c%c%c", (int) baud, sps, sps, sps);
	}
    else
	{
	sprintf(buf, "baud=%i%c%c%c", (int) baud, sps, sps, sps);
	}

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);
    }

void Nextion_Get_Text(char *object_name)
    {

    char sps = 0xFF;
    char buf[20] =
	{
	0
	};

    sprintf(buf, "get %s.txt%c%c%c", object_name, sps, sps, sps);

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);
    }

/*********************
 * get current page
 *
 *
 */

void Nextion_Get_Current_Page()
    {

    char buf[10] =
	{
	0
	};

    char sps = 0xFF;

    sprintf(buf, "sendme%c%c%c", sps, sps, sps);

    HAL_UART_Transmit(Nextion_UART, (uint8_t*) &buf, strlen(buf), 50);

    }
