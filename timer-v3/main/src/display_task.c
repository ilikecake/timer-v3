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
#define DISPLAY_STATUS_SET_OUTPUT	0x04


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



static void WriteSetTimeData(uint16_t ElementToHighlight);






static void SwitchToSetTime(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void TimeMenuDisplay(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void SetDSTHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void SetUTHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);


//static void SwitchToSetOutput(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
//static void SetOutputHandler(uint8_t ButtonPressed);
static void SetOutputHandler(uint8_t theButtonPressed, uint8_t theCurrentMenuItem, uint8_t thePreviousMenuItem);
static void UpdateOutputHandler(uint8_t theButtonPressed, uint8_t theCurrentMenuItem, uint8_t thePreviousMenuItem);

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

const char _OSM1_Name[]			= "Output";
const char _OSM2_Name[]			= "Event";
const char _OSM3_Name[]			= "State";
const char _OSM4_Name[]			= "Type";
const char _OSM5_Name[]			= "Time";
const char _OSM6_Name[]			= "DOW";
const char _OSM_Breadcrumb[]	= "> Outputs > Setup";

uint16_t MenuData[8];

const MenuItem MenuItemList[] =
{
	//handler,			name,			up, 	down, 	left, 	right, 	Center,	X Start,	Y Start}
	{_GoToIdle, 		NULL, 			0,		0,		0,		0,		0,		0,			0},			//0
	//Top level menu entries
	{ NULL,				_M0_Name,		4,		2,		0,		5, 		5,		16,			39},		//1
	{ NULL,				_M1_Name,		1,		3,		0,		11, 	11,		16,			28},		//2
	{ NULL,				_M2_Name,		2,		4,		0,		3, 		3,		16,			17},		//3
	{ NULL,				_M3_Name,		3,		1,		0,		4, 		4,		16,			6},			//4

	//Set time menu
	{ TimeMenuDisplay,	_TM1_Name,		7,		6,		1,		8, 		8,		16,			39},		//5
	{ TimeMenuDisplay,	_TM2_Name,		5,		7,		1,		9, 		9,		16,			28},		//6
	{ TimeMenuDisplay,	_TM3_Name,		6,		5,		1,		10, 	10,		16,			17},		//7

	//
	{SwitchToSetTime,	NULL,			8,		8,		8,		8,		8,		16,			39},		//8		- Set time
	{SetDSTHandler,		NULL,			9,		9,		6,		9,		9,		16,			28},		//9		- Set DST
	{SetUTHandler,		NULL,			10,		10,		7,		10,		10,		16,			17},		//10	- Set Time Zone

	//Output Menu
	{ NULL,				_OM1_Name,		13,		12,		2,		14, 	14,		16,			39},		//11
	{ NULL,				_OM2_Name,		11,		13,		2,		12, 	12,		16,			28},		//12
	{ NULL,				_OM3_Name,		12,		11,		2,		13, 	13,		16,			17},		//13

	//Set Output Menu
	{SetOutputHandler,	_OSM1_Name,		19,		15,		11,		20,		20,		16,			39},		//14
	{SetOutputHandler,	_OSM2_Name,		14,		16,		11,		20,		20,		16,			28},		//15
	{SetOutputHandler,	_OSM3_Name,		15,		17,		11,		20,		20,		16,			17},		//16
	{SetOutputHandler,	_OSM4_Name,		16,		18,		11,		20,		20,		120,		39},		//17
	{SetOutputHandler,	_OSM5_Name,		17,		19,		11,		20,		20,		120,		28},		//18
	{SetOutputHandler,	_OSM6_Name,		18,		14,		11,		20,		20,		120,		17},		//19

	{UpdateOutputHandler, NULL,			20,		20,		20,		20,		20,		0,			0},			//20

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

//TODO: Do i need to pass the current menu item and previous menu item, or can I just read them from the global registers?

//TODO: Combine the two output handlers?
//TODO: Clear the hours/minutes registers if the event type is switched
static void UpdateOutputHandler(uint8_t theButtonPressed, uint8_t theCurrentMenuItem, uint8_t thePreviousMenuItem)
{
	TimerEvent CurrentEvent;

	uint8_t MenuCaller;
	uint8_t SubmenuLevel;
	uint16_t ElementToHighlight = 0;

	/*MF_StringOptions StringOptions;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;*/

	//Determine which menu entry called the function
	if( (thePreviousMenuItem >= 14) && (thePreviousMenuItem <= 19 ) )
	{
		//TODO: Do I need to preserve the upper 8 bits of this?
		MenuData[2] = thePreviousMenuItem;
	}

	if(MenuData[2] <= 18)
	{
		ElementToHighlight = (1 << (MenuData[2]-14));
	}
	else
	{
		ElementToHighlight = (1<<7);
	}

	MenuCaller = (MenuData[2] & 0x00FF);
	SubmenuLevel = ((MenuData[2]>>8) & 0x00FF);

	//StringOptions.XStart = 0;
	//StringOptions.YStart = 0;
	//OLED_WriteMF_UInt2(MenuCaller, 0, &StringOptions);

	//StringOptions.XStart = 30;
	//StringOptions.YStart = 0;
	//OLED_WriteMF_UInt2(SubmenuLevel, 0, &StringOptions);

	TimerGetEvent(MenuData[0]-1, MenuData[1]-1, &CurrentEvent);

	switch(theButtonPressed)
	{
		case MENU_BUTTON_RIGHT:
			if( (MenuCaller == 18) && (MenuData[4] == TIMER_TASK_TYPE_TIME_EVENT) )
			{
				//Time input
				if(SubmenuLevel == 3)
				{
					SubmenuLevel = 1;
				}
				else
				{
					SubmenuLevel++;
				}
				MenuData[2] = (MenuData[2] & 0x00FF) | (SubmenuLevel << 8);
				ElementToHighlight = 1 << (4 + SubmenuLevel - 1);
			}
			else if( (MenuCaller == 19) && (MenuData[4] == TIMER_TASK_TYPE_TIME_EVENT) )
			{
				//Day of week
				if(SubmenuLevel == 7)
				{
					SubmenuLevel = 1;
				}
				else
				{
					SubmenuLevel++;
				}
				MenuData[2] = (MenuData[2] & 0x00FF) | (SubmenuLevel << 8);
				ElementToHighlight = 1 << (7 + SubmenuLevel - 1);
			}
			break;

		case MENU_BUTTON_UP:
			switch (MenuCaller)
			{
				case 14:
					//Change output number
					if(MenuData[0] < TIMER_OUTPUT_NUMBER)
					{
						MenuData[0]++;
					}
					else
					{
						MenuData[0] = 1;
					}
					TimerGetEvent(MenuData[0]-1, MenuData[1]-1, &CurrentEvent);
					MenuData[3] = CurrentEvent.EventOutputState;
					MenuData[4] = CurrentEvent.EventType;
					MenuData[5] = CurrentEvent.EventTime[0];
					MenuData[6] = CurrentEvent.EventTime[1];
					MenuData[7] = CurrentEvent.EventTime[2];
					ElementToHighlight = 1<<0;
					break;

				case 15:
					//Change event number
					if(MenuData[1] < TIMER_EVENT_NUMBER)
					{
						MenuData[1] ++;
					}
					else
					{
						MenuData[1] = 1;
					}
					TimerGetEvent(MenuData[0]-1, MenuData[1]-1, &CurrentEvent);
					MenuData[3] = CurrentEvent.EventOutputState;
					MenuData[4] = CurrentEvent.EventType;
					MenuData[5] = CurrentEvent.EventTime[0];
					MenuData[6] = CurrentEvent.EventTime[1];
					MenuData[7] = CurrentEvent.EventTime[2];
					ElementToHighlight = 1<<1;
					break;

				case 16:
					//Change event state
					if(MenuData[3] == 1)
					{
						MenuData[3] = 0;
					}
					else
					{
						MenuData[3] = 1;
					}
					ElementToHighlight = 1<<2;
					break;

				case 17:
					//Change event type
					if(MenuData[4] == TIMER_TASK_TYPE_STEADY_EVENT)
					{
						MenuData[4] = TIMER_TASK_EVENT_TYPE_NONE;
					}
					else
					{
						MenuData[4]++;
					}
					ElementToHighlight = 1<<3;
					break;

				case 18:
					if(SubmenuLevel == 1)
					{
						//Hours
						if(MenuData[6] == 12)
						{
							MenuData[6] = 1;
						}
						else
						{
							MenuData[6] ++;
						}
						ElementToHighlight = 1<<4;
					}
					else if(SubmenuLevel == 2)
					{
						//Minutes
						if(MenuData[7] == 59)
						{
							MenuData[7] = 0;
						}
						else
						{
							MenuData[7] ++;
						}
						ElementToHighlight = 1<<5;
					}
					else if(SubmenuLevel == 3)
					{
						//AM/PM
						if(MenuData[6] > 12)
						{
							MenuData[6] -= 12;
						}
						else
						{
							MenuData[6] += 12;
						}
						ElementToHighlight = 1<<6;
					}
					break;

				case 19:
					MenuData[5] = MenuData[5] ^ (1<<(SubmenuLevel-1));
					break;

			}
			break;

		case MENU_BUTTON_DOWN:
			switch (MenuCaller)
			{
				case 14:
					//Change output number
					if(MenuData[0] == 1)
					{
						MenuData[0] = TIMER_OUTPUT_NUMBER;
					}
					else
					{
						MenuData[0]--;
					}
					TimerGetEvent(MenuData[0]-1, MenuData[1]-1, &CurrentEvent);
					MenuData[3] = CurrentEvent.EventOutputState;
					MenuData[4] = CurrentEvent.EventType;
					MenuData[5] = CurrentEvent.EventTime[0];
					MenuData[6] = CurrentEvent.EventTime[1];
					MenuData[7] = CurrentEvent.EventTime[2];
					ElementToHighlight = 1<<0;
					break;

				case 15:
					//Change event number
					if(MenuData[1] == 1)
					{
						MenuData[1] = TIMER_EVENT_NUMBER;
					}
					else
					{
						MenuData[1]--;
					}
					TimerGetEvent(MenuData[0]-1, MenuData[1]-1, &CurrentEvent);
					MenuData[3] = CurrentEvent.EventOutputState;
					MenuData[4] = CurrentEvent.EventType;
					MenuData[5] = CurrentEvent.EventTime[0];
					MenuData[6] = CurrentEvent.EventTime[1];
					MenuData[7] = CurrentEvent.EventTime[2];
					ElementToHighlight = 1<<1;
					break;

				case 16:
					//Change event state
					if(MenuData[3] == 1)
					{
						MenuData[3] = 0;
					}
					else
					{
						MenuData[3] = 1;
					}
					ElementToHighlight = 1<<2;
					break;

				case 17:
					//Change event type
					if(MenuData[4] == TIMER_TASK_EVENT_TYPE_NONE)
					{
						MenuData[4] = TIMER_TASK_TYPE_STEADY_EVENT;
					}
					else
					{
						MenuData[4]--;
					}
					ElementToHighlight = 1<<3;
					break;

				case 18:
					if(SubmenuLevel == 1)
					{
						//Hours
						if(MenuData[6] == 1)
						{
							MenuData[6] = 12;
						}
						else
						{
							MenuData[6] --;
						}
						ElementToHighlight = 1<<4;
					}
					else if(SubmenuLevel == 2)
					{
						//Minutes
						if(MenuData[7] == 0)
						{
							MenuData[7] = 59;
						}
						else
						{
							MenuData[7] --;
						}
						ElementToHighlight = 1<<5;
					}
					else if(SubmenuLevel == 3)
					{
						//AM/PM
						if(MenuData[6] > 12)
						{
							MenuData[6] -= 12;
						}
						else
						{
							MenuData[6] += 12;
						}
						ElementToHighlight = 1<<6;
					}
					break;

				case 19:
					MenuData[5] = MenuData[5] ^ (1<<(SubmenuLevel-1));
					break;
			}
			break;

		case MENU_BUTTON_LEFT:
			if((SubmenuLevel == 0) || ((SubmenuLevel == 1) && ((MenuCaller == 18) || (MenuCaller == 19))) )
			{
				//TODO: Add code here to update the event in RAM if it is changed.
				PreviousMenuItem = 20;
				CurrentMenuItem = MenuData[2];
				_MenuItem_TL (PreviousMenuItem, PreviousMenuItem);
				ElementToHighlight = 0;
			}
			else if( (MenuCaller == 18) && (MenuData[4] == TIMER_TASK_TYPE_TIME_EVENT) )
			{
				//Time input
				SubmenuLevel--;
				MenuData[2] = (MenuData[2] & 0x00FF) | (SubmenuLevel << 8);
				ElementToHighlight = 1 << (4 + SubmenuLevel - 1);
			}
			else if( (MenuCaller == 19) && (MenuData[4] == TIMER_TASK_TYPE_TIME_EVENT) )
			{
				//Day of week
				SubmenuLevel--;
				MenuData[2] = (MenuData[2] & 0x00FF) | (SubmenuLevel << 8);
				ElementToHighlight = 1 << (7 + SubmenuLevel - 1);
			}
			break;















		//case MENU_BUTTON_RIGHT:
		//case MENU_BUTTON_CENTER:




	}







	WriteSetTimeData(ElementToHighlight);

	//StringOptions.XStart = 70;
	//StringOptions.YStart = 0;
	//OLED_WriteMF_UInt2(MenuCaller, 0, &StringOptions);

	//StringOptions.XStart = 100;
	//StringOptions.YStart = 0;
	//OLED_WriteMF_UInt2(SubmenuLevel, 0, &StringOptions);

	return;
}


static void SetOutputHandler(uint8_t theButtonPressed, uint8_t theCurrentMenuItem, uint8_t thePreviousMenuItem)
{
	TimerEvent CurrentEvent;

	if((thePreviousMenuItem < 14) || (thePreviousMenuItem > 19))
	{
		//Perform initial setup of the set output screen
		MenuData[0] = 1;		//Current Output
		MenuData[1] = 1;		//Current Event
		MenuData[2] = 0;		//This entry is used to determine which bit of data to modify. It needs to be initalized to zero for proper operation.

		//Get data for the current event
		TimerGetEvent(MenuData[0]-1, MenuData[1]-1, &CurrentEvent);

		MenuData[3] = CurrentEvent.EventOutputState;
		MenuData[4] = CurrentEvent.EventType;
		MenuData[5] = CurrentEvent.EventTime[0];
		MenuData[6] = CurrentEvent.EventTime[1];
		MenuData[7] = CurrentEvent.EventTime[2];

		//TODO: Put somthing here to only update the screen if needed.
		WriteSetTimeData(0);
	}
	else
	{
		//handle button press
	}



	return;
}

//TODO: Clear out the time and DOW data when not used.
/**Write the event data to the display
 *
 * 	ElementToHighlight:	Which element(s) to highlight. This is a bitmapped field.
 * 						A one in each bit position indicated that that data should be highlighted.
 * 						The bits are:
 * 							[0]  : Output number
 * 							[1]  : Event number
 * 							[2]  : Event state
 * 							[3]  : Event type
 * 							[4]  : First time field (hours)
 * 							[5]  : Second time field (minutes)
 * 							[6]  : Third time field (AM/PM)
 *
 * 							NOTE: The day of the week bits are not selectable in the ElementToHighlight field.
 * 							TODO: Should i allow the DOW bits to be set on the input?
 */
static void WriteSetTimeData(uint16_t ElementToHighlight)
{
	uint8_t FirstColumnDataOffset = 54;
	uint8_t SecondColumnDataOffset = 37;

	MF_StringOptions StringOptions;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	//Print output number
	if( ((ElementToHighlight >> 0) & 0x01) == 0x01)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	StringOptions.YStart = MenuItemList[14].Y_Start;
	StringOptions.XStart = MenuItemList[14].X_Start + FirstColumnDataOffset;
	OLED_WriteMF_UInt2(MenuData[0], 0, &StringOptions);


	//Print event number
	if( ((ElementToHighlight >> 1) & 0x01) == 0x01)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	StringOptions.YStart = MenuItemList[15].Y_Start;
	StringOptions.XStart = MenuItemList[15].X_Start + FirstColumnDataOffset;
	OLED_WriteMF_UInt2(MenuData[1], 0, &StringOptions);

	//Print event output state
	if( ((ElementToHighlight >> 2) & 0x01) == 0x01)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	StringOptions.YStart = MenuItemList[16].Y_Start;
	StringOptions.XStart = MenuItemList[16].X_Start + FirstColumnDataOffset;
	OLED_ClearWindow(StringOptions.XStart/4, StringOptions.XStart/4+7, StringOptions.YStart-1, StringOptions.YStart+10);
	if(MenuData[3] == 0)
	{
		OLED_WriteMFString2("Off", &StringOptions);
	}
	else
	{
		OLED_WriteMFString2("On", &StringOptions);
	}

	//Print event specific data
	StringOptions.XStart = MenuItemList[17].X_Start + SecondColumnDataOffset;
	StringOptions.YStart = MenuItemList[17].Y_Start;
	if( ((ElementToHighlight >> 3) & 0x01) == 0x01)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_ClearWindow(StringOptions.XStart/4, StringOptions.XStart/4+19, StringOptions.YStart-1, StringOptions.YStart+10);

	switch(MenuData[4])		//EventType
	{
		case TIMER_TASK_TYPE_TIME_EVENT:
			OLED_WriteMFString2("Timer", &StringOptions);
			StringOptions.YStart = MenuItemList[18].Y_Start;

			//Display hours
			if( ((ElementToHighlight >> 4) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			StringOptions.EndPadding = 2;
			if(MenuData[6] > 12)
			{
				OLED_WriteMF_UInt2(MenuData[6]-12, 2, &StringOptions);
			}
			else if(MenuData[6] == 0)
			{
				OLED_WriteMF_UInt2(12, 2, &StringOptions);
			}
			else
			{
				OLED_WriteMF_UInt2(MenuData[6], 2, &StringOptions);
			}

			StringOptions.StartPadding = 0;
			StringOptions.EndPadding = 0;
			StringOptions.FontOptions = OLED_FONT_NORMAL;

			StringOptions.XStart = MenuItemList[17].X_Start + SecondColumnDataOffset + 20;
			OLED_WriteMFString2(":", &StringOptions);

			StringOptions.StartPadding = 2;
			StringOptions.EndPadding = 1;

			//Display min
			if( ((ElementToHighlight >> 5) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			StringOptions.XStart = MenuItemList[17].X_Start + SecondColumnDataOffset + 28;
			OLED_WriteMF_UInt2(MenuData[7], 2, &StringOptions);

			//Find AM/PM
			if( ((ElementToHighlight >> 6) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			StringOptions.XStart = MenuItemList[17].X_Start + SecondColumnDataOffset + 52;
			StringOptions.EndPadding = 2;
			if(MenuData[6] > 11)
			{
				OLED_WriteMFString2("PM", &StringOptions);
			}
			else
			{
				OLED_WriteMFString2("AM", &StringOptions);
			}
			StringOptions.EndPadding = 1;


			//DOW
			ElementToHighlight |= MenuData[5] << 7;


			StringOptions.FontOptions = OLED_FONT_INVERSE;
			StringOptions.YStart = MenuItemList[19].Y_Start;
			StringOptions.XStart = MenuItemList[17].X_Start + SecondColumnDataOffset;

			//StringOptions.CharSize = MF_ASCII_SIZE_5X7;
			StringOptions.StartPadding = 1;

			if( ((ElementToHighlight >> 7) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			OLED_WriteMFString2("Su", &StringOptions);
			//StringOptions.XStart = 2;

			if( ((ElementToHighlight >> 8) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			StringOptions.XStart += 12+8;
			OLED_WriteMFString2("M", &StringOptions);
			//StringOptions.XStart = 2;

			if( ((ElementToHighlight >> 9) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			StringOptions.XStart += 12;
			OLED_WriteMFString2("T", &StringOptions);
			//StringOptions.XStart = 2;

			if( ((ElementToHighlight >> 10) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			StringOptions.XStart += 12;
			OLED_WriteMFString2("W", &StringOptions);
			//StringOptions.XStart = 2;

			if( ((ElementToHighlight >> 11) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			StringOptions.XStart += 12;
			OLED_WriteMFString2("Th", &StringOptions);
			//StringOptions.XStart = 2;

			if( ((ElementToHighlight >> 12) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			StringOptions.XStart += 12+8;
			OLED_WriteMFString2("F", &StringOptions);
			//StringOptions.XStart = 2;

			if( ((ElementToHighlight >> 13) & 0x01) == 0x01)
			{
				StringOptions.FontOptions = OLED_FONT_INVERSE;
			}
			else
			{
				StringOptions.FontOptions = OLED_FONT_NORMAL;
			}
			StringOptions.XStart += 12;
			OLED_WriteMFString2("S", &StringOptions);
			StringOptions.XStart = 2;
			StringOptions.FontOptions = OLED_FONT_NORMAL;













			break;

		case TIMER_TASK_TYPE_REPEATING_EVENT:
			OLED_WriteMFString2("Repeat", &StringOptions);
			break;

		case TIMER_TASK_TYPE_SUN_EVENT:
			OLED_WriteMFString2("Sun", &StringOptions);
			break;

		case TIMER_TASK_TYPE_STEADY_EVENT:
			OLED_WriteMFString2("Steady", &StringOptions);
			break;

		case TIMER_TASK_EVENT_TYPE_NONE:
			OLED_WriteMFString2("None", &StringOptions);
			break;
	}

	/*
	//DOW
	StringOptions.YStart -= 11;
	OLED_WriteMFString2("DOW:", &StringOptions);

	StringOptions.CharSize = MF_ASCII_SIZE_5X7;
	StringOptions.StartPadding = 1;
	StringOptions.XStart = 2+4*12-1+120;
	OLED_WriteMFString2("M", &StringOptions);
	//StringOptions.XStart = 2;

	StringOptions.XStart += 8;
	OLED_WriteMFString2("T", &StringOptions);
	//StringOptions.XStart = 2;

	StringOptions.XStart += 8;
	OLED_WriteMFString2("W", &StringOptions);
	//StringOptions.XStart = 2;

	StringOptions.XStart += 8;
	OLED_WriteMFString2("Th", &StringOptions);
	//StringOptions.XStart = 2;

	StringOptions.XStart += 8+4;
	OLED_WriteMFString2("F", &StringOptions);
	//StringOptions.XStart = 2;

	StringOptions.XStart += 8;
	OLED_WriteMFString2("S", &StringOptions);
	//StringOptions.XStart = 2;

	StringOptions.XStart += 8;
	OLED_WriteMFString2("Su", &StringOptions);
	StringOptions.XStart = 2;*/

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
	else if(Caller > 13 && Caller <= 19 )
	{
		MenuMin = 14;
		MenuMax = 19;
		MenuBreadcrumb = _OSM_Breadcrumb;
		//strncpy(MenuBreadcrumb, "> Time\0",7);
	}
	else
	{
		return;
	}

	//TODO: make these compiler defines

	//If the menu entry redirects to itself, we don't need to do anything
	if(Caller == Previous) return;

	if( (Previous < MenuMin) ||  (Previous > MenuMax) )
	{
		DrawMenuScreen();
		StringOptions.XStart = 44;
		StringOptions.YStart = 55;
		OLED_WriteMFString2(MenuBreadcrumb, &StringOptions);
	}

	//StringOptions.XStart = Horiz*4;
	//StringOptions.YStart = Vert;

	for(i=MenuMin;i<=MenuMax;i++)
	{
		StringOptions.XStart = MenuItemList[i].X_Start;
		StringOptions.YStart = MenuItemList[i].Y_Start;

		//TODO: Check the X limits, can i use strlen here?

		OLED_ClearWindow((StringOptions.XStart/4)-1, ((StringOptions.XStart/4) + strlen(MenuItemList[i].name)*2), StringOptions.YStart-2, StringOptions.YStart+9);

		if(i == Caller)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}



		OLED_WriteMFString2(MenuItemList[i].name, &StringOptions);

		//StringOptions.YStart-=11;
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
