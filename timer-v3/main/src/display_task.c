/*
 * display_task.c
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#include "main.h"


#define MENU_BUTTON_UP		1
#define MENU_BUTTON_DOWN	2
#define MENU_BUTTON_LEFT	3
#define MENU_BUTTON_RIGHT	4
#define MENU_BUTTON_CENTER	5
#define MENU_BUTTON_NONE	6		//TODO: Give this a better name later


//Defines for the OLED display task
#define MENU_TO_IDLE_TIME			30000
#define IDLE_TO_DIM_TIME			3000

//TODOL Change these to pixels instead of columns
#define IDLE_TIME_ROW				37
#define IDLE_TIME_COLUMN			4
#define IDLE_DATE_ROW				37
#define IDLE_DATE_COLUMN			35
#define IDLE_STATUS_ROW				19
#define IDLE_STATUS_COLUMN			13

#define DISPLAY_STATUS_IDLE_DIM		0x00
#define DISPLAY_STATUS_IDLE_BRIGHT	0x01
#define DISPLAY_STATUS_MENU			0x02
#define DISPLAY_STATUS_SET_TIME		0x03


//Private functions
void DrawIdleScreen(void);
void UpdateTimeAndDate(void);
void UpdateOutputs(void);
void DrawMenuScreen(void);

uint8_t CurrentMenuItem;
uint8_t PreviousMenuItem;

/** The ButtonHandler function is called when a button is pressed
 *   The default button handler function is the TopLevelMenu function.
 *   Other button handler functions can be registered to do specific tasks
 */
void (*ButtonHandler) (uint8_t);

static void TopLevelMenu(uint8_t ButtonPressed);
static void SetTimeHandler(uint8_t ButtonPressed);










static void SwitchToSetTime(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void TimeMenuDisplay(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);

static void SetDSTHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void SetUTHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);

static void SetOutputHandler(uint8_t ButtonPressed);

//static void SetTimeMenu(uint8_t ButtonPressed);

uint8_t DisplayStatus;

//static void _SetTimeMenu (uint8_t Caller, uint8_t Previous);
//static void _ConfirmSetTime (uint8_t Caller, uint8_t Previous);

static void _MenuItem_TL (uint8_t Caller, uint8_t Previous);
//static void _TimeMenu_1 (uint8_t Caller, uint8_t Previous);
static void _GoToIdle (uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);

const char _M0_Name[] 			= "Time";
const char _M1_Name[] 			= "Outputs";
const char _M2_Name[] 			= "Setup";
const char _M3_Name[] 			= "Overrides";
const char _TL_Breadcrumb[]		= "\0";

const char _TM1_Name[]			= "Time";
const char _TM2_Name[]			= "DST";
const char _TM3_Name[]			= "Timezone";
const char _TM_Breadcrumb[]		= "> Time";

const char _OM1_Name[]			= "Setup Outputs";
const char _OM2_Name[]			= "Reload from EEPROM";
const char _OM3_Name[]			= "Save to EEPROM";
const char _OM_Breadcrumb[]		= "> Outputs";

uint16_t MenuData[8];

const MenuItem MenuItemList[] =
{
	//handler,			name,			up, 	down, 	left, 	right, 	Center}
	{_GoToIdle, 		NULL, 			0,		0,		0,		0,		0},		//0
	//Top level menu entries
	{ NULL,				_M0_Name,		4,		2,		0,		5, 		5},		//1
	{ NULL,				_M1_Name,		1,		3,		0,		11, 	11},	//2
	{ NULL,				_M2_Name,		2,		4,		0,		3, 		3},		//3
	{ NULL,				_M3_Name,		3,		1,		0,		4, 		4},		//4

	//Set time menu
	{ TimeMenuDisplay,	_TM1_Name,		7,		6,		1,		8, 		8},		//5
	{ TimeMenuDisplay,	_TM2_Name,		5,		7,		1,		9, 		9},		//6
	{ TimeMenuDisplay,	_TM3_Name,		6,		5,		1,		10, 	10},	//7

	//
	{SwitchToSetTime,	NULL,			8,		8,		8,		8,		8},		//8			//Set time
	{SetDSTHandler,		NULL,			9,		9,		6,		9,		9},		//9			//Set DST
	{SetUTHandler,		NULL,			10,		10,		7,		10,		10},	//10		//Set DST

	//Output Menu
	{ NULL,				_OM1_Name,		13,		12,		2,		11, 	11},	//11
	{ NULL,				_OM2_Name,		11,		13,		2,		12, 	12},	//12
	{ NULL,				_OM3_Name,		12,		11,		2,		13, 	13},	//13

};



uint8_t MenuLevel;		//1=toplevel, 2=timemenu, ...




static void TopLevelMenu(uint8_t ButtonPressed)
{

	if(DisplayStatus != DISPLAY_STATUS_MENU)
	{
		CurrentMenuItem = 1;
		PreviousMenuItem = 2;
		DisplayStatus = DISPLAY_STATUS_MENU;
		OLED_DisplayContrast(0x7F);
		DrawMenuScreen();
		MenuLevel = 1;
	}
	else
	{
		PreviousMenuItem = CurrentMenuItem;
		switch(ButtonPressed)
		{
			case MENU_BUTTON_UP:
				CurrentMenuItem = MenuItemList[CurrentMenuItem].UpItem;
				break;

			case MENU_BUTTON_DOWN:
				CurrentMenuItem = MenuItemList[CurrentMenuItem].DownItem;
				break;

			case MENU_BUTTON_LEFT:
				CurrentMenuItem = MenuItemList[CurrentMenuItem].LeftItem;
				break;

			case MENU_BUTTON_RIGHT:
				CurrentMenuItem = MenuItemList[CurrentMenuItem].RightItem;
				break;

			case MENU_BUTTON_CENTER:
				CurrentMenuItem = MenuItemList[CurrentMenuItem].CenterItem;
				break;
		}
	}

	_MenuItem_TL (CurrentMenuItem, PreviousMenuItem);

	if(MenuItemList[CurrentMenuItem].handler != NULL)
	{
		(*(MenuItemList[CurrentMenuItem].handler))(ButtonPressed, CurrentMenuItem, PreviousMenuItem);
	}

	return;
}

static void TimeMenuDisplay(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	uint8_t Horiz = 4;
	uint8_t Vert = 39;
	MF_StringOptions StringOptions;		//TODO: Make this global?
	char StringToOutput[11];

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	StringOptions.XStart = Horiz*4;
	StringOptions.YStart = Vert;


	StringOptions.XStart = (16+Horiz)*4;
	DS3232M_GetTimeString(StringToOutput, 0);
	OLED_WriteMFString2(StringToOutput, &StringOptions);

	StringOptions.XStart = (32+Horiz)*4;
	DS3232M_GetDateString(StringToOutput, 0);
	OLED_WriteMFString2(StringToOutput, &StringOptions);

	StringOptions.YStart-=11;
	StringOptions.XStart = (16+Horiz+2)*4;
	if( (GetDST() & 0x01) == 0x01)
	{
		OLED_WriteMFString2("Yes", &StringOptions);
	}
	else
	{
		OLED_WriteMFString2("No", &StringOptions);
	}

	StringOptions.YStart-=11;
	StringOptions.XStart = (16+Horiz+2)*4;
	sprintf(StringToOutput, "UT%+d", GetUTOffset());
	OLED_WriteMFString2(StringToOutput, &StringOptions);

	return;
}

static void SwitchToSetTime(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	ButtonHandler = SetTimeHandler;
	SetTimeHandler(ButtonPressed);
	return;
}

static void SetTimeHandler(uint8_t ButtonPressed)
{
	TimeAndDate CurrentTime;
	MF_StringOptions StringOptions;
	uint8_t temp;

	uint8_t Horiz = 4;
	uint8_t Vert = 39;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;



	if(DisplayStatus != DISPLAY_STATUS_SET_TIME)
	{
		//Perform initial setup of the display time screen
		DS3232M_GetTime(&CurrentTime);

		//Save the current time to the menu data
		MenuData[0] = CurrentTime.hour;
		MenuData[1] = CurrentTime.min;
		MenuData[2] = CurrentTime.month;
		MenuData[3] = CurrentTime.day;
		MenuData[4] = CurrentTime.year;


		MenuData[5] = 0;	//Our position in the time and date string

		DrawMenuScreen();

		StringOptions.XStart = 4*11;
		StringOptions.YStart = 55;
		OLED_WriteMFString2("> Time > Set Time", &StringOptions);


		ButtonHandler = SetTimeHandler;
		DisplayStatus = DISPLAY_STATUS_SET_TIME;
	}
	else
	{
		PreviousMenuItem = CurrentMenuItem;
		switch(ButtonPressed)
		{
			case MENU_BUTTON_UP:
				if(MenuData[5] < 6)
				{
					if(MenuData[5] == 0)
					{
						//Change hours

						if(MenuData[0] == 23)
						{
							MenuData[0] = 0;
						}
						else
						{
							MenuData[0] ++;
						}
					}
					else if(MenuData[5] == 1)
					{
						//Change minutes
						if(MenuData[1] == 59)
						{
							MenuData[1] = 0;
						}
						else
						{
							MenuData[1] ++;
						}
					}
					else if(MenuData[5] == 2)
					{
						//Change AM/PM
						if(MenuData[0] >= 12)
						{
							MenuData[0] -= 12;
						}
						else
						{
							MenuData[0] += 12;
						}
					}
					else if(MenuData[5] == 3)
					{
						//Change month
						if(MenuData[2] == 12)
						{
							MenuData[2] = 1;
						}
						else
						{
							MenuData[2] ++;
						}

						//Check if the current days value is not valid for this month
						temp = DaysInTheMonth(MenuData[2], MenuData[4]);
						if(MenuData[3] > temp)
						{
							MenuData[3] = temp;
						}
					}
					else if(MenuData[5] == 4)
					{
						//Change day
						temp = DaysInTheMonth(MenuData[2], MenuData[4]);
						if(MenuData[3] == temp)
						{
							MenuData[3] = 1;
						}
						else
						{
							MenuData[3] ++;
						}
					}
					else if(MenuData[5] == 5)
					{
						//Change year
						MenuData[4] ++;

						//Check if the current days value is not valid for this year (this will only happen on leap years)
						temp = DaysInTheMonth(MenuData[2], MenuData[4]);
						if(MenuData[3] > temp)
						{
							MenuData[3] = temp;
						}
					}
				}
				break;

			case MENU_BUTTON_DOWN:
				if(MenuData[5] < 6)
				{
					if(MenuData[5] == 0)
					{
						//Change hours
						if(MenuData[0] == 0)
						{
							MenuData[0] = 23;
						}
						else
						{
							MenuData[0] --;
						}
					}
					else if(MenuData[5] == 1)
					{
						//Change minutes
						if(MenuData[1] == 00)
						{
							MenuData[1] = 59;
						}
						else
						{
							MenuData[1]--;
						}
					}
					else if(MenuData[5] == 2)
					{
						//Change AM/PM
						if(MenuData[0] >= 12)
						{
							MenuData[0] -= 12;
						}
						else
						{
							MenuData[0] += 12;
						}
					}
					else if(MenuData[5] == 3)
					{
						//Change month
						if(MenuData[2] == 1)
						{
							MenuData[2] = 12;
						}
						else
						{
							MenuData[2] --;
						}

						//Check if the current days value is not valid for this month
						temp = DaysInTheMonth(MenuData[2], MenuData[4]);
						if(MenuData[3] > temp)
						{
							MenuData[3] = temp;
						}
					}
					else if(MenuData[5] == 4)
					{
						//Change day
						temp = DaysInTheMonth(MenuData[2], MenuData[4]);
						if(MenuData[3] == 1)
						{
							MenuData[3] = temp;
						}
						else
						{
							MenuData[3] --;
						}
					}
					else if(MenuData[5] == 5)
					{
						//Change year
						MenuData[4] --;

						//Check if the current days value is not valid for this year (this will only happen on leap years)
						temp = DaysInTheMonth(MenuData[2], MenuData[4]);
						if(MenuData[3] > temp)
						{
							MenuData[3] = temp;
						}
					}
				}
				break;

			case MENU_BUTTON_LEFT:
				if(MenuData[5] < 6)
				{
					if(MenuData[5] == 0)
					{
						MenuData[5] = 5;
					}
					else
					{
						MenuData[5]--;
					}
				}
				else
				{
					if(MenuData[5] == 6)
					{
						MenuData[5] = 7;
					}
					else
					{
						MenuData[5] = 6;
					}
				}
				break;

			case MENU_BUTTON_RIGHT:
				if(MenuData[5] < 6)
				{
					if(MenuData[5] == 5)
					{
						MenuData[5] = 0;
					}
					else
					{
						MenuData[5]++;
					}
				}
				else
				{
					if(MenuData[5] == 6)
					{
						MenuData[5] = 7;
					}
					else
					{
						MenuData[5] = 6;
					}
				}
				break;

			case MENU_BUTTON_CENTER:
				if(MenuData[5] < 6)
				{
					MenuData[5] = 7;

					StringOptions.XStart = 4*(1);
					StringOptions.YStart = 2;
					OLED_WriteMFString2("Save Changes:", &StringOptions);
				}
				else
				{
					if(MenuData[5] == 6)
					{
						CurrentTime.sec = 0;
						CurrentTime.hour = MenuData[0];
						CurrentTime.min = MenuData[1];
						CurrentTime.month = MenuData[2];
						CurrentTime.day = MenuData[3];
						CurrentTime.year = MenuData[4];
						DS3232M_SetTime(&CurrentTime);
					}
					_GoToIdle(0,8,0);
					return;
				}
				break;
		}
	}



	if(MenuData[5] > 5)
	{
		StringOptions.YStart = 2;

		if(MenuData[5] == 6)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		StringOptions.XStart = 108;
		OLED_WriteMFString2("Yes", &StringOptions);

		if(MenuData[5] == 7)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		StringOptions.XStart = 148;
		OLED_WriteMFString2("No", &StringOptions);
	}

	StringOptions.FontOptions = OLED_FONT_NORMAL;
	StringOptions.XStart = 4*(Horiz+15-5);
	StringOptions.YStart = Vert-11;
	OLED_WriteMFString2(":", &StringOptions);

	//Write Hours
	StringOptions.XStart = 4*(Horiz+11-5)-1;
	StringOptions.YStart = Vert-11;
	if(MenuData[5] == 0)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}

	if(MenuData[0] > 12)
	{
		OLED_WriteMF_UInt2(MenuData[0]-12, 2, &StringOptions);
	}
	else if(MenuData[0] == 0)
	{
		OLED_WriteMF_UInt2(12, 2, &StringOptions);
	}
	else
	{
		OLED_WriteMF_UInt2(MenuData[0], 2, &StringOptions);
	}

	//Write AM/PM
	StringOptions.EndPadding = 2;
	StringOptions.XStart = 4*(Horiz+23-5);
	StringOptions.YStart = Vert-11;
	if(MenuData[5] == 2)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}

	if(MenuData[0] > 11)
	{
		OLED_WriteMFString2("PM", &StringOptions);
	}
	else
	{
		OLED_WriteMFString2("AM", &StringOptions);

	}
	StringOptions.EndPadding = 1;

	//Write minutes
	if(MenuData[5] == 1)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	//StringOptions.FontOptions = OLED_FONT_INVERSE;
	StringOptions.XStart = 4*(Horiz+17-5)+1;
	StringOptions.YStart = Vert-11;
	OLED_WriteMF_UInt2(MenuData[1], 2, &StringOptions);

	//Write the month
	StringOptions.XStart = 4*(Horiz+16+10)-1;
	StringOptions.YStart = Vert-11;
	if(MenuData[5] == 3)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_WriteMF_UInt2(MenuData[2], 2, &StringOptions);

	//Write the day
	StringOptions.XStart = 4*(Horiz+22+10)+1;
	if(MenuData[5] == 4)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_WriteMF_UInt2(MenuData[3], 2, &StringOptions);

	//Write the year
	StringOptions.XStart = 4*(Horiz+29+10);
	if(MenuData[5] == 5)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_WriteMF_UInt2(MenuData[4], 4, &StringOptions);

	//Write slashes separating the date
	StringOptions.StartPadding = 0;
	StringOptions.EndPadding = 0;
	StringOptions.FontOptions = OLED_FONT_NORMAL;
	StringOptions.XStart = 4*(Horiz+20+10)+1;
	OLED_WriteMFString2("/", &StringOptions);

	StringOptions.XStart = 4*(Horiz+27+10);
	OLED_WriteMFString2("/", &StringOptions);

	//Write the day of the week
	StringOptions.YStart = Vert-22;
	StringOptions.EndPadding = 32;
	OLED_WriteMFString2(DOW[GetDOW(MenuData[4], MenuData[2], MenuData[3])], &StringOptions);

	return;
}

static void SetDSTHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	uint8_t Horiz = 4;
	uint8_t Vert = 39;
	MF_StringOptions StringOptions;		//TODO: Make this global?
	//char StringToOutput[11];

	StringOptions.XStart = Horiz*4;
	StringOptions.YStart = Vert;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	StringOptions.YStart-=11;
	StringOptions.XStart = (16+Horiz+2)*4;

	if(PreviousMenuItem != 9)
	{
		MenuData[0] = GetDST() & 0x01;
	}
	else
	{
		if( (ButtonPressed == MENU_BUTTON_UP) || (ButtonPressed == MENU_BUTTON_DOWN) || (ButtonPressed == MENU_BUTTON_CENTER) )
		{
			if(MenuData[0] == 0x01)
			{
				MenuData[0] = 0x00;
			}
			else
			{
				MenuData[0] = 0x01;
			}

			SetDST(MenuData[0]);
		}
	}

	StringOptions.FontOptions = OLED_FONT_INVERSE;
	if(MenuData[0] == 0x01)
	{
		OLED_WriteMFString2("Yes", &StringOptions);
	}
	else
	{
		OLED_ClearWindow((16+Horiz+2+4), (16+Horiz+2+8), StringOptions.YStart, StringOptions.YStart+11);
		OLED_WriteMFString2("No", &StringOptions);
	}

	return;
}

static void SetUTHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	int8_t	UT_Offset;
	char StringToOutput[6];


	//uint8_t mod = 0;

	uint8_t Horiz = 4;
	uint8_t Vert = 39;
	MF_StringOptions StringOptions;		//TODO: Make this global?
	//char StringToOutput[11];

	StringOptions.XStart = Horiz*4;
	StringOptions.YStart = Vert;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_INVERSE;


	UT_Offset = GetUTOffset();

	if( (ButtonPressed == MENU_BUTTON_UP) )
	{
		UT_Offset++;
		SetUTOffset(UT_Offset);
	}
	else if(ButtonPressed == MENU_BUTTON_DOWN)
	{
		UT_Offset--;
		SetUTOffset(UT_Offset);
	}





	StringOptions.YStart-=11;
	//StringOptions.XStart = (16+Horiz+2)*4;

	StringOptions.YStart-=11;
	StringOptions.XStart = (16+Horiz+2)*4;
	sprintf(StringToOutput, "UT%+d", UT_Offset);
	OLED_WriteMFString2(StringToOutput, &StringOptions);



	return;
}

static void SetOutputHandler(uint8_t ButtonPressed)
{



	return;
}




//Update the menu screen
//TODO: Rename this function later
void _MenuItem_TL (uint8_t Caller, uint8_t Previous)
{
	uint8_t i;
	MF_StringOptions StringOptions;		//TODO: Make this global?

	const char* MenuBreadcrumb;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	//Figure out some way of making this global?
	uint8_t MenuMin;
	uint8_t MenuMax;
	if(Caller > 0 && Caller <= 4 )
	{
		MenuMin = 1;
		MenuMax = 4;
		MenuBreadcrumb = _TL_Breadcrumb;
		//strncpy(MenuBreadcrumb, "\0",1);
	}
	else if(Caller > 4 && Caller <= 7 )
	{
		MenuMin = 5;
		MenuMax = 7;
		MenuBreadcrumb = _TM_Breadcrumb;
		//strncpy(MenuBreadcrumb, "> Time\0",7);
	}
	else if(Caller > 10 && Caller <= 13 )
	{
		MenuMin = 11;
		MenuMax = 13;
		MenuBreadcrumb = _OM_Breadcrumb;
		//strncpy(MenuBreadcrumb, "> Time\0",7);
	}
	else
	{
		return;
	}

	//TODO: make these compiler defines
	uint8_t Horiz = 4;
	uint8_t Vert = 39;

	StringOptions.XStart = Horiz*4;
	StringOptions.YStart = Vert;

	//If the menu entry redirects to itself, we don't need to do anything
	if(Caller == Previous) return;

	if( (Previous < MenuMin) ||  (Previous > MenuMax) )
	{
		DrawMenuScreen();
		StringOptions.XStart = 4*11;
		StringOptions.YStart = 55;
		OLED_WriteMFString2(MenuBreadcrumb, &StringOptions);
	}

	StringOptions.XStart = Horiz*4;
	StringOptions.YStart = Vert;

	for(i=MenuMin;i<=MenuMax;i++)
	{
		OLED_ClearWindow(0, 32, StringOptions.YStart-2, StringOptions.YStart+9);

		if(i == Caller)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		OLED_WriteMFString2(MenuItemList[i].name, &StringOptions);

		StringOptions.YStart-=11;
	}
	return;
}

static void _GoToIdle (uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	ButtonHandler = TopLevelMenu;
	CurrentMenuItem = 0;
	PreviousMenuItem = 1;
	DisplayStatus = DISPLAY_STATUS_IDLE_BRIGHT;
	DrawIdleScreen();
	return;
}

void DisplayTaskInit(void)
{
	//Create the queue to send data to the command interpreter
	xDisplayCommands = xQueueCreate( 2, sizeof( DisplayCommand ) );

	//Initalize the menu variables
	//MenuGlobalVars[0] = 0;
	//MenuGlobalVars[1] = 0;
	//MenuGlobalVars[2] = 0;
	//MenuGlobalVars[3] = 0;
	//MenuGlobalVars[4] = 0;
	//MenuGlobalVars[5] = 0;
	//MenuGlobalVars[6] = 0;
	//MenuGlobalVars[7] = 0;

	CurrentMenuItem = 0;
	PreviousMenuItem = 1;

	ButtonHandler = TopLevelMenu;

	return;
}

void DrawIdleScreen(void)
{
	MF_StringOptions StringOptions;		//TODO: Make this global
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

	//Display the current time and date
	UpdateTimeAndDate();
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



	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.XStart = (IDLE_STATUS_COLUMN)*4;
	StringOptions.YStart = IDLE_STATUS_ROW;


	OutputState = TimerGetTimerState();

	switch(OutputState)
	{
		case TIMER_STATUS_OFF:
			OLED_WriteMFString2("Off     ", &StringOptions);
			break;

		case TIMER_STATUS_ON:
			OLED_WriteMFString2("Auto    ", &StringOptions);
			break;

		case TIMER_STATUS_PAUSED:
			OLED_WriteMFString2("Override", &StringOptions);
			break;
	}

	return;
}

/* OLED display thread */
void DisplayTask(void *pvParameters)
{
	DisplayCommand CommandToExecute;
	MF_StringOptions StringOptions;

	//uint32_t blarg;

	//char blarg[12];

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 0;
	StringOptions.EndPadding = 0;
	StringOptions.TopPadding = 0;
	StringOptions.BottomPadding = 0;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;


	//MF_StringOptions StringOptions;

	//uint8_t CurrentMenuItem = 0;
	//uint8_t PreviousMenuItem = 1;	//Note: this cannot be zero, or the menu screen will not be drawn after the first button press

	uint8_t HWStatus;
	//TimeAndDate CurrentTime;

	//char TimeString[11];

	DisplayStatus = DISPLAY_STATUS_IDLE_BRIGHT;
	portBASE_TYPE QueueStatus = pdFALSE;

	//uint8_t i;
	//uint8_t CharWidth;

	//static char InitString[] = "POST Successful";
	static char InvalidString[] = "Invalid Command";


	StringOptions.XStart = 16*4;
	StringOptions.YStart = 28;

	HWStatus = App_GetStatus();

	switch(HWStatus)
	{
		case APP_STATUS_OK:
		OLED_WriteMFString2("POST Successful", &StringOptions);

		//StringOptions.YStart = 28-11;
		//sprintf(blarg, "%d", -12);
		//OLED_WriteMFString2(blarg, &StringOptions);
		//blarg = 12;
		//OLED_WriteMF_Int2((void*)&12, 3, 0, &StringOptions);
		//StringOptions.YStart = 28;


		vTaskDelay(1000);
		break;

		case APP_STATUS_OSC_STOPPED:
			StringOptions.XStart = 10*4;
			OLED_WriteMFString2("Time invalid", &StringOptions);
			StringOptions.YStart = 28-11;
			OLED_WriteMFString2("Please reset the time.", &StringOptions);
			StringOptions.YStart = 28;
			vTaskDelay(3000);
			break;

		default:
			OLED_WriteMFString2("POST Error", &StringOptions);
			StringOptions.XStart = 37*4;
			OLED_WriteMF_UInt2(App_GetStatus(), 0, &StringOptions);	//year
			vTaskDelay(3000);
	}


	OLED_ClearDisplay();


	//Set up idle screen
	DrawIdleScreen();

	while (1)
	{

		//Wait for a command to come in from the queue
		if(DisplayStatus == DISPLAY_STATUS_IDLE_BRIGHT)
		{
			QueueStatus = xQueueReceive(xDisplayCommands, &CommandToExecute, IDLE_TO_DIM_TIME);
		}
		else if(DisplayStatus == DISPLAY_STATUS_IDLE_DIM)
		{
			QueueStatus = xQueueReceive(xDisplayCommands, &CommandToExecute, portMAX_DELAY);
		}
		else
		{
			QueueStatus = xQueueReceive(xDisplayCommands, &CommandToExecute, MENU_TO_IDLE_TIME);
		}

		//If a command did not come in before the queue receive timed out, change the mode
		if(QueueStatus == pdFALSE)
		{
			if(DisplayStatus == DISPLAY_STATUS_IDLE_BRIGHT)
			{
				OLED_DisplayContrast(0);
				DisplayStatus = DISPLAY_STATUS_IDLE_DIM;
			}
			else
			{
				ButtonHandler = TopLevelMenu;
				CurrentMenuItem = 0;
				PreviousMenuItem = 1;
				DrawIdleScreen();
				DisplayStatus = DISPLAY_STATUS_IDLE_BRIGHT;
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
					//A button has been pressed. Figure out which one and call the button handler.
					if(CommandToExecute.CommandData[0] == 4)
					{
						ButtonHandler(MENU_BUTTON_CENTER);
					}
					else
					{
						if((OLED_GetStatus() & OLED_STATUS_ORENTATION_MASK) == OLED_STATUS_ORENTATION_DOWN)
						{
							if(CommandToExecute.CommandData[0] == 1)
							{
								ButtonHandler(MENU_BUTTON_DOWN);
							}
							else if(CommandToExecute.CommandData[0] == 2)
							{
								ButtonHandler(MENU_BUTTON_UP);
							}
							else if(CommandToExecute.CommandData[0] == 3)
							{
								ButtonHandler(MENU_BUTTON_RIGHT);
							}
							else if(CommandToExecute.CommandData[0] == 5)
							{
								ButtonHandler(MENU_BUTTON_LEFT);
							}
						}
						else
						{
							if(CommandToExecute.CommandData[0] == 1)
							{
								ButtonHandler(MENU_BUTTON_UP);
							}
							else if(CommandToExecute.CommandData[0] == 2)
							{
								ButtonHandler(MENU_BUTTON_DOWN);
							}
							else if(CommandToExecute.CommandData[0] == 3)
							{
								ButtonHandler(MENU_BUTTON_LEFT);
							}
							else if(CommandToExecute.CommandData[0] == 5)
							{
								ButtonHandler(MENU_BUTTON_RIGHT);
							}
						}
					}

					break;

				case OLED_CMD_TIME_IN:
					if( (DisplayStatus == DISPLAY_STATUS_IDLE_DIM) || (DisplayStatus == DISPLAY_STATUS_IDLE_BRIGHT) )
					{
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
