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

	static char InitString[] = "OLED Initialized";
	static char InvalidString[] = "Invalid Command";

	if(App_GetStatus() == APP_STATUS_OK)
	{
		OLED_WriteMFString(MF_ASCII_SIZE_7X8, InitString, 16, 28);
		vTaskDelay(3000);
		OLED_ClearDisplay();
	}

	while (1)
	{
		xQueueReceive(xOLEDCommands, &CommandToExecute, portMAX_DELAY);
		//OLED_WriteMFString(MF_ASCII_SIZE_7X8, InvalidString, 16, 28);


		switch(CommandToExecute.CommandName)
		{
			case OLED_CMD_WRITE_STRING:
				OLED_WriteMFString(CommandToExecute.CommandData[2], CommandToExecute.CommandCharData, CommandToExecute.CommandData[0], CommandToExecute.CommandData[1]);
				break;

			case OLED_CMD_BUTTON_IN:
				if(CommandToExecute.CommandData[0] == 4)
				{
					OLED_WriteMFString(MF_ASCII_SIZE_7X8, "center", 0, 10);
				}
				else
				{
					if((OLED_GetStatus() & OLED_STATUS_ORENTATION_MASK) == OLED_STATUS_ORENTATION_DOWN)
					{
						if(CommandToExecute.CommandData[0] == 1)
						{
							OLED_WriteMFString(MF_ASCII_SIZE_7X8, "down", 0, 10);
						}
						else if(CommandToExecute.CommandData[0] == 2)
						{
							OLED_WriteMFString(MF_ASCII_SIZE_7X8, "up", 0, 10);
						}
						else if(CommandToExecute.CommandData[0] == 3)
						{
							OLED_WriteMFString(MF_ASCII_SIZE_7X8, "right", 0, 10);
						}
						else if(CommandToExecute.CommandData[0] == 5)
						{
							OLED_WriteMFString(MF_ASCII_SIZE_7X8, "left", 0, 10);
						}
					}
					else
					{
						if(CommandToExecute.CommandData[0] == 1)
						{
							OLED_WriteMFString(MF_ASCII_SIZE_7X8, "up", 0, 10);
						}
						else if(CommandToExecute.CommandData[0] == 2)
						{
							OLED_WriteMFString(MF_ASCII_SIZE_7X8, "down", 0, 10);
						}
						else if(CommandToExecute.CommandData[0] == 3)
						{
							OLED_WriteMFString(MF_ASCII_SIZE_7X8, "left", 0, 10);
						}
						else if(CommandToExecute.CommandData[0] == 5)
						{
							OLED_WriteMFString(MF_ASCII_SIZE_7X8, "right", 0, 10);
						}
					}
				}
				vTaskDelay(2000);
				OLED_ClearDisplay();

				break;



			default:
				OLED_WriteMFString(MF_ASCII_SIZE_7X8, InvalidString, 16, 28);
				break;




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
	prvSetupHardware();

	Board_LED_Set(2, 0);
	Board_LED_Set(1, 0);
	Board_LED_Set(3, 0);




	i2c_app_init(I2C0, I2C_DEFAULT_SPEED);
	DS3232M_Init();
	App_SSP_Init();
	App_Buzzer_Init();
	OLED_Init();
	vcom_init();

	App_Button_Init();


	App_SetStatus(APP_STATUS_OK);

	/* LED1 toggle thread */
	xTaskCreate(vLEDTask1, (signed char *) "vTaskLed1", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2UL), (xTaskHandle *) NULL);

	/* LED2 toggle thread */
	xTaskCreate(vConsoleTask, (signed char *) "vConsole", ( unsigned short )300, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);

	/* OLED Display Task */
	xTaskCreate(vOLEDTask, (signed char *) "vOLEDTask", ( unsigned short )300, NULL, (tskIDLE_PRIORITY + 2UL), (xTaskHandle *) NULL);

	/* LED0 toggle thread */
	//xTaskCreate(vLEDTask0, (signed char *) "vTaskLed0", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

