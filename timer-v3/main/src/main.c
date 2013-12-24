/*
 * @brief FreeRTOS Blinky example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "main.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
}

void DrawIdleScreen(void)
{
	char TimeString[11];

	//Set up idle screen
	DS3232M_GetTimeString(TimeString, 0);
	OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, TimeString, IDLE_TIME_COLUMN, IDLE_TIME_ROW, OLED_FONT_NORMAL);

	OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 1", 3, 0, OLED_FONT_NORMAL);
	OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 2", 19, 0, OLED_FONT_INVERSE);
	OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 3", 35, 0, OLED_FONT_NORMAL);
	OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 4", 51, 0, OLED_FONT_NORMAL);

	OLED_WriteLine(16*4, 0, 16*4, 16, 1, 1);
	OLED_WriteLine(32*4, 0, 32*4, 16, 1, 1);
	OLED_WriteLine(48*4, 0, 48*4, 16, 1, 1);
	OLED_WriteLine(0, 16, 255, 16, 1, 1);

	OLED_WriteMFString(MF_ASCII_SIZE_7X8, "Time:", IDLE_TIME_COLUMN-2, IDLE_TIME_ROW+18);
	OLED_WriteMFString(MF_ASCII_SIZE_7X8, "Date:", IDLE_DATE_COLUMN-2, IDLE_DATE_ROW+18);
	OLED_WriteMFString(MF_ASCII_SIZE_7X8, "Mode:", IDLE_STATUS_COLUMN-11, IDLE_STATUS_ROW);
	OLED_WriteMFString(MF_ASCII_SIZE_7X8, "Auto  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);

	DS3232M_GetDateString(TimeString, 0);
	OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, TimeString, IDLE_DATE_COLUMN, IDLE_DATE_ROW, OLED_FONT_NORMAL);
	return;
}








/* LED0 toggle thread */
/*static void vLEDTask0(void *pvParameters) {
	bool LedState = false;
	while (1) {
		Board_LED_Set(2, LedState);
		LedState = (bool) !LedState;

		vTaskDelay(configTICK_RATE_HZ/2);
	}
}*/

/* LED1 toggle thread */
static void vLEDTask1(void *pvParameters) {
	bool LedState = false;

	while (1) {
		Board_LED_Set(1, LedState);
		LedState = (bool) !LedState;
		vTaskDelay(configTICK_RATE_HZ*2);
	}
}

/* Console thread */
static void vConsoleTask(void *pvParameters)
{
	//TODO: implement a queue to receive characters
	//unsigned portBASE_TYPE uxHighWaterMark;
	uint32_t prompt = 0;
	//rdCnt = 0;
	
	//uint8_t rxdata[16];
	//uint8_t i;

	char DataFromUSB;

	while (1)
	{
		if(prompt == 1)
		{
			//The task is suspended here until data is received from USB
			xQueueReceive(xUSBCharReceived, &DataFromUSB, portMAX_DELAY);
			CommandGetInputChar(DataFromUSB);
			RunCommand();
		}


		else if ((vcom_connected() != 0) && (prompt == 0))
		{
			vcom_write((uint8_t *)"Console Active\r\n>", 17);
			prompt = 1;
		}







		/*if (prompt)
		{
			rdCnt = vcom_bread(rxdata, 16);
			if (rdCnt)
			{
				for(i=0; i<rdCnt; i++)
				{
					CommandGetInputChar((char)rxdata[i]);	//TODO: Fix this to account for more than one character input
				}
			}
			RunCommand();
		}
		vTaskDelay(10);*/
	}
}

/* OLED display thread */
static void vOLEDTask(void *pvParameters)
{
	OLED_Command CommandToExecute;
	//TimeAndDate CurrentTime;

	char TimeString[11];

	uint8_t DisplayStatus = DISPLAY_STATUS_IDLE_BRIGHT;
	portBASE_TYPE QueueStatus;

	//uint8_t TimeColumn = 4;
	//uint8_t TimeRow = 37;
	//uint8_t DateColumn = 35;
	//uint8_t DateRow = 37;
	//uint8_t StatusColumn = 13;
	//uint8_t StatusRow = 19;

	uint8_t i;
	//uint8_t CharWidth;

	static char InitString[] = "OLED Initialized";
	static char InvalidString[] = "Invalid Command";

	if(App_GetStatus() == APP_STATUS_OK)
	{
		OLED_WriteMFString(MF_ASCII_SIZE_7X8, InitString, 16, 28);
		vTaskDelay(3000);
		OLED_ClearDisplay();
	}

	//Set up idle screen
	DrawIdleScreen();

	while (1)
	{

		//Wait for a command to come in from the queue
		if(DisplayStatus == DISPLAY_STATUS_MENU)
		{
			QueueStatus = xQueueReceive(xOLEDCommands, &CommandToExecute, MENU_TO_IDLE_TIME);
		}
		else if(DisplayStatus == DISPLAY_STATUS_IDLE_BRIGHT)
		{
			QueueStatus = xQueueReceive(xOLEDCommands, &CommandToExecute, IDLE_TO_DIM_TIME);
		}
		else if(DisplayStatus == DISPLAY_STATUS_IDLE_DIM)
		{
			QueueStatus = xQueueReceive(xOLEDCommands, &CommandToExecute, portMAX_DELAY);
		}

		//If a command did not come in before the queue receive timed out, change the mode
		if(QueueStatus == pdFALSE)
		{
			if(DisplayStatus == DISPLAY_STATUS_MENU)
			{
				DrawIdleScreen();
				DisplayStatus = DISPLAY_STATUS_IDLE_BRIGHT;
			}
			else if(DisplayStatus == DISPLAY_STATUS_IDLE_BRIGHT)
			{
				OLED_DisplayContrast(0);
				DisplayStatus = DISPLAY_STATUS_IDLE_DIM;
			}
		}
		else
		{
			//Execute a command if it is received
			switch(CommandToExecute.CommandName)
			{
				case OLED_CMD_WRITE_STRING:
					OLED_WriteMFString(CommandToExecute.CommandData[2], CommandToExecute.CommandCharData, CommandToExecute.CommandData[0], CommandToExecute.CommandData[1]);
					break;

				case OLED_CMD_BUTTON_IN:
					OLED_ClearDisplay();
					DisplayStatus = DISPLAY_STATUS_MENU;
					OLED_DisplayContrast(0x7F);
					if(CommandToExecute.CommandData[0] == 4)
					{
						OLED_WriteMFString(MF_ASCII_SIZE_7X8, "center", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
					}
					else
					{
						if((OLED_GetStatus() & OLED_STATUS_ORENTATION_MASK) == OLED_STATUS_ORENTATION_DOWN)
						{
							if(CommandToExecute.CommandData[0] == 1)
							{
								OLED_WriteMFString(MF_ASCII_SIZE_7X8, "down  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
							}
							else if(CommandToExecute.CommandData[0] == 2)
							{
								OLED_WriteMFString(MF_ASCII_SIZE_7X8, "up    ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
							}
							else if(CommandToExecute.CommandData[0] == 3)
							{
								OLED_WriteMFString(MF_ASCII_SIZE_7X8, "right ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
							}
							else if(CommandToExecute.CommandData[0] == 5)
							{
								OLED_WriteMFString(MF_ASCII_SIZE_7X8, "left  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
							}
						}
						else
						{
							if(CommandToExecute.CommandData[0] == 1)
							{
								OLED_WriteMFString(MF_ASCII_SIZE_7X8, "up    ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
							}
							else if(CommandToExecute.CommandData[0] == 2)
							{
								OLED_WriteMFString(MF_ASCII_SIZE_7X8, "down  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
							}
							else if(CommandToExecute.CommandData[0] == 3)
							{
								OLED_WriteMFString(MF_ASCII_SIZE_7X8, "left  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
							}
							else if(CommandToExecute.CommandData[0] == 5)
							{
								OLED_WriteMFString(MF_ASCII_SIZE_7X8, "right ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
							}
						}
					}
					//vTaskDelay(2000);
					//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "Auto  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
					break;

				case OLED_CMD_TIME_IN:
					if(DisplayStatus != DISPLAY_STATUS_MENU)
					{
						DS3232M_GetTimeString(TimeString, 0);
						OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, TimeString, IDLE_TIME_COLUMN, IDLE_TIME_ROW, OLED_FONT_NORMAL);
						DS3232M_GetDateString(TimeString, 0);
						OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, TimeString, IDLE_DATE_COLUMN, IDLE_DATE_ROW, OLED_FONT_NORMAL);
					}
					break;

				default:
					OLED_WriteMFString(MF_ASCII_SIZE_7X8, InvalidString, IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
					break;
			}
		}
	}
}





/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	main routine for FreeRTOS
 * @return	Nothing, function should not exit
 */
int main(void)
{
	App_SetStatus(APP_STATUS_INIT);

	prvSetupHardware();

	Board_LED_Set(2, 1);
	Board_LED_Set(1, 0);
	Board_LED_Set(3, 0);

	i2c_app_init(I2C0, I2C_DEFAULT_SPEED);
	App_SSP_Init();
	App_Buzzer_Init();
	OLED_Init();
	vcom_init();
	App_Button_Init();
	DS3232M_Init();

	App_SetStatus(APP_STATUS_OK);


	/* LED1 toggle thread */
	xTaskCreate(vLEDTask1, (signed char *) "vTaskLed1", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2UL), (xTaskHandle *) NULL);

	/* LED2 toggle thread */
	xTaskCreate(vConsoleTask, (signed char *) "vConsole", ( unsigned short )400, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);

	/* OLED Display Task */
	xTaskCreate(vOLEDTask, (signed char *) "vOLEDTask", ( unsigned short )300, NULL, (tskIDLE_PRIORITY + 2UL), (xTaskHandle *) NULL);

	/* LED0 toggle thread */
	//xTaskCreate(vLEDTask0, (signed char *) "vTaskLed0", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

