/*
 * display_task.c
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */
#include "main.h"

//Private functions
void DrawIdleScreen(void);
void UpdateOutputs(void);


void DisplayTaskInit(void)
{
	//Create the queue to send data to the command interpreter
	xOLEDCommands = xQueueCreate( 2, sizeof( OLED_Command ) );

	return;
}


void DrawIdleScreen(void)
{
	char TimeString[11];

	//Set up idle screen
	DS3232M_GetTimeString(TimeString, 0);
	OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, TimeString, IDLE_TIME_COLUMN, IDLE_TIME_ROW, OLED_FONT_NORMAL);

	//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 1", 3, 0, OLED_FONT_NORMAL);
	//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 2", 19, 0, OLED_FONT_INVERSE);
	//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 3", 35, 0, OLED_FONT_NORMAL);
	//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 4", 51, 0, OLED_FONT_NORMAL);

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

	UpdateOutputs();

	return;
}

void UpdateOutputs(void)
{
	uint8_t OutputState;

	OutputState = TimerGetOutputState();

	if((OutputState & 0x01) == 0x01)
	{
		OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 1", 3, 0, OLED_FONT_INVERSE);
	}
	else
	{
		OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 1", 3, 0, OLED_FONT_NORMAL);
	}

	if((OutputState & 0x02) == 0x02)
	{
		OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 2", 19, 0, OLED_FONT_INVERSE);
	}
	else
	{
		OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 2", 19, 0, OLED_FONT_NORMAL);
	}

	if((OutputState & 0x04) == 0x04)
	{
		OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 3", 35, 0, OLED_FONT_INVERSE);
	}
	else
	{
		OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 3", 35, 0, OLED_FONT_NORMAL);
	}

	if((OutputState & 0x08) == 0x08)
	{
		OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 4", 51, 0, OLED_FONT_INVERSE);
	}
	else
	{
		OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 4", 51, 0, OLED_FONT_NORMAL);
	}

	return;
}

/* OLED display thread */
void DisplayTask(void *pvParameters)
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
						//TODO: Clear the extra stuff from the display. WHen going from longer strings to shorter strings (12:59 -> 1:00) there is extra stuff on the display.
						DS3232M_GetTimeString(TimeString, 0);
						OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, TimeString, IDLE_TIME_COLUMN, IDLE_TIME_ROW, OLED_FONT_NORMAL);
						DS3232M_GetDateString(TimeString, 0);
						OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, TimeString, IDLE_DATE_COLUMN, IDLE_DATE_ROW, OLED_FONT_NORMAL);
						UpdateOutputs();
					}
					break;

				default:
					OLED_WriteMFString(MF_ASCII_SIZE_7X8, InvalidString, IDLE_STATUS_COLUMN, IDLE_STATUS_ROW);
					break;
			}
		}
	}
}
