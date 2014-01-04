/*
 * display_task.c
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#include "main.h"


//Private functions
void DrawIdleScreen(void);
void UpdateTimeAndDate(void);
void UpdateOutputs(void);
void DrawMenuScreen(void);




static void _MenuItem_TL (uint8_t Caller, uint8_t Previous);
static void _TimeMenu_1 (uint8_t Caller, uint8_t Previous);

const char _M0_Name[] 			= "Time";
const char _M1_Name[] 			= "Outputs";
const char _M2_Name[] 			= "Orientation";
const char _M3_Name[] 			= "Overrides";

uint8_t MenuData[8];

const MenuItem MenuItemList[] =
{
	//handler,			name,		up, 	down, 	left, 	right, 	Center}
	//Top level menu entries
	{ _MenuItem_TL,		_M0_Name,	3,		1,		0,		4, 		4},		//0
	{ _MenuItem_TL,		_M1_Name,	0,		2,		1,		1, 		1},		//1
	{ _MenuItem_TL,		_M2_Name,	1,		3,		2,		2, 		2},		//2
	{ _MenuItem_TL,		_M3_Name,	2,		0,		3,		3, 		3},		//3
	//Set time menu entries
	{ _TimeMenu_1,		NULL,		4,		4,		0,		4, 		0},		//4

};

//Top level menu item
void _MenuItem_TL (uint8_t Caller, uint8_t Previous)
{
	uint8_t i;
	uint8_t MenuItemSize;

	//TODO: make these compiler defines
	uint8_t Horiz = 4;
	uint8_t Vert = 39;

	//If the menu entry redirects to itself, we don't need to do anything
	if(Caller == Previous) return;

	if(Previous > 3)
	{
		DrawMenuScreen();
	}

	for(i=0;i<4;i++)
	{
		MenuItemSize = strlen(MenuItemList[i].name);
		OLED_ClearWindow(0, 32, Vert-2, Vert+9);
		OLED_WriteMFString(MF_ASCII_SIZE_7X8, MenuItemList[i].name, Horiz, Vert, OLED_FONT_NORMAL);
		if(i == Caller)
		{
			OLED_WriteLine((Horiz*4)-2, Vert-1, (Horiz*4)-2, Vert+9, 1, 1);
			OLED_WriteLine((Horiz*4+MenuItemSize*8)+1, Vert-1, (Horiz*4+MenuItemSize*8)+1, Vert+9, 1, 1);
			OLED_WriteLine((Horiz*4)-2, Vert-1, (Horiz*4+MenuItemSize*8)+1, Vert-1, 1, 1);
			OLED_WriteLine((Horiz*4)-2, Vert+9, (Horiz*4+MenuItemSize*8)+1, Vert+9, 1, 1);
		}

		Vert -= 11;
	}
	return;
}

static void _TimeMenu_1 (uint8_t Caller, uint8_t Previous)
{
	TimeAndDate CurrentTime;

	//TODO: make these compiler defines
	uint8_t Horiz = 4;
	uint8_t Vert = 39;

	//If the menu entry redirects to itself, we don't need to do anything
	if(Caller == Previous) return;

	DS3232M_GetTime(&CurrentTime);

	MenuData[0] = CurrentTime.min;
	MenuData[1] = CurrentTime.hour;
	MenuData[2] = CurrentTime.day;
	MenuData[3] = CurrentTime.month;
	MenuData[4] = CurrentTime.year;
	MenuData[5] = CurrentTime.dow;

	DrawMenuScreen();
	OLED_WriteMFString(MF_ASCII_SIZE_7X8, "> Time", 12, 55, OLED_FONT_NORMAL);

	OLED_WriteMFString(MF_ASCII_SIZE_7X8, "Time:   :", Horiz, Vert, OLED_FONT_NORMAL);
	if(MenuData[1] > 12)
	{
		OLED_WriteMF_UInt(MF_ASCII_SIZE_7X8, MenuData[1]-12, Horiz+12, Vert, OLED_FONT_NORMAL, 2);
		OLED_WriteMFString(MF_ASCII_SIZE_7X8, "PM", Horiz+23, Vert, OLED_FONT_NORMAL);
	}
	else
	{
		OLED_WriteMF_UInt(MF_ASCII_SIZE_7X8, MenuData[1], Horiz+12, Vert, OLED_FONT_NORMAL, 2);
		OLED_WriteMFString(MF_ASCII_SIZE_7X8, "AM", Horiz+23, Vert, OLED_FONT_NORMAL);
	}
	OLED_WriteMF_UInt(MF_ASCII_SIZE_7X8, MenuData[0], Horiz+18, Vert, OLED_FONT_NORMAL, 2);


	OLED_WriteMFString(MF_ASCII_SIZE_7X8, "Date: ", Horiz, Vert-11, OLED_FONT_NORMAL);
	OLED_WriteMFString(MF_ASCII_SIZE_7X8, "Use DST: ", Horiz, Vert-22, OLED_FONT_NORMAL);


		//OLED_WriteLine((Horiz*4)-2, Vert-1, (Horiz*4)-2, Vert+9, 1, 1);
		//OLED_WriteLine((Horiz*4+MenuItemSize*8)+1, Vert-1, (Horiz*4+MenuItemSize*8)+1, Vert+9, 1, 1);
		//OLED_WriteLine((Horiz*4)-2, Vert-1, (Horiz*4+MenuItemSize*8)+1, Vert-1, 1, 1);
		//OLED_WriteLine((Horiz*4)-2, Vert+9, (Horiz*4+MenuItemSize*8)+1, Vert+9, 1, 1);







}


void DisplayTaskInit(void)
{
	//Create the queue to send data to the command interpreter
	xOLEDCommands = xQueueCreate( 2, sizeof( OLED_Command ) );

	return;
}

void DrawIdleScreen(void)
{
	char TimeString[11];
	MF_StringOptions StringOptions;
	MF_LineOptions LineOptions;

	LineOptions.LinePattern = 0xFF;
	LineOptions.LineWeight = 1;
	LineOptions.LineOptions = 0;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 0;
	StringOptions.EndPadding = 0;
	StringOptions.TopPadding = 0;
	StringOptions.BottomPadding = 0;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	OLED_ClearDisplay();

	//Draw the lines around the outputs
	LineOptions.XStart = 16*4;
	LineOptions.XEnd = 16*4;
	LineOptions.YStart = 0;
	LineOptions.YEnd = 16;
	OLED_WriteLine2(&LineOptions);

	LineOptions.XStart = 32*4;
	LineOptions.XEnd = 32*4;
	LineOptions.YStart = 0;
	LineOptions.YEnd = 16;
	OLED_WriteLine2(&LineOptions);

	LineOptions.XStart = 48*4;
	LineOptions.XEnd = 48*4;
	LineOptions.YStart = 0;
	LineOptions.YEnd = 16;
	OLED_WriteLine2(&LineOptions);

	LineOptions.XStart = 0;
	LineOptions.XEnd = 255;
	LineOptions.YStart = 16;
	LineOptions.YEnd = 16;
	OLED_WriteLine2(&LineOptions);

	//Display the labels for time, date and mode
	//StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.XStart = (IDLE_TIME_COLUMN-2)*4;
	StringOptions.YStart = IDLE_TIME_ROW+18;
	OLED_WriteMFString2("Time:", &StringOptions);

	StringOptions.XStart = (IDLE_DATE_COLUMN-2)*4;
	StringOptions.YStart = IDLE_DATE_ROW+18;
	OLED_WriteMFString2("Date:", &StringOptions);

	StringOptions.XStart = (IDLE_STATUS_COLUMN-11)*4;
	StringOptions.YStart = IDLE_STATUS_ROW;
	OLED_WriteMFString2("Mode:", &StringOptions);

	//TODO: move this one to the update output function
	StringOptions.XStart = (IDLE_STATUS_COLUMN)*4;
	StringOptions.YStart = IDLE_STATUS_ROW;
	OLED_WriteMFString2("Auto  ", &StringOptions);

	//Display the current time
	DS3232M_GetTimeString(TimeString, 0);
	StringOptions.CharSize = MF_ASCII_SIZE_WA;
	StringOptions.XStart = (IDLE_TIME_COLUMN)*4;
	StringOptions.YStart = IDLE_TIME_ROW;
	OLED_WriteMFString2(TimeString, &StringOptions);

	//Display the current date
	//StringOptions.CharSize = MF_ASCII_SIZE_WA;
	DS3232M_GetDateString(TimeString, 0);
	StringOptions.XStart = (IDLE_DATE_COLUMN)*4;
	StringOptions.YStart = IDLE_DATE_ROW;
	OLED_WriteMFString2(TimeString, &StringOptions);

	UpdateOutputs();

	return;
}

void UpdateTimeAndDate(void)
{
	char TimeString[11];
	MF_StringOptions StringOptions;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 0;
	StringOptions.EndPadding = 20;
	StringOptions.TopPadding = 0;
	StringOptions.BottomPadding = 0;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	//Display the current time
	DS3232M_GetTimeString(TimeString, 0);
	StringOptions.CharSize = MF_ASCII_SIZE_WA;
	StringOptions.XStart = (IDLE_TIME_COLUMN)*4;
	StringOptions.YStart = IDLE_TIME_ROW;
	OLED_WriteMFString2(TimeString, &StringOptions);

	//Display the current date
	DS3232M_GetDateString(TimeString, 0);
	StringOptions.XStart = (IDLE_DATE_COLUMN)*4;
	StringOptions.YStart = IDLE_DATE_ROW;
	OLED_WriteMFString2(TimeString, &StringOptions);

	return;
}

void DrawMenuScreen(void)
{
	MF_StringOptions StringOptions;
	MF_LineOptions LineOptions;

	LineOptions.LinePattern = 0xFF;
	LineOptions.LineWeight = 1;
	LineOptions.LineOptions = 0;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 0;
	StringOptions.EndPadding = 0;
	StringOptions.TopPadding = 0;
	StringOptions.BottomPadding = 0;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	OLED_ClearDisplay();

	StringOptions.XStart = 8;
	StringOptions.YStart = 55;
	OLED_WriteMFString2("Menu", &StringOptions);

	LineOptions.XStart = 0;
	LineOptions.XEnd = 255;
	LineOptions.YStart = 53;
	LineOptions.YEnd = 53;
	OLED_WriteLine2(&LineOptions);

	return;
}

void UpdateOutputs(void)
{
	uint8_t OutputState;
	MF_StringOptions StringOptions;

	StringOptions.CharSize = MF_ASCII_SIZE_WA;
	StringOptions.StartPadding = 0;
	StringOptions.EndPadding = 0;
	StringOptions.TopPadding = 0;
	StringOptions.BottomPadding = 0;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;







	OutputState = TimerGetOutputState();

	//Output 1
	StringOptions.XStart = 12;
	StringOptions.YStart = 0;
	if((OutputState & 0x01) == 0x01)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
		//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 1", 3, 0, OLED_FONT_INVERSE);
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
		//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 1", 3, 0, OLED_FONT_NORMAL);
	}
	OLED_WriteMFString2("Out 1", &StringOptions);

	//Output 2
	StringOptions.XStart = 76;
	StringOptions.YStart = 0;
	if((OutputState & 0x02) == 0x02)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
		//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 2", 19, 0, OLED_FONT_INVERSE);
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
		//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 2", 19, 0, OLED_FONT_NORMAL);
	}
	OLED_WriteMFString2("Out 2", &StringOptions);

	//Output 3
	StringOptions.XStart = 140;
	StringOptions.YStart = 0;
	if((OutputState & 0x04) == 0x04)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
		//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 3", 35, 0, OLED_FONT_INVERSE);
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
		//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 3", 35, 0, OLED_FONT_NORMAL);
	}
	OLED_WriteMFString2("Out 3", &StringOptions);

	//Output 4
	StringOptions.XStart = 204;
	StringOptions.YStart = 0;
	if((OutputState & 0x08) == 0x08)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
		//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 4", 51, 0, OLED_FONT_INVERSE);
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
		//OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, "Out 4", 51, 0, OLED_FONT_NORMAL);
	}
	OLED_WriteMFString2("Out 4", &StringOptions);

	return;
}

/* OLED display thread */
void DisplayTask(void *pvParameters)
{
	OLED_Command CommandToExecute;

	//MF_StringOptions StringOptions;

	uint8_t CurrentMenuItem = 0;
	uint8_t PreviousMenuItem = 1;	//Note: this cannot be zero, or the menu screen will not be drawn after the first button press
	//TimeAndDate CurrentTime;

	//char TimeString[11];

	uint8_t DisplayStatus = DISPLAY_STATUS_IDLE_BRIGHT;
	portBASE_TYPE QueueStatus;

	//uint8_t i;
	//uint8_t CharWidth;

	static char InitString[] = "OLED Initialized";
	static char InvalidString[] = "Invalid Command";

	if(App_GetStatus() == APP_STATUS_OK)
	{
		OLED_WriteMFString(MF_ASCII_SIZE_7X8, InitString, 16, 28, OLED_FONT_NORMAL);
		//OLED_WriteMFString(MF_ASCII_SIZE_7X8, InitString, 1, 1, OLED_FONT_INVERSE);
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
					OLED_WriteMFString(CommandToExecute.CommandData[2], CommandToExecute.CommandCharData, CommandToExecute.CommandData[0], CommandToExecute.CommandData[1], OLED_FONT_NORMAL);
					break;

				case OLED_CMD_BUTTON_IN:
					if(DisplayStatus != DISPLAY_STATUS_MENU)
					{
						CurrentMenuItem = 0;
						PreviousMenuItem = 1;
						//OLED_ClearDisplay();
						DisplayStatus = DISPLAY_STATUS_MENU;
						OLED_DisplayContrast(0x7F);
						DrawMenuScreen();
					}
					else
					{
						PreviousMenuItem = CurrentMenuItem;
						if(CommandToExecute.CommandData[0] == 4)
						{
							CurrentMenuItem = MenuItemList[CurrentMenuItem].CenterItem;
							//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "center", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
						}
						else
						{
							if((OLED_GetStatus() & OLED_STATUS_ORENTATION_MASK) == OLED_STATUS_ORENTATION_DOWN)
							{
								if(CommandToExecute.CommandData[0] == 1)
								{
									CurrentMenuItem = MenuItemList[CurrentMenuItem].DownItem;
									//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "down  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
								}
								else if(CommandToExecute.CommandData[0] == 2)
								{
									CurrentMenuItem = MenuItemList[CurrentMenuItem].UpItem;
									//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "up    ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
								}
								else if(CommandToExecute.CommandData[0] == 3)
								{
									CurrentMenuItem = MenuItemList[CurrentMenuItem].RightItem;
									//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "right ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
								}
								else if(CommandToExecute.CommandData[0] == 5)
								{
									CurrentMenuItem = MenuItemList[CurrentMenuItem].LeftItem;
									//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "left  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
								}
							}
							else
							{
								if(CommandToExecute.CommandData[0] == 1)
								{
									CurrentMenuItem = MenuItemList[CurrentMenuItem].UpItem;
									//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "up    ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
								}
								else if(CommandToExecute.CommandData[0] == 2)
								{
									CurrentMenuItem = MenuItemList[CurrentMenuItem].DownItem;
									//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "down  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
								}
								else if(CommandToExecute.CommandData[0] == 3)
								{
									CurrentMenuItem = MenuItemList[CurrentMenuItem].LeftItem;
									//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "left  ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
								}
								else if(CommandToExecute.CommandData[0] == 5)
								{
									CurrentMenuItem = MenuItemList[CurrentMenuItem].RightItem;
									//OLED_WriteMFString(MF_ASCII_SIZE_7X8, "right ", IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
								}
							}
						}
					}
					(*(MenuItemList[CurrentMenuItem].handler))(CurrentMenuItem, PreviousMenuItem);
					break;

				case OLED_CMD_TIME_IN:
					if( (DisplayStatus != DISPLAY_STATUS_MENU) )
					{
						//TODO: Clear the extra stuff from the display. WHen going from longer strings to shorter strings (12:59 -> 1:00) there is extra stuff on the display. (should be fixed, check this...)
						UpdateTimeAndDate();
						UpdateOutputs();
					}
					break;

				default:
					OLED_WriteMFString(MF_ASCII_SIZE_7X8, InvalidString, IDLE_STATUS_COLUMN, IDLE_STATUS_ROW, OLED_FONT_NORMAL);
					break;
			}
		}
	}
}
