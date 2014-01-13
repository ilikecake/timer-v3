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
	uint32_t prompt = 0;
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
	if(DS3232M_Init() != 0x00)
	{
		App_SetStatus(APP_STATUS_OSC_STOPPED);
	}

	DisplayTaskInit();
	InitTimerTask();

	//Init finished successfully, set status to OK
	if(App_GetStatus() == APP_STATUS_INIT)
	{
		App_SetStatus(APP_STATUS_OK);
	}

	/* LED1 toggle thread */
	xTaskCreate(vLEDTask1, (signed char *) "vTaskLed1", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2UL), &TaskList[0]);

	/* LED2 toggle thread */
	xTaskCreate(vConsoleTask, (signed char *) "vConsole", ( unsigned short )400, NULL, (tskIDLE_PRIORITY + 1UL), &TaskList[1]);

	/* OLED Display Task */
	xTaskCreate(DisplayTask, (signed char *) "vDisplay", ( unsigned short )200, NULL, (tskIDLE_PRIORITY + 2UL), &TaskList[2]);

	//Timer task
	//This should be the highest priority task
	xTaskCreate(TimerTask, (signed char *) "vTimer", ( unsigned short )64, NULL, (tskIDLE_PRIORITY + 4UL), &TaskList[3]);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

