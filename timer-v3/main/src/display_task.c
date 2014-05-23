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
//TODO: Make these EEPROM variables
#define MENU_TO_IDLE_TIME			30
#define IDLE_TO_DIM_TIME			3
#define OVERRIDE_TIME				30

//#define BRIGHT_DIMMING_VAL			0x7F
//#define DIM_DIMMING_VAL				0x00

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
#define DISPLAY_STATUS_STATUS		0x05
#define DISPLAY_STATUS_MESSAGE		0x06
#define DISPLAY_STATUS_IN_SUB_MENU	0x07


//Global string for temporary use
char ScratchString[15];
//TimeAndDate CurrentTime;
struct tm CurrentTime;

//Note: These variables are 8-bit values that represent the timeout vaules in seconds. THey must be multiplied by 1000 for use in the timer functions
uint8_t MenuToIdleTime;
uint8_t IdleToDimTime;
uint8_t OverrideTime;

uint8_t DimmingVaules[6] = {0x00, 0x10, 0x20, 0x40, 0x80, 0xF0};
uint8_t BrightDimmingValue;// = 4;
uint8_t DimDimmingValue;// = 0;

//Private functions
void DrawIdleScreen(void);
void UpdateTimeAndDate(void);
void UpdateOutputs(void);
void DrawMenuScreen(void);
void DrawStatusScreen(void);

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

static void SetRotationHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void SetTimeoutHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void SetDimmingHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void SetLocationHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void SetOverridesHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);

static void SwitchToSetTime(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void TimeMenuDisplay(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void SetDSTHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
static void SetUTHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);


//static void SwitchToSetOutput(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);
//static void SetOutputHandler(uint8_t ButtonPressed);
static void SetOutputHandler(uint8_t theButtonPressed, uint8_t theCurrentMenuItem, uint8_t thePreviousMenuItem);
static void UpdateOutputHandler(uint8_t theButtonPressed, uint8_t theCurrentMenuItem, uint8_t thePreviousMenuItem);


static void EEPROMHandler(uint8_t theButtonPressed, uint8_t theCurrentMenuItem, uint8_t thePreviousMenuItem);

void DisplayShowMessage(char *MessageToDisplay);

//static void SetTimeMenu(uint8_t ButtonPressed);

uint8_t DisplayStatus;

//static void _SetTimeMenu (uint8_t Caller, uint8_t Previous);
//static void _ConfirmSetTime (uint8_t Caller, uint8_t Previous);

static void _MenuItem_TL (uint8_t Caller, uint8_t Previous);
//static void _TimeMenu_1 (uint8_t Caller, uint8_t Previous);
static void _GoToIdle (uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem);

//TODO: do i need to define these seperately, or can I define them in the 'MenuItemList' struct directly
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
const char _OM2_Name[]			= "Load from EEPROM";
const char _OM3_Name[]			= "Save to EEPROM";
const char _OM_Breadcrumb[]		= "> Outputs";

const char _OSM1_Name[]			= "Output";
const char _OSM2_Name[]			= "Event";
const char _OSM3_Name[]			= "State";
const char _OSM4_Name[]			= "Type";
const char _OSM5_Name[]			= "Time";
const char _OSM6_Name[]			= "DOW";
const char _OSM_Breadcrumb[]	= "> Outputs > Setup";

const char _SETUP1_Name[]			= "Flip Display";
const char _SETUP2_Name[]			= "Timeouts";
const char _SETUP3_Name[]			= "Dimming";
const char _SETUP4_Name[]			= "Location";
const char _SETUP_Breadcrumb[]		= "> Setup";

const char _TIMING1_Name[]			= "Menu -> Idle:";
const char _TIMING2_Name[]			= "Idle -> Dim:";
const char _TIMING3_Name[]			= "Override:";
const char _TIMING_Breadcrumb[]		= "> Setup > Timeouts";

const char _DIMMING1_Name[]			= "Bright Level:";
const char _DIMMING2_Name[]			= "Dim Level:";
const char _DIMMING_Breadcrumb[]		= "> Setup > Dimming";

const char _LOCATION1_Name[]			= "Latitude:";
const char _LOCATION2_Name[]			= "Longitude:";
const char _LOCATION_Breadcrumb[]		= "> Setup > Location";

const char _OVERRIDE1_Name[]			= "Outputs:";
const char _OVERRIDE2_Name[]			= "Timeout:";
const char _OVERRIDE_Breadcrumb[]		= "> Overrides";


//TODO: the menu breadcrumb and the sublevel breadcrumbs are not aligned. (fixed I think)
//TODO: Make the X start and Y start into compiler defines
uint16_t MenuData[8];

const MenuItem MenuItemList[] =
{
	//handler,			name,				up, 	down, 	left, 	right, 	Center,	X Start,	Y Start}
	{_GoToIdle, 		NULL, 				0,		0,		0,		0,		0,		0,			0},			//0
	//Top level menu entries
	{ NULL,				_M0_Name,			4,		2,		0,		5, 		5,		16,			39},		//1		-Set time menu
	{ NULL,				_M1_Name,			1,		3,		0,		11, 	11,		16,			28},		//2		-Set outputs menu
	{ NULL,				_M2_Name,			2,		4,		0,		21, 	21,		16,			17},		//3		-Setup menu
	{ NULL,				_M3_Name,			3,		1,		0,		31, 	31,		120,		39},		//4		-Overrides menu

	//Set time menu
	{ TimeMenuDisplay,	_TM1_Name,			7,		6,		1,		8, 		8,		16,			39},		//5
	{ TimeMenuDisplay,	_TM2_Name,			5,		7,		1,		9, 		9,		16,			28},		//6
	{ TimeMenuDisplay,	_TM3_Name,			6,		5,		1,		10, 	10,		16,			17},		//7

	//
	{SwitchToSetTime,	NULL,				8,		8,		8,		8,		8,		16,			39},		//8		- Set time
	{SetDSTHandler,		NULL,				9,		9,		6,		9,		9,		16,			28},		//9		- Set DST
	{SetUTHandler,		NULL,				10,		10,		7,		10,		10,		16,			17},		//10	- Set Time Zone

	/** Output Menu */
	{ EEPROMHandler,	_OM1_Name,			13,		12,		2,		14, 	14,		16,			39},		//11	-Setup Outputs
	{ EEPROMHandler,	_OM2_Name,			11,		13,		2,		12, 	12,		16,			28},		//12	-Load from EEPROM
	{ EEPROMHandler,	_OM3_Name,			12,		11,		2,		13, 	13,		16,			17},		//13	-Save to EEPROM

	//Set Output Menu
	{SetOutputHandler,	_OSM1_Name,			19,		15,		11,		20,		20,		16,			39},		//14
	{SetOutputHandler,	_OSM2_Name,			14,		16,		11,		20,		20,		16,			28},		//15
	{SetOutputHandler,	_OSM3_Name,			15,		17,		11,		20,		20,		16,			17},		//16
	{SetOutputHandler,	_OSM4_Name,			16,		18,		11,		20,		20,		120,		39},		//17
	{SetOutputHandler,	_OSM5_Name,			17,		19,		11,		20,		20,		120,		28},		//18
	{SetOutputHandler,	_OSM6_Name,			18,		14,		11,		20,		20,		120,		17},		//19

	{UpdateOutputHandler, NULL,				20,		20,		20,		20,		20,		0,			0},			//20

	/** Setup menu */
	{SetRotationHandler,_SETUP1_Name,		24,		22,		3,		21,		21,		16,			39},		//21	-Flip display
	{NULL,				_SETUP2_Name,		21,		23,		3,		25,		25,		16,			28},		//22	-Timeouts
	{NULL,				_SETUP3_Name,		22,		24,		3,		27,		27,		16,			17},		//23	-Dimming
	{NULL,				_SETUP4_Name,		23,		21,		3,		29,		29,		120,		39},		//24	-Location (Lat and Long)

	/** Setup timeouts menu */
	{SetTimeoutHandler,	_TIMING1_Name,		26,		26,		22,		25,		25,		16,			39},		//25	-Menu to idle
	{SetTimeoutHandler,	_TIMING2_Name,		25,		26,		22,		26,		26,		16,			28},		//26	-Idle to dim
	//{SetTimeoutHandler,	_TIMING3_Name,		26,		25,		22,		27,		27,		16,			17},		//27	-Override

	/**Setup dimming menu */
	{SetDimmingHandler,	_DIMMING1_Name,		28,		28,		23,		27,		27,		16,			39},		//27	-Bright level
	{SetDimmingHandler,	_DIMMING2_Name,		27,		27,		23,		28,		28,		16,			28},		//28	-Dim level

	/**Set Lat and Long menu */
	{SetLocationHandler,	_LOCATION1_Name,	30,		30,		24,		29,		29,		16,			39},		//29	-Latitude
	{SetLocationHandler,	_LOCATION2_Name,	29,		29,		24,		30,		30,		16,			28},		//30	-Longitude

	{SetOverridesHandler,	_OVERRIDE1_Name,				32,		32,		4,		31,		31,		16,			39},		//31	-Outputs
	{SetOverridesHandler,	_OVERRIDE2_Name,				31,		31,		4,		32,		32,		16,			28},		//32	-Timeout






};
//TODO: Move the name and handler to the end of this struct to make it easier to layout



uint8_t MenuLevel;		//1=toplevel, 2=timemenu, ...

static void SetRotationHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	if (ButtonPressed == MENU_BUTTON_CENTER)
	{
		OLED_ClearDisplay();
		OLED_FlipRotation();
		_MenuItem_TL (CurrentMenuItem, 0);
	}
}

static void SetTimeoutHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	MF_StringOptions StringOptions;		//TODO: Make this global?

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	if(PreviousMenuItem < 25)
	{
		MenuData[0] 							= 0;				//The level of the handler
		MenuData[TIMEOUT_TYPE_MENU_TO_IDLE]		= MenuToIdleTime;
		MenuData[TIMEOUT_TYPE_IDLE_TO_DIM]		= IdleToDimTime;
		MenuData[TIMEOUT_TYPE_OVERRIDE]			= OverrideTime;
	}

	if(CurrentMenuItem == PreviousMenuItem)
	{
		if ((ButtonPressed == MENU_BUTTON_RIGHT) && (MenuData[0] == 0))
		{
			MenuData[0] = CurrentMenuItem-24;		//This will set MenuData[0] to the index of the timeout to change (1,2 or 3)
			DisplayStatus = DISPLAY_STATUS_IN_SUB_MENU;
		}
		else if(ButtonPressed == MENU_BUTTON_UP)
		{
			if(MenuData[MenuData[0]] < 250)
			{
				MenuData[MenuData[0]]++;
			}
			else
			{
				MenuData[MenuData[0]] = 1;
			}
		}
		else if(ButtonPressed == MENU_BUTTON_DOWN)
		{
			if(MenuData[MenuData[0]] > 1)
			{
				MenuData[MenuData[0]]--;
			}
			else
			{
				MenuData[MenuData[0]] = 250;
			}
		}
		else if((ButtonPressed == MENU_BUTTON_LEFT))// && (MenuData[0] != 0))
		{
			SetTimeout(MenuData[0], MenuData[MenuData[0]]);
			DisplayStatus = DISPLAY_STATUS_MENU;
			MenuData[0] = 0;
		}


	}

	//Menu to idle display
	StringOptions.XStart = 130;
	StringOptions.YStart = 39;
	sprintf(ScratchString, "%u sec", MenuData[1]);
	if(MenuData[0] == 1)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_ClearWindow(StringOptions.XStart/4, (StringOptions.XStart/4) + 16, StringOptions.YStart-StringOptions.BottomPadding, StringOptions.YStart+8+StringOptions.TopPadding);
	OLED_WriteMFString2(ScratchString, &StringOptions);

	//Idle to dim display
	StringOptions.YStart = 28;
	sprintf(ScratchString, "%u sec", MenuData[2]);
	if(MenuData[0] == 2)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_ClearWindow(StringOptions.XStart/4, (StringOptions.XStart/4) + 16, StringOptions.YStart-StringOptions.BottomPadding, StringOptions.YStart+8+StringOptions.TopPadding);
	OLED_WriteMFString2(ScratchString, &StringOptions);

	//Override timeout display
	/*StringOptions.YStart = 17;
	sprintf(ScratchString, "%u sec", MenuData[3]);
	if(MenuData[0] == 3)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_ClearWindow(StringOptions.XStart/4, (StringOptions.XStart/4) + 16, StringOptions.YStart-StringOptions.BottomPadding, StringOptions.YStart+8+StringOptions.TopPadding);
	OLED_WriteMFString2(ScratchString, &StringOptions);*/

	return;
}

static void SetDimmingHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	MF_StringOptions StringOptions;		//TODO: Make this global?

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	if(PreviousMenuItem < 27)
	{
		MenuData[0] = 0;							//The level of the handler
		MenuData[1]	= BrightDimmingValue;
		MenuData[2]	= DimDimmingValue;
	}

	if(CurrentMenuItem == PreviousMenuItem)
	{
		if ((ButtonPressed == MENU_BUTTON_RIGHT) && (MenuData[0] == 0))
		{
			MenuData[0] = CurrentMenuItem-26;		//This will set MenuData[0] to the index of the diming to change (1 or 2)
			DisplayStatus = DISPLAY_STATUS_IN_SUB_MENU;
			if(MenuData[MenuData[0]] == 6)
			{
				DisplayDimming(0);
			}
			else
			{
				DisplayDimming(MenuData[MenuData[0]]);
			}
			//DisplayDimming(MenuData[MenuData[0]]);
		}
		else if(ButtonPressed == MENU_BUTTON_UP)
		{
			if( ((CurrentMenuItem == 27) && (MenuData[MenuData[0]] == 5)) || ((CurrentMenuItem == 28) && (MenuData[MenuData[0]] == 6)) )
			{
				MenuData[MenuData[0]] = 0;
			}
			else
			{
				MenuData[MenuData[0]]++;
			}


			if(MenuData[MenuData[0]] == 6)
			{
				DisplayDimming(0);
			}
			else
			{
				DisplayDimming(MenuData[MenuData[0]]);
			}
		}
		else if(ButtonPressed == MENU_BUTTON_DOWN)
		{
			if(MenuData[MenuData[0]] > 0)
			{
				MenuData[MenuData[0]]--;
			}
			else if(CurrentMenuItem == 27)
			{
				MenuData[MenuData[0]] = 5;
			}
			else
			{
				MenuData[MenuData[0]] = 6;
			}

			if(MenuData[MenuData[0]] == 6)
			{
				DisplayDimming(0);
			}
			else
			{
				DisplayDimming(MenuData[MenuData[0]]);
			}
			//DisplayDimming(MenuData[MenuData[0]]);
		}
		else if((ButtonPressed == MENU_BUTTON_LEFT))// && (MenuData[0] != 0))
		{
			//SetTimeout(MenuData[0], MenuData[MenuData[0]]);
			SetDimming(MenuData[0], MenuData[MenuData[0]]);
			DisplayStatus = DISPLAY_STATUS_MENU;
			MenuData[0] = 0;
			DisplayDimming(BrightDimmingValue);
		}


	}

	//bright
	StringOptions.XStart = 130;
	StringOptions.YStart = 39;
	sprintf(ScratchString, "%u", MenuData[1]+1);
	if(MenuData[0] == 1)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_WriteMFString2(ScratchString, &StringOptions);

	//dim
	StringOptions.YStart = 28;

	if(MenuData[2] == 6)
	{
		memcpy ( ScratchString, "Off\0", 4);
		//sprintf(ScratchString, "Off");
	}
	else
	{
		sprintf(ScratchString, "%u  ", MenuData[2]+1);
	}

	if(MenuData[0] == 2)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_WriteMFString2(ScratchString, &StringOptions);



	return;
}

static void SetLocationHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	int16_t LHS;
	uint16_t RHS;

	MF_StringOptions StringOptions;		//TODO: Make this global?
	MF_LineOptions LineOptions;

	LineOptions.LinePattern = 0xFF;
	LineOptions.LineWeight = 1;
	LineOptions.LineOptions = 0;


	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 1;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 1;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	if(PreviousMenuItem < 29)
	{
		MenuData[0] = 0;							//The level of the handler		//TODO: I can tell this from currentmenu
		MenuData[1] = 0;							//The level of the digit

		GetLatitude(&LHS, &RHS);
		MenuData[2] = (uint16_t)LHS;
		MenuData[3] = RHS;

		GetLongitude(&LHS, &RHS);
		MenuData[4] = (uint16_t)LHS;
		MenuData[5] = RHS;
	}


	//TODO: separate the positive and negative sign from the rest of the calculation. If I use menudata[6-7] to store the negativeness of the two values, I can simplify the following code a bunch.
	if(CurrentMenuItem == PreviousMenuItem)
	{
		if (ButtonPressed == MENU_BUTTON_RIGHT)
		{
			if(MenuData[0] == 0)
			{
				DisplayStatus = DISPLAY_STATUS_IN_SUB_MENU;
			}

			MenuData[0]++;
			if( ((MenuData[0] > 8) && (CurrentMenuItem == 30)) || ((MenuData[0] > 7) && (CurrentMenuItem == 29)) )
			{
				MenuData[0] = 1;
			}
		}
		else if(ButtonPressed == MENU_BUTTON_UP)
		{
			switch(MenuData[0])
			{
				case 1:		//plus or minus
					if(CurrentMenuItem == 29)
					{
						MenuData[2] = (uint16_t)((int16_t)MenuData[2] * -1);
					}
					else
					{
						MenuData[4] = (uint16_t)((int16_t)MenuData[4] * -1);
					}
					break;

				case 2:
					if(CurrentMenuItem == 29)		//Tens digit for latitude
					{
						LHS = (int16_t)MenuData[2];
						if(LHS > 80)
						{
							LHS -= 80;
						}
						else if(LHS < -80)
						{
							LHS += 80;
						}
						else if(LHS < 0)
						{
							LHS -= 10;
						}
						else
						{
							LHS += 10;
						}
						MenuData[2] = (uint16_t)(LHS);
					}
					else	//Hundreds digit for longitude
					{
						LHS = (int16_t)MenuData[4];

						if(LHS > 300)
						{
							LHS -= 300;
						}
						else if(LHS >= 260)
						{
							LHS -= 200;
						}
						else if(LHS < -300)
						{
							LHS += 300;
						}
						else if(LHS <= -260)
						{
							LHS += 200;
						}
						else if(LHS < 0)
						{
							LHS -= 100;
						}
						else
						{
							LHS += 100;
						}
						MenuData[4] = (uint16_t)(LHS);
					}
					break;

				case 3:
					if(CurrentMenuItem == 29)		//Ones digit for latitude
					{
						if((int16_t)MenuData[2] < 0)
						{
							MenuData[2] = (uint16_t)((int16_t)MenuData[2] - 1);
						}
						else
						{
							MenuData[2] = (uint16_t)((int16_t)MenuData[2] + 1);
						}

						if((((int16_t)MenuData[2]) % 10) == 0)
						{
							if((int16_t)MenuData[2] < 0)
							{
								MenuData[2] = (uint16_t)((int16_t)MenuData[2] + 10);
							}
							else
							{
								MenuData[2] = (uint16_t)((int16_t)MenuData[2] - 10);
							}
						}
					}
					else	//Tens digit for longitude
					{
						if((int16_t)MenuData[4] < 0)
						{
							MenuData[4] = (uint16_t)((int16_t)MenuData[4] - 10);
						}
						else
						{
							MenuData[4] = (uint16_t)((int16_t)MenuData[4] + 10);
						}

						if( (((int16_t)MenuData[4] / 10) % 10) == 0)
						{
							if((int16_t)MenuData[4] < 0)
							{
								MenuData[4] = (uint16_t)((int16_t)MenuData[4] + 100);
							}
							else
							{
								MenuData[4] = (uint16_t)((int16_t)MenuData[4] - 100);
							}
						}
					}
					break;

				case 4:
					if(CurrentMenuItem == 29)		//first digit after decimal for latitude
					{
						MenuData[3] += 1000;
						if(MenuData[3] > 10000)
						{
							MenuData[3] -= 10000;
						}
					}
					else	//Ones digit for longitude
					{
						if((int16_t)MenuData[4] < 0)
						{
							MenuData[4] = (uint16_t)((int16_t)MenuData[4] - 1);
						}
						else
						{
							MenuData[4] = (uint16_t)((int16_t)MenuData[4] + 1);
						}

						if( (((int16_t)MenuData[4]) % 10) == 0)
						{
							if((int16_t)MenuData[4] < 0)
							{
								MenuData[4] = (uint16_t)((int16_t)MenuData[4] + 10);
							}
							else
							{
								MenuData[4] = (uint16_t)((int16_t)MenuData[4] - 10);
							}
						}
					}
					break;

				case 5:
					if(CurrentMenuItem == 29)
					{
						//Second digit after decimal for latitude
						MenuData[3] += 100;
						if( ((MenuData[3] / 100) % 10) == 0)
						{
							MenuData[3] -= 1000;
						}
					}
					else
					{
						//first digit after decimal for longitude
						MenuData[5] += 1000;
						if(MenuData[5] > 10000)
						{
							MenuData[5] -= 10000;
						}
					}
					break;

				case 6:
					if(CurrentMenuItem == 29)		//Third digit after decimal for latitude
					{
						MenuData[3] += 10;
						if( ((MenuData[3] / 10) % 10) == 0)
						{
							MenuData[3] -= 100;
						}
					}
					else	//Second digit after decimal for longitude
					{
						MenuData[5] += 100;
						if( ((MenuData[5] / 100) % 10) == 0)
						{
							MenuData[5] -= 1000;
						}
					}
					break;

				case 7:
					if(CurrentMenuItem == 29)		//Fourth digit after decimal for latitude
					{
						MenuData[3] += 1;
						if( (MenuData[3] % 10) == 0)
						{
							MenuData[3] -= 10;
						}
					}
					else	//Third digit after decimal for longitude
					{
						MenuData[5] += 10;
						if( ((MenuData[5] / 10) % 10) == 0)
						{
							MenuData[5] -= 100;
						}
					}
					break;

				case 8:
					if(CurrentMenuItem == 30)		//Fourth digit after decimal for longitude
					{
						MenuData[5] += 1;

						if( (MenuData[5] % 10) == 0)
						{
							MenuData[5] -= 10;
						}
					}
					break;
			}
		}
		else if(ButtonPressed == MENU_BUTTON_DOWN)
		{
			switch(MenuData[0])
			{
				case 1:		//plus or minus
					if(CurrentMenuItem == 29)
					{
						MenuData[2] = (uint16_t)((int16_t)MenuData[2] * -1);
					}
					else
					{
						MenuData[4] = (uint16_t)((int16_t)MenuData[4] * -1);
					}
					break;

				case 2:
					if(CurrentMenuItem == 29)		//Tens digit for latitude
					{
						LHS = (int16_t)MenuData[2];
						if((LHS > -10) && (LHS < 0))
						{
							LHS -= 80;
						}
						else if((LHS < 10) && (LHS >= 0))
						{
							LHS += 80;
						}
						else if(LHS < 0)
						{
							LHS += 10;
						}
						else
						{
							LHS -= 10;
						}
						MenuData[2] = (uint16_t)(LHS);
					}
					else	//Hundreds digit for longitude
					{
						LHS = (int16_t)MenuData[4];
						if((LHS >= 0) && (LHS < 100))
						{
							if((LHS % 100) >= 60)
							{
								LHS += 200;
							}
							else
							{
								LHS += 300;
							}
						}
						else if((LHS > -100) && (LHS < 0))
						{

							if(((-1*LHS) % 100) >= 60)
							{
								LHS -= 200;
							}
							else
							{
								LHS -= 300;
							}
						}
						else if(LHS < 0)
						{
							LHS += 100;
						}
						else
						{
							LHS -= 100;
						}
						MenuData[4] = (uint16_t)(LHS);
					}
					break;

				case 3:
					if(CurrentMenuItem == 29)		//Ones digit for latitude
					{
						if((int16_t)MenuData[2] > 0)
						{
							if(((int16_t)MenuData[2] % 10) == 0)
							{
								MenuData[2] = (uint16_t)(((int16_t)MenuData[2])+9);
							}
							else
							{
								MenuData[2] = (uint16_t)(((int16_t)MenuData[2])-1);
							}
						}
						else
						{
							if(((-1*(int16_t)MenuData[2]) % 10) == 0)	//TODO: Do I need to take the absolute value if I am only looking for x%10=0?
							{
								MenuData[2] = (uint16_t)(((int16_t)MenuData[2])-9);
							}
							else
							{
								MenuData[2] = (uint16_t)(((int16_t)MenuData[2])+1);
							}
						}
					}
					else	//Tens digit for longitude
					{
						if((int16_t)MenuData[4] > 0)
						{
							if( (((int16_t)MenuData[4])/10) % 10 == 0)
							{
								MenuData[4] = (uint16_t)(((int16_t)MenuData[4])+90);
							}
							else
							{
								MenuData[4] = (uint16_t)(((int16_t)MenuData[4])-10);
							}
						}
						else
						{
							if( ((((int16_t)MenuData[4]*-1))/10) % 10 == 0)
							{
								MenuData[4] = (uint16_t)(((int16_t)MenuData[4])-90);
							}
							else
							{
								MenuData[4] = (uint16_t)(((int16_t)MenuData[4])+10);
							}
						}
					}
					break;

				case 4:
					if(CurrentMenuItem == 29)		//first digit after decimal for latitude
					{
						if(MenuData[3] < 1000)
						{
							MenuData[3] += 9000;
						}
						else
						{
							MenuData[3] -= 1000;
						}
					}
					else	//Ones digit for longitude
					{
						if((int16_t)MenuData[4] > 0)
						{
							if( ((int16_t)MenuData[4]) % 10 == 0)
							{
								MenuData[4] = (uint16_t)(((int16_t)MenuData[4])+9);
							}
							else
							{
								MenuData[4] = (uint16_t)(((int16_t)MenuData[4])-1);
							}
						}
						else
						{
							if( (((int16_t)MenuData[4]*-1)) % 10 == 0)
							{
								MenuData[4] = (uint16_t)(((int16_t)MenuData[4])-9);
							}
							else
							{
								MenuData[4] = (uint16_t)(((int16_t)MenuData[4])+1);
							}
						}
					}
					break;

				case 5:
					if(CurrentMenuItem == 29)
					{
						//Second digit after decimal for latitude
						if(((MenuData[3]/100) % 10) == 0)
						{
							MenuData[3] += 900;
						}
						else
						{
							MenuData[3] -= 100;
						}
					}
					else
					{
						//first digit after decimal for longitude
						if(MenuData[5] < 1000)
						{
							MenuData[5] += 9000;
						}
						else
						{
							MenuData[5] -= 1000;
						}
					}
					break;

				case 6:
					if(CurrentMenuItem == 29)		//Third digit after decimal for latitude
					{
						if( ((MenuData[3] / 10) % 10) == 0)
						{
							MenuData[3] += 90;
						}
						else
						{
							MenuData[3] -= 10;
						}
					}
					else	//Second digit after decimal for longitude
					{

						if( ((MenuData[5] / 100) % 10) == 0)
						{
							MenuData[5] += 900;
						}
						else
						{
							MenuData[5] -= 100;
						}
					}
					break;

				case 7:
					if(CurrentMenuItem == 29)		//Fourth digit after decimal for latitude
					{

						if( (MenuData[3] % 10) == 0)
						{
							MenuData[3] += 9;
						}
						else
						{
							MenuData[3] -= 1;
						}
					}
					else	//Third digit after decimal for longitude
					{

						if( ((MenuData[5] / 10) % 10) == 0)
						{
							MenuData[5] += 90;
						}
						else
						{
							MenuData[5] -= 10;
						}
					}
					break;

				case 8:
					if(CurrentMenuItem == 30)		//Fourth digit after decimal for longitude
					{


						if( (MenuData[5] % 10) == 0)
						{
							MenuData[5] += 9;
						}
						else
						{
							MenuData[5] -= 1;
						}
					}
					break;
			}
		}
		else if((ButtonPressed == MENU_BUTTON_LEFT))// && (MenuData[0] != 0))
		{
			//Exit the submenu if the user is in position 1 and presses left
			if(MenuData[0] == 1)
			{
				if(CurrentMenuItem == 29)
				{
					SetLatitude((int16_t)MenuData[2], MenuData[3]);
				}
				else
				{

					SetLongitude((int16_t)MenuData[4], MenuData[5]);
				}
				UpdateSunriseAndSunset(1);
				DisplayStatus = DISPLAY_STATUS_MENU;
				MenuData[0] = 0;
			}
			else
			{
				MenuData[0]--;
			}
		}
	}

	//Latitude
	StringOptions.XStart = 128;
	StringOptions.YStart = 39;
	//StringOptions.EndPadding = 3;
	sprintf(ScratchString, "%+03d.%04d N", (int16_t)MenuData[2], MenuData[3]);
	StringOptions.FontOptions = OLED_FONT_NORMAL;
	OLED_WriteMFString2(ScratchString, &StringOptions);

	if((CurrentMenuItem == 29) && (MenuData[0] != 0))
	{

		if(MenuData[0] < 4)
		{
			LineOptions.XStart = 128 + 8*(MenuData[0]-1);
		}
		else
		{
			LineOptions.XStart = 128 + 8*(MenuData[0]);
		}

		LineOptions.XEnd = LineOptions.XStart+8;
		LineOptions.YStart = 39;
		LineOptions.YEnd = 39;
		OLED_WriteLine2(&LineOptions);
	}

	//Longitude

	StringOptions.XStart = 128;
	StringOptions.YStart = 28;
	StringOptions.StartPadding = 1;
	StringOptions.EndPadding = 1;
	sprintf(ScratchString, "%+04d.%04d E", (int16_t)MenuData[4], MenuData[5]);
	StringOptions.FontOptions = OLED_FONT_NORMAL;
	OLED_WriteMFString2(ScratchString, &StringOptions);

	if((CurrentMenuItem == 30) && (MenuData[0] != 0))
	{
		if(MenuData[0] < 5)
		{
			LineOptions.XStart = 128 + 8*(MenuData[0]-1);
		}
		else
		{
			LineOptions.XStart = 128 + 8*(MenuData[0]);
		}

		LineOptions.XEnd = LineOptions.XStart+8;
		LineOptions.YStart = 28;
		LineOptions.YEnd = 28;
		OLED_WriteLine2(&LineOptions);
	}





	/*if(MenuData[0] == 2)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}*/
	//OLED_WriteMFString2(ScratchString, &StringOptions);






	return;
}

static void SetOverridesHandler(uint8_t ButtonPressed, uint8_t CurrentMenuItem, uint8_t PreviousMenuItem)
{
	MF_StringOptions StringOptions;		//TODO: Make this global?
	uint8_t i;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 1;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 1;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	if(PreviousMenuItem < 31)
	{
		MenuData[0] = 0;							//The level of the handler
		MenuData[1] = TimerGetOverrideOutputs();
		MenuData[2] = TimerGetPauseTime();
	}

	if(CurrentMenuItem == PreviousMenuItem)
	{
		//Handle buttons
		if (ButtonPressed == MENU_BUTTON_RIGHT)
		{
			if(CurrentMenuItem == 31)
			{
				if(MenuData[0] < 4)
				{
					MenuData[0] ++;
				}
				else
				{
					MenuData[0] = 1;
				}
			}
			else
			{
				MenuData[0] = 10;
			}
			//MenuData[0] = CurrentMenuItem-24;		//This will set MenuData[0] to the index of the timeout to change (1,2 or 3)
			DisplayStatus = DISPLAY_STATUS_IN_SUB_MENU;
		}
		else if(ButtonPressed == MENU_BUTTON_UP)
		{
			if(CurrentMenuItem == 31)
			{
				MenuData[1] = MenuData[1] ^ (1<<(MenuData[0]-1));
			}
			else
			{
				if(MenuData[2] < 250)
				{
					MenuData[2]++;
				}
				else
				{
					MenuData[2] = 1;
				}
			}

		}
		else if(ButtonPressed == MENU_BUTTON_DOWN)
		{
			if(CurrentMenuItem == 31)
			{
				MenuData[1] = MenuData[1] ^ (1<<(MenuData[0]-1));
			}
			else
			{
				if(MenuData[2] > 1)
				{
					MenuData[2]--;
				}
				else
				{
					MenuData[2] = 250;
				}
			}
		}
		else if((ButtonPressed == MENU_BUTTON_LEFT))// && (MenuData[0] != 0))
		{
			if((CurrentMenuItem == 31) && (MenuData[0] > 1))
			{
				MenuData[0] --;
			}
			else
			{
				if(CurrentMenuItem == 31)
				{
					TimerSetOverrideOutputs(MenuData[1]);
				}
				else
				{
					TimerSetPauseTime(MenuData[2]);
				}
				DisplayStatus = DISPLAY_STATUS_MENU;
				MenuData[0] = 0;
			}
		}
	}

	//Write output states
	StringOptions.XStart = 130;
	StringOptions.YStart = 39;
	StringOptions.StartPadding = 2;
	OLED_ClearWindow(StringOptions.XStart/4, (StringOptions.XStart/4) + 16, StringOptions.YStart-StringOptions.BottomPadding, StringOptions.YStart+8+StringOptions.TopPadding);

	for(i=0;i<4;i++)
	{
		sprintf(ScratchString, "%u", i+1);
		if(MenuData[0] == (i+1))
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else if((MenuData[1] & (1<<i)) != 0)
		{
			StringOptions.FontOptions = OLED_FONT_BOX;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		OLED_WriteMFString2(ScratchString, &StringOptions);

		StringOptions.XStart = StringOptions.XStart +16;
	}

	//Write timeout to display
	StringOptions.XStart = 130;
	StringOptions.YStart = 28;
	sprintf(ScratchString, "%u min", MenuData[2]);
	if(MenuData[0] == 10)
	{
		StringOptions.FontOptions = OLED_FONT_INVERSE;
	}
	else
	{
		StringOptions.FontOptions = OLED_FONT_NORMAL;
	}
	OLED_ClearWindow(StringOptions.XStart/4, (StringOptions.XStart/4) + 16, StringOptions.YStart-StringOptions.BottomPadding, StringOptions.YStart+8+StringOptions.TopPadding);
	OLED_WriteMFString2(ScratchString, &StringOptions);

	return;
}

static void TopLevelMenu(uint8_t ButtonPressed)
{
	uint8_t Stuff;

	if((DisplayStatus == DISPLAY_STATUS_IDLE_BRIGHT) || (DisplayStatus == DISPLAY_STATUS_IDLE_DIM) || (DisplayStatus == DISPLAY_STATUS_MESSAGE))
	{
		//Brighten the display when a button is pressed
		if(DisplayStatus == DISPLAY_STATUS_IDLE_DIM)
		{
			DisplayDimming(BrightDimmingValue);
			DisplayStatus = DISPLAY_STATUS_IDLE_BRIGHT;
		}


		if(ButtonPressed == MENU_BUTTON_CENTER)
		{
			//Go to menu
			CurrentMenuItem = 1;
			PreviousMenuItem = 2;
			DisplayStatus = DISPLAY_STATUS_MENU;
			DisplayDimming(BrightDimmingValue);
			DrawMenuScreen();
			MenuLevel = 1;
		}
		else
		{
			switch(ButtonPressed)
			{

				case MENU_BUTTON_DOWN:
					Stuff = TIMER_TASK_CMD_STOP;
					xQueueSend(xTimerCommands, &Stuff, 50);
					UpdateOutputs();
					break;

				case MENU_BUTTON_UP:
					if(App_GetStatus() == APP_STATUS_OSC_STOPPED)
					{
						DisplayShowMessage("Invalid time and date");
					}
					else
					{
						Stuff = TIMER_TASK_CMD_START;
						xQueueSend(xTimerCommands, &Stuff, 50);
						UpdateOutputs();
					}
					break;

				case MENU_BUTTON_LEFT:
					//if(TimerGetTimerState() == TIMER_STATUS_PAUSED)
					//{

					//}
					DisplayStatus = DISPLAY_STATUS_STATUS;
					DisplayDimming(BrightDimmingValue);
					DrawStatusScreen();
					break;

				case MENU_BUTTON_RIGHT:
					Stuff = TIMER_TASK_CMD_PAUSE;
					xQueueSend(xTimerCommands, &Stuff, 50);
					UpdateOutputs();
					//DisplayShowMessage("Invalid time and date");
					//PauseTimer();
					break;



			}

			return;
		}
	}
	else if(DisplayStatus == DISPLAY_STATUS_STATUS)
	{
		if(ButtonPressed == MENU_BUTTON_RIGHT)
		{
			_GoToIdle(0,0,0);
		}
		return;
	}
	else if(DisplayStatus == DISPLAY_STATUS_IN_SUB_MENU)
	{
		//Nothing?
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


	StringOptions.XStart = (16+Horiz+2)*4;
	DS3232M_GetTimeString(StringToOutput, 1);
	OLED_WriteMFString2(StringToOutput, &StringOptions);

	StringOptions.XStart = (32+Horiz+2)*4;
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
	//TimeAndDate CurrentTime;
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
		MenuData[0] = CurrentTime.tm_hour;
		MenuData[1] = CurrentTime.tm_min;
		MenuData[2] = CurrentTime.tm_mon+1;
		MenuData[3] = CurrentTime.tm_mday;
		MenuData[4] = CurrentTime.tm_year+1900;


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
					StringOptions.YStart = 0;
					OLED_WriteMFString2("Save Changes:", &StringOptions);
				}
				else
				{
					if(MenuData[5] == 6)
					{
						CurrentTime.tm_sec = 0;
						CurrentTime.tm_hour = MenuData[0];
						CurrentTime.tm_min = MenuData[1];
						CurrentTime.tm_mon = MenuData[2]-1;
						CurrentTime.tm_mday = MenuData[3];
						CurrentTime.tm_year = MenuData[4]-1900;		//TODO: Make sure this is not < 0
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
		StringOptions.YStart = 0;

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
//TODO: Combine the set time menu with the top level menu handler
//TODO: Combine the two output handlers?
//TODO: Clear the hours/minutes registers if the event type is switched
static void UpdateOutputHandler(uint8_t theButtonPressed, uint8_t theCurrentMenuItem, uint8_t thePreviousMenuItem)
{
	TimerEvent CurrentEvent;
	int8_t i;
	uint8_t MenuCaller;
	uint8_t SubmenuLevel;
	uint16_t ElementToHighlight = 0;

	//Determine which menu entry called the function
	if( (thePreviousMenuItem >= 14) && (thePreviousMenuItem <= 19 ) )
	{
		//TODO: Do I need to preserve the upper 8 bits of this?
		MenuData[2] = thePreviousMenuItem;
	}

	MenuCaller = (MenuData[2] & 0x00FF);
	SubmenuLevel = ((MenuData[2]>>8) & 0x00FF);

	if(MenuCaller <= 17)
	{
		ElementToHighlight = (1 << (MenuCaller-14));
	}
	else if(MenuCaller == 18)
	{
		ElementToHighlight = 1 << (4 + SubmenuLevel - 1);
	}
	else
	{
		ElementToHighlight = 1 << (7 + SubmenuLevel - 1);
	}

	TimerGetEvent(MenuData[0]-1, MenuData[1]-1, &CurrentEvent);

	switch(theButtonPressed)
	{
		case MENU_BUTTON_RIGHT:
			if( (MenuCaller == 18) && (MenuData[4] == TIMER_EVENT_TYPE_TIMED) )
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
			else if( (MenuCaller == 18) && ((MenuData[4] == TIMER_EVENT_TYPE_SUNRISE) || (MenuData[4] == TIMER_EVENT_TYPE_SUNSET) || (MenuData[4] == TIMER_EVENT_TYPE_REPEATING)) )
			{
				//Time input
				if(SubmenuLevel == 2)
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
			else if( (MenuCaller == 19) && ((MenuData[4] == TIMER_EVENT_TYPE_TIMED) || (MenuData[4] == TIMER_EVENT_TYPE_SUNRISE) || (MenuData[4] == TIMER_EVENT_TYPE_SUNSET)) )
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
			else if (MenuCaller == 19)
			{
				//If we get to this condition, the menu item 19 does not have any submenu, reset the CurrentMenuItem and do not highlight anything.
				CurrentMenuItem = 19;
				ElementToHighlight = 0;
			}
			else if(MenuCaller == 18)
			{
				//If we get to this condition, the menu item 18 does not have any submenu, reset the CurrentMenuItem and do not highlight anything.
				CurrentMenuItem = 18;
				ElementToHighlight = 0;
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

					//If the event type is none, repeating, or steady, only get the first event for this output
					TimerGetEvent(MenuData[0]-1, 0, &CurrentEvent);
					if( (CurrentEvent .EventType == TIMER_EVENT_TYPE_NONE) || (CurrentEvent .EventType == TIMER_EVENT_TYPE_REPEATING) || (CurrentEvent .EventType == TIMER_EVENT_TYPE_STEADY) )
					{
						MenuData[1] = 1;
					}
					else
					{
						//This output has timed or sun based events. Look for the last event up to the previously set event
						for(i=0;i<MenuData[1];i++)
						{
							TimerGetEvent(MenuData[0]-1, i, &CurrentEvent);
							if((CurrentEvent.EventType != TIMER_EVENT_TYPE_SUNRISE) || (CurrentEvent.EventType != TIMER_EVENT_TYPE_SUNSET) || (CurrentEvent.EventType != TIMER_EVENT_TYPE_TIMED))
							{
								MenuData[1] = i+1;
								break;
							}
						}
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
					//Only change events if the event is timed or sun based
					//Also, if the current event is none, do not let the user advance to the next event.
					TimerGetEvent(MenuData[0]-1, 0, &CurrentEvent);
					if(((CurrentEvent.EventType == TIMER_EVENT_TYPE_TIMED) || (CurrentEvent.EventType == TIMER_EVENT_TYPE_SUNRISE) || (CurrentEvent.EventType == TIMER_EVENT_TYPE_SUNSET)) && (MenuData[4] != TIMER_EVENT_TYPE_NONE))
					{
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
					}
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

					//If this is the first event for a given output, allow the user to select any event type except for none.
					if(MenuData[1] == 1)
					{
						if(MenuData[4] == TIMER_EVENT_LAST)
						{
							MenuData[4] = TIMER_EVENT_FIRST+1;
						}
						else
						{
							MenuData[4]++;
						}
					}
					else
					{

						//If the current output is timed or sun based, only allow the user to select timed or sun based events
						TimerGetEvent(MenuData[0]-1, 0, &CurrentEvent);
						if((CurrentEvent.EventType == TIMER_EVENT_TYPE_TIMED) || (CurrentEvent.EventType == TIMER_EVENT_TYPE_SUNRISE) || (CurrentEvent.EventType == TIMER_EVENT_TYPE_SUNSET))
						{
							if(MenuData[4] == TIMER_EVENT_LAST_TIMED)
							{
								MenuData[4] = TIMER_EVENT_TYPE_NONE;
							}
							else if(MenuData[4] == TIMER_EVENT_TYPE_NONE)
							{
								MenuData[4] = TIMER_EVENT_FIRST_TIMED;
							}
							else
							{
								MenuData[4]++;
							}
						}
						else
						{
							App_Die(12);
							if(MenuData[4] == TIMER_EVENT_LAST)
							{
								MenuData[4] = TIMER_EVENT_FIRST;
							}
							else
							{
								MenuData[4]++;
							}
						}
					}

					ElementToHighlight = 1<<3;
					MenuData[5] = 0xFF;
					if(MenuData[4] == TIMER_EVENT_TYPE_TIMED)
					{
						MenuData[6] = 12;
					}
					else
					{
						MenuData[6] = 0;
					}

					MenuData[7] = 0;

					break;

				case 18:
					if(MenuData[4] == TIMER_EVENT_TYPE_TIMED)
					{
						if(SubmenuLevel == 1)
						{
							//Hours
							if(MenuData[6] == 24)
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
					}
					else if((MenuData[4] == TIMER_EVENT_TYPE_SUNRISE) || (MenuData[4] == TIMER_EVENT_TYPE_SUNSET) || (MenuData[4] == TIMER_EVENT_TYPE_REPEATING))
					{
						if(SubmenuLevel == 1)
						{
							//Hours
							if(MenuData[6] == 12)
							{
								MenuData[6] = 0;
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
					}
					break;

				case 19:
					MenuData[5] = MenuData[5] ^ (1<<(SubmenuLevel-1));
					ElementToHighlight = 1 << (7 + SubmenuLevel - 1);
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

					//If the event type is none, repeating, or steady, only get the first event for this output
					TimerGetEvent(MenuData[0]-1, 0, &CurrentEvent);
					if( (CurrentEvent .EventType == TIMER_EVENT_TYPE_NONE) || (CurrentEvent .EventType == TIMER_EVENT_TYPE_REPEATING) || (CurrentEvent .EventType == TIMER_EVENT_TYPE_STEADY) )
					{
						MenuData[1] = 1;
					}
					else
					{
						//This output has timed or sun based events. Look for the last event up to the previously set event
						for(i=0;i<MenuData[1];i++)
						{
							TimerGetEvent(MenuData[0]-1, i, &CurrentEvent);
							if((CurrentEvent.EventType != TIMER_EVENT_TYPE_SUNRISE) || (CurrentEvent.EventType != TIMER_EVENT_TYPE_SUNSET) || (CurrentEvent.EventType != TIMER_EVENT_TYPE_TIMED))
							{
								MenuData[1] = i+1;
								break;
							}
						}
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
					//Only change events if the event is timed or sun based
					TimerGetEvent(MenuData[0]-1, 0, &CurrentEvent);
					if((CurrentEvent.EventType == TIMER_EVENT_TYPE_TIMED) || (CurrentEvent.EventType == TIMER_EVENT_TYPE_SUNRISE) || (CurrentEvent.EventType != TIMER_EVENT_TYPE_SUNSET))
					{
						if(MenuData[1] == 1)
						{
							//If the user is on event 1, and event six is of type none, do not go to event six.
							TimerGetEvent(MenuData[0]-1, TIMER_EVENT_NUMBER-1, &CurrentEvent);
							if(CurrentEvent.EventType != TIMER_EVENT_TYPE_NONE)
							{
								MenuData[1] = TIMER_EVENT_NUMBER;
							}
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
					}
					break;

				case 16:
					//Change event state
					//TODO: Does this need to be disabled for repeating events?
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

					//If this is the first event, allow the user to select any event type except for 'none.'
					if(MenuData[1] == 1)
					{
						if(MenuData[4] == TIMER_EVENT_FIRST+1)
						{
							MenuData[4] = TIMER_EVENT_LAST;
						}
						else
						{
							MenuData[4]--;
						}

					}
					else
					{
						//If the current output is timed or sun based, only allow the user to select timed or sun based events
						TimerGetEvent(MenuData[0]-1, 0, &CurrentEvent);
						if(((CurrentEvent.EventType == TIMER_EVENT_TYPE_TIMED) || (CurrentEvent.EventType == TIMER_EVENT_TYPE_SUNRISE)|| (CurrentEvent.EventType != TIMER_EVENT_TYPE_SUNSET)) && (MenuData[1] != 1))
						{
							if(MenuData[4] == TIMER_EVENT_TYPE_NONE)
							{
								MenuData[4] = TIMER_EVENT_LAST_TIMED;
							}
							else if(MenuData[4] == TIMER_EVENT_FIRST_TIMED)
							{
								MenuData[4] = TIMER_EVENT_TYPE_NONE;
							}
							else
							{
								MenuData[4]--;
							}
						}
						else
						{
							//I don't think this case can ever happen
							App_Die(6);

							if(MenuData[4] == TIMER_EVENT_TYPE_NONE)
							{
								MenuData[4] = TIMER_EVENT_TYPE_REPEATING;
							}
							else
							{
								MenuData[4]--;
							}
						}
					}

					ElementToHighlight = 1<<3;
					MenuData[5] = 0xFF;

					if(MenuData[4] == TIMER_EVENT_TYPE_TIMED)
					{
						MenuData[6] = 12;
					}
					else
					{
						MenuData[6] = 0;
					}
					MenuData[7] = 0;
					break;

				case 18:
					if(MenuData[4] == TIMER_EVENT_TYPE_TIMED)
					{
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
					}
					else if((MenuData[4] == TIMER_EVENT_TYPE_SUNRISE) || (MenuData[4] == TIMER_EVENT_TYPE_SUNSET) || (MenuData[4] == TIMER_EVENT_TYPE_REPEATING))
					{
						if(SubmenuLevel == 1)
						{
							//Hours
							if(MenuData[6] == 0)
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
					}
					break;

				case 19:
					MenuData[5] = MenuData[5] ^ (1<<(SubmenuLevel-1));
					ElementToHighlight = 1 << (7 + SubmenuLevel - 1);
					break;
			}
			break;

		case MENU_BUTTON_LEFT:
			if((SubmenuLevel == 0) || ((SubmenuLevel == 1) && ((MenuCaller == 18) || (MenuCaller == 19))) )
			{
				//TODO: Add code here to update the event in RAM if it is changed.

				CurrentEvent.EventOutputState	= (uint8_t) MenuData[3];
				CurrentEvent.EventType			= (uint8_t)MenuData[4];
				CurrentEvent.EventTime[0]		= (uint8_t)MenuData[5];
				CurrentEvent.EventTime[1]		= (uint8_t)MenuData[6];
				CurrentEvent.EventTime[2]		= (uint8_t)MenuData[7];
				TimerSetEvent(MenuData[0]-1, MenuData[1]-1, &CurrentEvent);

				PreviousMenuItem = 20;
				CurrentMenuItem = MenuData[2];
				_MenuItem_TL (PreviousMenuItem, PreviousMenuItem);
				ElementToHighlight = 0;
			}
			else if( (MenuCaller == 18) && (MenuData[4] != TIMER_EVENT_TYPE_STEADY) && (MenuData[4] != TIMER_EVENT_TYPE_NONE))
			{
				//Time input
				SubmenuLevel--;
				MenuData[2] = (MenuData[2] & 0x00FF) | (SubmenuLevel << 8);
				ElementToHighlight = 1 << (4 + SubmenuLevel - 1);
			}
			else if( (MenuCaller == 19) && ((MenuData[4] == TIMER_EVENT_TYPE_TIMED) || (MenuData[4] == TIMER_EVENT_TYPE_SUNRISE) || (MenuData[4] == TIMER_EVENT_TYPE_SUNSET)) )
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

//TODO: This will have to change when I add more types
//TODO: Clear out the time and DOW data when not used.
//TODO: Name this function something not dumb
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
 * 							[7]  : DOW, Sunday
 * 							[8]  : DOW, Monday
 * 							[9]  : DOW, Tuesday
 * 							[10] : DOW, Wednesday
 * 							[11] : DOW, Thursday
 * 							[12] : DOW, Friday
 * 							[12] : DOW, Saturday
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

	//StringOptions.XStart = 100;
	//StringOptions.YStart = 0;
	//OLED_WriteMF_UInt2(ElementToHighlight, 0, &StringOptions);


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




	OLED_WriteMFString2(EventNames[MenuData[4]], &StringOptions);


	/*switch(MenuData[4])		//EventType
	{
		//TODO: Make these into an array of strings
		case TIMER_EVENT_TYPE_TIMED:
			OLED_WriteMFString2("Timer", &StringOptions);
			break;

		case TIMER_EVENT_TYPE_REPEATING:
			OLED_WriteMFString2("Repeat", &StringOptions);
			break;

		case TIMER_EVENT_TYPE_SUNRISE:
			OLED_WriteMFString2("Sunrise", &StringOptions);
			break;

		case TIMER_EVENT_TYPE_SUNSET:
			OLED_WriteMFString2("Sunset", &StringOptions);
			break;

		case TIMER_EVENT_TYPE_STEADY:
			OLED_WriteMFString2("Steady", &StringOptions);
			break;

		case TIMER_EVENT_TYPE_NONE:
			OLED_WriteMFString2("None", &StringOptions);
			break;
	}*/


	//Time
	if(MenuData[4] == TIMER_EVENT_TYPE_TIMED)
	{
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
	}
	else if((MenuData[4] == TIMER_EVENT_TYPE_REPEATING) || (MenuData[4] == TIMER_EVENT_TYPE_SUNRISE) || (MenuData[4] == TIMER_EVENT_TYPE_SUNSET) )
	{
		//TODO: Combine this with the none event?
		OLED_ClearWindow((MenuItemList[17].X_Start + SecondColumnDataOffset)/4, (MenuItemList[17].X_Start + SecondColumnDataOffset)/4+30, MenuItemList[18].Y_Start-2, MenuItemList[18].Y_Start+10);
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
		OLED_WriteMF_UInt2(MenuData[6], 2, &StringOptions);


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

	}
	else
	{
		//If day of the week is not use, clear the field
		//TODO: do I want the clear the 'DOW' header as well?
		OLED_ClearWindow((MenuItemList[17].X_Start + SecondColumnDataOffset)/4, (MenuItemList[17].X_Start + SecondColumnDataOffset)/4+30, MenuItemList[18].Y_Start-2, MenuItemList[18].Y_Start+10);
	}




	//Day of the week
	if((MenuData[4] == TIMER_EVENT_TYPE_TIMED) || (MenuData[4] == TIMER_EVENT_TYPE_SUNRISE) || (MenuData[4] == TIMER_EVENT_TYPE_SUNSET))
	{
		//The day of the week field is only used for timer or sun based events
		StringOptions.YStart = MenuItemList[19].Y_Start;
		StringOptions.XStart = MenuItemList[19].X_Start + SecondColumnDataOffset;
		StringOptions.StartPadding = 2;
		StringOptions.EndPadding = 2;

		//Sunday
		if( ((ElementToHighlight >> 7) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else if(((MenuData[5]>>0) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_BOX;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		OLED_WriteMFString2("Su", &StringOptions);

		//Monday
		if( ((ElementToHighlight >> 8) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else if(((MenuData[5]>>1) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_BOX;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		StringOptions.XStart += 12+8;
		OLED_WriteMFString2("M", &StringOptions);

		//Tuesday
		if( ((ElementToHighlight >> 9) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else if(((MenuData[5]>>2) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_BOX;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		StringOptions.XStart += 12;
		OLED_WriteMFString2("T", &StringOptions);

		//Wednesday
		if( ((ElementToHighlight >> 10) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else if(((MenuData[5]>>3) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_BOX;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		StringOptions.XStart += 12;
		OLED_WriteMFString2("W", &StringOptions);

		//Thursday
		if( ((ElementToHighlight >> 11) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else if(((MenuData[5]>>4) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_BOX;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		StringOptions.XStart += 12;
		OLED_WriteMFString2("Th", &StringOptions);

		//Friday
		if( ((ElementToHighlight >> 12) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else if(((MenuData[5]>>5) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_BOX;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		StringOptions.XStart += 12+8;
		OLED_WriteMFString2("F", &StringOptions);

		//Saturday
		if( ((ElementToHighlight >> 13) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_INVERSE;
		}
		else if(((MenuData[5]>>6) & 0x01) == 0x01)
		{
			StringOptions.FontOptions = OLED_FONT_BOX;
		}
		else
		{
			StringOptions.FontOptions = OLED_FONT_NORMAL;
		}
		StringOptions.XStart += 12;
		OLED_WriteMFString2("S", &StringOptions);
	}
	else
	{
		//If day of the week is not use, clear the field
		//TODO: do I want the clear the 'DOW' header as well?
		OLED_ClearWindow((MenuItemList[17].X_Start + SecondColumnDataOffset)/4, (MenuItemList[17].X_Start + SecondColumnDataOffset)/4+30, MenuItemList[19].Y_Start-2, MenuItemList[19].Y_Start+10);
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

//TODO: Rename this to better identify what it does
static void EEPROMHandler(uint8_t theButtonPressed, uint8_t theCurrentMenuItem, uint8_t thePreviousMenuItem)
{
	uint8_t ret;

	MF_StringOptions StringOptions;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 2;
	StringOptions.EndPadding = 1;
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;
	StringOptions.XStart = 8;
	StringOptions.YStart = 0;

	if(theButtonPressed == MENU_BUTTON_CENTER)
	{
		if(theCurrentMenuItem == 12)
		{
			//Load from EEPROM
			ret = TimerReadEventsFromEEPROM();
			TimerValidateEventList();

			if(ret == 0x00)
			{
				OLED_WriteMFString2("Loaded event list from EEPROM", &StringOptions);
			}
			else
			{
				OLED_WriteMFString2("Error loading events", &StringOptions);
			}
			return;
		}
		else if(theCurrentMenuItem == 13)
		{
			//Save to EEPROM
			ret = TimerWriteEventsToEEPROM();

			if(ret == 0x00)
			{
				OLED_WriteMFString2("Event list saved to EEPROM", &StringOptions);
			}
			else
			{
				OLED_WriteMFString2("Error saving events", &StringOptions);
			}
			return;
		}
	}




	OLED_ClearWindow(0, 64, 0, 11);



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
	//TODO: Figure out a better way to do this...
	uint8_t MenuMin;
	uint8_t MenuMax;
	if(Caller > 0 && Caller <= 4 )
	{
		MenuMin = 1;
		MenuMax = 4;
		MenuBreadcrumb = _TL_Breadcrumb;
	}
	else if(Caller > 4 && Caller <= 7 )
	{
		MenuMin = 5;
		MenuMax = 7;
		MenuBreadcrumb = _TM_Breadcrumb;
	}
	else if(Caller > 10 && Caller <= 13 )
	{
		MenuMin = 11;
		MenuMax = 13;
		MenuBreadcrumb = _OM_Breadcrumb;
	}
	else if(Caller > 13 && Caller <= 19 )
	{
		MenuMin = 14;
		MenuMax = 19;
		MenuBreadcrumb = _OSM_Breadcrumb;
	}
	else if(Caller > 20 && Caller <= 24 )
	{
		MenuMin = 21;
		MenuMax = 24;
		MenuBreadcrumb = _SETUP_Breadcrumb;
	}
	else if(Caller >= 25 && Caller <= 26 )
	{
		MenuMin = 25;
		MenuMax = 26;
		MenuBreadcrumb = _TIMING_Breadcrumb;
	}
	else if(Caller >= 27 && Caller <= 28 )
	{
		MenuMin = 27;
		MenuMax = 28;
		MenuBreadcrumb = _DIMMING_Breadcrumb;
	}
	else if(Caller >= 29 && Caller <= 30 )
	{
		MenuMin = 29;
		MenuMax = 30;
		MenuBreadcrumb = _LOCATION_Breadcrumb;
	}

	else if(Caller >= 31 && Caller <= 32 )
	{
		MenuMin = 31;
		MenuMax = 32;
		MenuBreadcrumb = _OVERRIDE_Breadcrumb;
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
	uint8_t ReadData;
	//TODO: Initalize timeouts here...
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


	//TODO:Check if a timeout vaule of zero is allowed

	/**Initialize the idle to dim timeout
	 * This is the time that the idle screen will stay bright before dimming with no user input.
	 * This value is saved in EEPROM, and set to a default of IDLE_TO_DIM_TIME (3 sec) if the EEPROM value is zero
	 */
	if( EEPROM_Read(EEPROM_ADDRESS_IDLE_TO_DIM_TIMEOUT, &ReadData, 1 ) != 0)
	{
		App_Die(8);
	}

	if(ReadData <= 0)
	{
		IdleToDimTime = IDLE_TO_DIM_TIME;
	}
	else
	{
		IdleToDimTime = ReadData;
	}

	/**Initalize the menu to idle timeout
	 * This is the time the task will wait in a menu for the user to press a button before reverting to the idle screen.
	 * This value is saved in EEPROM, and set to a default of MENU_TO_IDLE_TIME (30 sec) if the EEPROM value is zero
	 */
	if( EEPROM_Read(EEPROM_ADDRESS_MENU_TO_IDLE_TIMEOUT, &ReadData, 1 ) != 0)
	{
		App_Die(8);
	}

	if(ReadData <= 5)	//TODO: Look at this more, this timeout should be limited to a reasonable value so that the menu screen does not timeout too quickly to change settings
	{
		MenuToIdleTime = MENU_TO_IDLE_TIME;
	}
	else
	{
		MenuToIdleTime = ReadData;
	}

	//Initalize the override timeout value
	//TODO: Should this be done somewhere else? Maybe put this in the timer task and override menus?
	//TODO: Maybe this should be in minutes?
	/*if( EEPROM_Read(EEPROM_ADDRESS_OVERRIDE_TIMEOUT, &ReadData, 1 ) != 0)
	{
		App_Die(8);
	}

	if(ReadData <= 0)
	{
		OverrideTime = OVERRIDE_TIME;
	}
	else
	{
		OverrideTime = ReadData;
	}*/

	//uint8_t BrightDimmingValue;// = 4;
	//uint8_t DimDimmingValue;// = 0;

	/**Initalize the bright dimming value
	 * This is the brightness of the OLED when in a menu or when the user pushes a button
	 * This value is saved in EEPROM, and should be 0-5
	 */
	if( EEPROM_Read(EEPROM_ADDRESS_BRIGHT_DIMMING_VAL, &ReadData, 1 ) != 0)
	{
		App_Die(8);
	}

	if((ReadData < 0) || (ReadData >5))
	{
		BrightDimmingValue = 4;		//TODO: Make this compiler define?
	}
	else
	{
		BrightDimmingValue = ReadData;
	}

	/**Initalize the bright dimming value
	 * This is the brightness of the OLED when in a no user input has been entered for a while
	 * This value is saved in EEPROM, and should be 0-5
	 */
	if( EEPROM_Read(EEPROM_ADDRESS_DIM_DIMMING_VAL, &ReadData, 1 ) != 0)
	{
		App_Die(8);
	}

	if((ReadData < 0) || (ReadData >6))
	{
		DimDimmingValue = 0;		//TODO: Make this compiler define?
	}
	else
	{
		DimDimmingValue = ReadData;
	}





	//BrightDimmingValue;// = 4;
	//uint8_t DimDimmingValue;


	DisplayDimming(BrightDimmingValue);

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
	StringOptions.TopPadding = 2;
	StringOptions.BottomPadding = 1;
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

	LineOptions.XStart = 0;
	LineOptions.XEnd = 255;
	LineOptions.YStart = 12;
	LineOptions.YEnd = 12;
	OLED_WriteLine2(&LineOptions);


	//OLED_ClearWindow(0, 64, 0, 11);

	//StringOptions.XStart = 8;
	//StringOptions.YStart = 0;
	//OLED_WriteMFString2("Help:", &StringOptions);

	return;
}


void DrawStatusScreen(void)
{
	//char StringToOutput[11];

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
	OLED_WriteMFString2("Status", &StringOptions);

	LineOptions.XStart = 0;
	LineOptions.XEnd = 255;
	LineOptions.YStart = 53;
	LineOptions.YEnd = 53;
	OLED_WriteLine2(&LineOptions);

	//Write the current time and date:
	StringOptions.XStart = 8;
	StringOptions.YStart = 39;
	OLED_WriteMFString2("Time:", &StringOptions);

	DS3232M_GetTimeString(ScratchString, 0);
	StringOptions.XStart = 45;
	OLED_WriteMFString2(ScratchString, &StringOptions);

	DS3232M_GetDateString(ScratchString, 0);
	StringOptions.XStart = 85;
	OLED_WriteMFString2(ScratchString, &StringOptions);

	//Write the sunrise time
	StringOptions.XStart = 8;
	StringOptions.YStart = 28;
	OLED_WriteMFString2("Sunrise", &StringOptions);

	GetSunriseTime(&CurrentTime);
	StringOptions.XStart = 64;
	strftime(ScratchString, 8, "%I:%M %p", &CurrentTime);
	ScratchString[8] = '\0';
	//sprintf(ScratchString, "%02u:%02u AM", CurrentTime.tm_hour, CurrentTime.tm_min);
	OLED_WriteMFString2(ScratchString, &StringOptions);

	GetAltSunriseTime(&CurrentTime);
	StringOptions.XStart = 150;
	strftime(ScratchString, 8, "%I:%M %p", &CurrentTime);
	ScratchString[8] = '\0';
	//sprintf(ScratchString, "%02u:%02u AM", CurrentTime.tm_hour, CurrentTime.tm_min);
	OLED_WriteMFString2(ScratchString, &StringOptions);

	//Write the sunset time
	StringOptions.XStart = 8;
	StringOptions.YStart = 17;
	OLED_WriteMFString2("Sunset", &StringOptions);

	GetSunsetTime(&CurrentTime);
	StringOptions.XStart = 64;
	strftime(ScratchString, 8, "%I:%M %p", &CurrentTime);
	ScratchString[8] = '\0';
	//sprintf(ScratchString, "%02u:%02u PM", CurrentTime.tm_hour, CurrentTime.tm_min);
	OLED_WriteMFString2(ScratchString, &StringOptions);

	GetAltSunsetTime(&CurrentTime);
	StringOptions.XStart = 150;
	strftime(ScratchString, 8, "%I:%M %p", &CurrentTime);
	ScratchString[8] = '\0';
	//sprintf(ScratchString, "%02u:%02u AM", CurrentTime.tm_hour, CurrentTime.tm_min);
	OLED_WriteMFString2(ScratchString, &StringOptions);



	//LineOptions.XStart = 0;
	//LineOptions.XEnd = 255;
	//LineOptions.YStart = 12;
	//LineOptions.YEnd = 12;
	//OLED_WriteLine2(&LineOptions);


	//OLED_ClearWindow(0, 64, 0, 11);

	//StringOptions.XStart = 8;
	//StringOptions.YStart = 0;
	//OLED_WriteMFString2("Help:", &StringOptions);

	return;
}

void DisplayShowMessage(char *MessageToDisplay)
{
	MF_StringOptions StringOptions;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 0;
	StringOptions.EndPadding = 0;
	StringOptions.TopPadding = 0;
	StringOptions.BottomPadding = 0;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.XStart = (IDLE_STATUS_COLUMN)*4;
	StringOptions.YStart = IDLE_STATUS_ROW;
	StringOptions.FontOptions = OLED_FONT_NORMAL;

	OLED_WriteMFString2(MessageToDisplay, &StringOptions);
	DisplayDimming(BrightDimmingValue);

	DisplayStatus = DISPLAY_STATUS_MESSAGE;

	return;
}

//Updates the display to reflect the state of the outputs and the mode
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
	StringOptions.FontOptions = OLED_FONT_NORMAL;
	switch(OutputState)
	{
		case TIMER_STATUS_OFF:
			//TODO: replace the spaces in this string with padding.
			OLED_WriteMFString2("Off     ", &StringOptions);
			break;

		case TIMER_STATUS_ON:
			//TODO: replace the spaces in this string with padding.
			OLED_WriteMFString2("Auto    ", &StringOptions);
			break;

		case TIMER_STATUS_PAUSED:
			OLED_WriteMFString2("Override", &StringOptions);
			break;
	}

	return;
}

/*uint8_t GetTimeout(uint8_t TimeoutToGet)
{
	//TODO: Make TimeoutToGet defines match the addresses so I don't need the switch statement.
	uint8_t AddressToRead;

	switch(TimeoutToGet)
	{
		case TIMEOUT_TYPE_IDLE_TO_DIM:
			AddressToRead = EEPROM_ADDRESS_IDLE_TO_DIM_TIMEOUT;
			break;

		case TIMEOUT_TYPE_MENU_TO_IDLE:
			AddressToRead = EEPROM_ADDRESS_MENU_TO_IDLE_TIMEOUT;
			break;

		case TIMEOUT_TYPE_OVERRIDE:
			AddressToRead = EEPROM_ADDRESS_OVERRIDE_TIMEOUT;
			break;

		default:
			return;
	}

	if( EEPROM_Read(EEPROM_ADDRESS_DISPLAY_STATUS, &StatusReg, 1 ) != 0)
		{
			App_Die(8);
		}







}*/

void SetTimeout(uint8_t TimeoutToSet, uint8_t TimeoutVal)
{
	uint8_t TimeoutAddress;

	switch(TimeoutToSet)
	{
		case TIMEOUT_TYPE_IDLE_TO_DIM:
			IdleToDimTime = TimeoutVal;
			TimeoutAddress = EEPROM_ADDRESS_IDLE_TO_DIM_TIMEOUT;
			break;

		case TIMEOUT_TYPE_MENU_TO_IDLE:
			MenuToIdleTime = TimeoutVal;
			TimeoutAddress = EEPROM_ADDRESS_MENU_TO_IDLE_TIMEOUT;
			break;

		/*case TIMEOUT_TYPE_OVERRIDE:	//TODO: This may not be needed.
			OverrideTime = TimeoutVal;
			TimeoutAddress = EEPROM_ADDRESS_OVERRIDE_TIMEOUT;
			break;*/

		default:
			return;
	}

	//Save new value in EEPROM
	if( EEPROM_Write(TimeoutAddress, &TimeoutVal, 1 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}

	return;
}

//TODO: Make a dimming value that corresponds to the screen being off. DO not let the user set the menu dimming to this value
void SetDimming(uint8_t DimmingType, uint8_t DimmingVal)
{
	uint8_t DimmingAddress;

	switch(DimmingType)
	{
		case DIMMING_TYPE_BRIGHT:
			BrightDimmingValue = DimmingVal;
			DimmingAddress = EEPROM_ADDRESS_BRIGHT_DIMMING_VAL;
			break;

		case DIMMING_TYPE_DIM:
			DimDimmingValue = DimmingVal;
			DimmingAddress = EEPROM_ADDRESS_DIM_DIMMING_VAL;
			break;

		default:
			return;
	}

	//Save new value in EEPROM
	if( EEPROM_Write(DimmingAddress, &DimmingVal, 1 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}

	return;
}


void DisplayDimming(uint8_t DimmingVal)
{
	if(DimmingVal <= 5)
	{
		OLED_Sleep(0);
		OLED_DisplayContrast(DimmingVaules[DimmingVal]);
	}
	else
	{
		OLED_Sleep(1);
	}
	return;
}


/* OLED display thread */
void DisplayTask(void *pvParameters)
{
	DisplayCommand CommandToExecute;
	MF_StringOptions StringOptions;
	uint8_t HWStatus;

	StringOptions.CharSize = MF_ASCII_SIZE_7X8;
	StringOptions.StartPadding = 0;
	StringOptions.EndPadding = 0;
	StringOptions.TopPadding = 0;
	StringOptions.BottomPadding = 0;
	StringOptions.CharacterSpacing = 0;
	StringOptions.Brightness = 0x0F;
	StringOptions.FontOptions = OLED_FONT_NORMAL;



	DisplayStatus = DISPLAY_STATUS_IDLE_BRIGHT;
	portBASE_TYPE QueueStatus = pdFALSE;

	static char InvalidString[] = "Invalid Command";


	StringOptions.XStart = 16*4;
	StringOptions.YStart = 28;

	HWStatus = App_GetStatus();

	switch(HWStatus)
	{
		case APP_STATUS_OK:
		OLED_WriteMFString2("POST Successful", &StringOptions);
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
		if((DisplayStatus == DISPLAY_STATUS_IDLE_BRIGHT) || (DisplayStatus == DISPLAY_STATUS_MESSAGE))
		{
			QueueStatus = xQueueReceive(xDisplayCommands, &CommandToExecute, ((portTickType)IdleToDimTime * 1000));
		}
		else if(DisplayStatus == DISPLAY_STATUS_IDLE_DIM)
		{
			QueueStatus = xQueueReceive(xDisplayCommands, &CommandToExecute, portMAX_DELAY);
		}
		else
		{
			QueueStatus = xQueueReceive(xDisplayCommands, &CommandToExecute, ((portTickType)MenuToIdleTime * 1000));
		}

		//If a command did not come in before the queue receive timed out, change the mode
		if(QueueStatus == pdFALSE)
		{
			if(DisplayStatus == DISPLAY_STATUS_IDLE_BRIGHT)
			{
				DisplayDimming(DimDimmingValue);
				DisplayStatus = DISPLAY_STATUS_IDLE_DIM;
			}
			else if(DisplayStatus == DISPLAY_STATUS_MESSAGE)
			{
				DisplayDimming(DimDimmingValue);
				DisplayStatus = DISPLAY_STATUS_IDLE_DIM;
				DrawIdleScreen();
				//TODO: This call makes the menu blink when the message is removed. I can probably keep this from happening by only updating the message string text. If I update this, I will need to make sure the string after "mode:" is padded on the end enough to clear out any old text.
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

					if((DisplayStatus == DISPLAY_STATUS_IDLE_DIM) && (DimDimmingValue == 6))
					{
						DisplayDimming(BrightDimmingValue);
						DisplayStatus = DISPLAY_STATUS_IDLE_BRIGHT;
					}
					else
					{

						//A button has been pressed. Figure out which one and call the button handler.
						if(CommandToExecute.CommandData[0] == 4)
						{
							//If the display is off, the first button press will wake up the display
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
					}

					break;

				case OLED_CMD_TIME_IN:
					UpdateSunriseAndSunset(0);
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
