/*   This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** \file
*	\brief		Command interpreter application specific functions
*	\author		Pat Satyshur
*	\version	1.1
*	\date		1/13/2013
*	\copyright	Copyright 2013, Pat Satyshur
*	\ingroup 	beer_heater_main
*
*	@{
*/

#include "main.h"
//#include "commands.h"
//#include <stdio.h>
//#include "board.h"

//The number of commands
const uint8_t NumCommands = 8;

//Handler function declerations

//LED control function
static int _F1_Handler (void);
const char _F1_NAME[] 			= "led";
const char _F1_DESCRIPTION[] 	= "Turn LED on or off";
const char _F1_HELPTEXT[] 		= "led <number>";

//DAC commands
static int _F2_Handler (void);
const char _F2_NAME[] 			= "cpu";
const char _F2_DESCRIPTION[] 	= "cpu commands";
const char _F2_HELPTEXT[] 		= "cpu <1> <2>";


//Get time from RTC
static int _F3_Handler (void);
const char _F3_NAME[]  			= "i2cscan";
const char _F3_DESCRIPTION[]  	= "Scan the I2C Bus";
const char _F3_HELPTEXT[]  		= "'i2cscan' has no parameters";


//Set time on RTC
static int _F4_Handler (void);
const char _F4_NAME[]  			= "time";
const char _F4_DESCRIPTION[]  	= "Get time from the RTC";
const char _F4_HELPTEXT[]  		= "Prints the current time";


//OLED functions
static int _F5_Handler (void);
const char _F5_NAME[] 			= "oled";
const char _F5_DESCRIPTION[] 	= "OLED functions";
const char _F5_HELPTEXT[]		= "'oled' has no parameters";

//Buzzer functions
static int _F6_Handler (void);
const char _F6_NAME[] 			= "beep";
const char _F6_DESCRIPTION[] 	= "Buzzer Functions";
const char _F6_HELPTEXT[]		= "beep <time>";

//EEPROM functions
static int _F7_Handler (void);
const char _F7_NAME[]			= "eeprom";
const char _F7_DESCRIPTION[]	= "EEPROM functions";
const char _F7_HELPTEXT[] 		= "eeprom <1> <2>";

//Timer functions
static int _F8_Handler (void);
const char _F8_NAME[]			= "timer";
const char _F8_DESCRIPTION[] 	= "Timer functions";
const char _F8_HELPTEXT[] 		= "timer <1>";
/*
//Turn the relay on or off
static int _F9_Handler (void);
const char _F9_NAME[] PROGMEM 			= "relay";
const char _F9_DESCRIPTION[] PROGMEM 	= "Control the relay";
const char _F9_HELPTEXT[] PROGMEM 		= "relay <state>";

//Manual calibration of the ADC
static int _F10_Handler (void);
const char _F10_NAME[] PROGMEM 			= "cal";
const char _F10_DESCRIPTION[] PROGMEM 	= "Calibrate the ADC";
const char _F10_HELPTEXT[] PROGMEM 		= "'cal' has no parameters";

//Get temperatures from the ADC
static int _F11_Handler (void);
const char _F11_NAME[] PROGMEM 			= "temp";
const char _F11_DESCRIPTION[] PROGMEM 	= "Get temperatures from the ADC";
const char _F11_HELPTEXT[] PROGMEM 		= "'temp' has no parameters";

//Scan the TWI bus for devices
static int _F12_Handler (void);
const char _F12_NAME[] PROGMEM 			= "twiscan";
const char _F12_DESCRIPTION[] PROGMEM 	= "Scan for TWI devices";
const char _F12_HELPTEXT[] PROGMEM 		= "'twiscan' has no parameters";

//Dataflash functions
static int _F13_Handler (void);
const char _F13_NAME[] PROGMEM 			= "mem";
const char _F13_DESCRIPTION[] PROGMEM 	= "dataflash functions";
const char _F13_HELPTEXT[] PROGMEM 		= "mem <1> <2> <3>";
*/

//Command list
const CommandListItem AppCommandList[] =
{
	{ _F1_NAME,		1,  2,	_F1_Handler,	_F1_DESCRIPTION,	_F1_HELPTEXT	},		//led
	{ _F2_NAME, 	0,  2,	_F2_Handler,	_F2_DESCRIPTION,	_F2_HELPTEXT	},		//cpu
	{ _F3_NAME, 	0,  0,	_F3_Handler,	_F3_DESCRIPTION,	_F3_HELPTEXT	},		//i2cscan
	{ _F4_NAME, 	0,  7,	_F4_Handler,	_F4_DESCRIPTION,	_F4_HELPTEXT	},		//time
	{ _F5_NAME, 	1,  3,	_F5_Handler,	_F5_DESCRIPTION,	_F5_HELPTEXT	},		//oled
	{ _F6_NAME, 	0,  1,	_F6_Handler,	_F6_DESCRIPTION,	_F6_HELPTEXT	},		//beep
	{ _F7_NAME, 	2,  2,	_F7_Handler,	_F7_DESCRIPTION,	_F7_HELPTEXT	},		//eeprom
	{ _F8_NAME,		1,  1,	_F8_Handler,	_F8_DESCRIPTION,	_F8_HELPTEXT	},		//timer
	/*
	{ _F9_NAME,		1,  1,	_F9_Handler,	_F9_DESCRIPTION,	_F9_HELPTEXT	},		//relay
	{ _F10_NAME,	0,  0,	_F10_Handler,	_F10_DESCRIPTION,	_F10_HELPTEXT	},		//cal
	{ _F11_NAME,	0,  0,	_F11_Handler,	_F11_DESCRIPTION,	_F11_HELPTEXT	},		//temp
	{ _F12_NAME,	0,  0,	_F12_Handler,	_F12_DESCRIPTION,	_F12_HELPTEXT	},		//twiscan
	{ _F13_NAME,	1,  3,	_F13_Handler,	_F13_DESCRIPTION,	_F13_HELPTEXT	},		//twiscan
	*/
};

//Command functions

//LED control function
static int _F1_Handler (void)
{
	if(NumberOfArguments() == 2)
	{
		Board_LED_Set(argAsInt(1), argAsInt(2));
	}
	else
	{
		if(argAsInt(1) == 1)
		{
			//Turn on all leds
			Board_LED_Set(1, 1);
			Board_LED_Set(2, 1);
			Board_LED_Set(3, 1);
		}
		else
		{
			//Turn off all leds
			Board_LED_Set(1, 0);
			Board_LED_Set(2, 0);
			Board_LED_Set(3, 0);
		}

	}
	return 0;
}

//cpu
static int _F2_Handler (void)
{
	uint32_t tempVal[4];
	uint8_t resp;

	if (NumberOfArguments() == 0)
	{
		//Output some useful info
		printf("Main Clk: %u Hz\r\n", Chip_Clock_GetMainClockRate());
		printf("SSP0 Clk div: %u\r\n", Chip_Clock_GetSSP0ClockDiv());
		printf("Main Clk: %u Hz\r\n", SystemCoreClock);

		resp = ReadUID(tempVal);
		if(resp == IAP_CMD_SUCCESS)
		{
			printf("UID: 0x%08lX %08lX %08lX %08lX\r\n", tempVal[0], tempVal[1], tempVal[2], tempVal[3]);
		}

		resp = ReadPartID(tempVal);
		if(resp == IAP_CMD_SUCCESS)
		{
			printf("Part ID: 0x%08lX\r\n", tempVal[0]);
		}

		resp = ReadBootVersion(tempVal);
		if(resp == IAP_CMD_SUCCESS)
		{
			printf("Boot Code Version: %u.%u\r\n", (uint8_t)((tempVal[0]>>8)&0xFF), (uint8_t)(tempVal[0]&0xFF));
		}

		printf("----------------------------\r\n");
		printf("Task Name\tStack Usage\r\n");
		printf("----------------------------\r\n");
		printf("vTaskLed1\t%u/64\r\n", 64-uxTaskGetStackHighWaterMark(TaskList[0]));
		printf("vConsole\t%u/400\r\n", 400-uxTaskGetStackHighWaterMark(TaskList[1]));
		printf("vOLEDTask\t%u/300\r\n", 300-uxTaskGetStackHighWaterMark(TaskList[2]));
		printf("vTimer\t\t%u/64\r\n", 64-uxTaskGetStackHighWaterMark(TaskList[3]));
		printf("----------------------------\r\n");

	}

	return 0;
}


//i2cscan
static int _F3_Handler (void)
{
	i2c_probe_slaves(I2C0);
	return 0;
}

//Timer functions
static int _F4_Handler (void)
{
	TimeAndDate CurrentTime;
	uint8_t temp;
	uint8_t temp2;
	char TimeString[9];

	switch(NumberOfArguments())
	{
		case 1:
			switch(argAsInt(1))
			{
				case 0:
					DS3232M_GetStatus(&temp, &temp2);
					printf("Control: 0x%02X\r\n", temp2);
					printf("Status: 0x%02X\r\n", temp);

					temp = DS3232M_AlarmsActive();
					if((temp & 0x04) == 0x04)
					{
						printf("Alarms Enabled\r\n");
					}
					if((temp & 0x02) == 0x02)
					{
						printf("Alarm 2 on\r\n");
					}
					else
					{
						printf("Alarm 2 off\r\n");
					}
					if((temp & 0x01) == 0x01)
					{
						printf("Alarm 1 on\r\n");
					}
					else
					{
						printf("Alarm 1 off\r\n");
					}

					temp = DS3232M_GetOSCFlag();
					printf("osc: %u\r\n", temp);

					if(temp == 1)
					{
						printf("OSC has stopped\r\n");
					}
					else
					{
						printf("OSC has not stopped\r\n");
					}
					break;

				case 1:
				case 2:
					printf("Set Alarm\r\n");

					//Set the alarm for 1 min from now and enable
					DS3232M_GetTime(&CurrentTime);
					printf("Current Time: %02u:%02u:%02u %02u/%02u/20%02u \r\n", CurrentTime.hour, CurrentTime.min, CurrentTime.sec, CurrentTime.month, CurrentTime.day, CurrentTime.year);
					CurrentTime.min += 1;
					printf("Alarm Time: %02u:%02u:%02u %02u/%02u/20%02u \r\n", CurrentTime.hour, CurrentTime.min, CurrentTime.sec, CurrentTime.month, CurrentTime.day, CurrentTime.year);
					DS3232M_SetAlarm(argAsInt(1), 0x00, &CurrentTime);
					DS3232M_EnableAlarm(argAsInt(1));
					printf("Alarm %u set\r\n", argAsInt(1));
					break;

				case 3:
				case 4:
					printf("Disable Alarm %u\r\n", argAsInt(1)-2);
					DS3232M_DisableAlarm(argAsInt(1)-2);
					break;

				case 5:
				case 6:
					printf("Get Alarm %u\r\n", argAsInt(1)-4);
					DS3232M_GetAlarmTime(argAsInt(1)-4, &temp, &CurrentTime);
					printf("Alarm set to: %02u:%02u:%02u %02u/%02u/20%02u \r\n", CurrentTime.hour, CurrentTime.min, CurrentTime.sec, CurrentTime.month, CurrentTime.day, CurrentTime.year);
					printf("Alarm Mask: 0x%02X\r\n", temp);
					break;

				case 7:
					DS3232M_ClearOSCFlag();
					printf("OSC Flag cleared\r\n");
					break;

				case 8:
				case 9:
					DS3232M_ClearAlarmFlag(argAsInt(1)-7);
					printf("Alarm flag %u cleared\r\n", argAsInt(1)-7);
					break;

				case 10:
					DS3232M_32KhzStart(0);
					break;

				case 11:
					DS3232M_32KhzStop();
					break;

				case 12:
					DS3232M_Reset();
					break;

				case 13:
					DS3232M_GetTemp((int8_t *)&temp, &temp2);
					printf("Temp: %u.%uC\r\n", temp, temp2);
					break;

				case 14:
					DS3232M_GetTimeString(TimeString, 0);
					printf("out\r\n");
					printf(TimeString);
					printf("\r\n");
					break;

			}
			break;

		case 2:
			printf("Set Alarm\r\n");

			//Set the alarm for 1 min from now and enable
			DS3232M_GetTime(&CurrentTime);
			printf("Current Time: %02u:%02u:%02u %02u/%02u/20%02u \r\n", CurrentTime.hour, CurrentTime.min, CurrentTime.sec, CurrentTime.month, CurrentTime.day, CurrentTime.year);
			CurrentTime.min += 1;
			printf("Alarm Time: %02u:%02u:%02u %02u/%02u/20%02u \r\n", CurrentTime.hour, CurrentTime.min, CurrentTime.sec, CurrentTime.month, CurrentTime.day, CurrentTime.year);
			DS3232M_SetAlarm(argAsInt(1), argAsInt(2), &CurrentTime);
			DS3232M_EnableAlarm(argAsInt(1));
			printf("Alarm %u set\r\n", argAsInt(1));
			break;

		case 6:
		case 7:
			//Set the time
			CurrentTime.day		= argAsInt(4);
			CurrentTime.month	= argAsInt(5);
			CurrentTime.year	= argAsInt(6);
			CurrentTime.hour	= argAsInt(1);
			CurrentTime.min		= argAsInt(2);
			CurrentTime.sec		= argAsInt(3);
			if(NumberOfArguments() > 6)
			{
				CurrentTime.dow	= argAsInt(7);
			}
			 DS3232M_SetTime(&CurrentTime);
			 printf("Time set to: %02u:%02u:%02u %02u/%02u/20%02u \r\n", CurrentTime.hour, CurrentTime.min, CurrentTime.sec, CurrentTime.month, CurrentTime.day, CurrentTime.year);
			 break;

		default:
			DS3232M_GetTime(&CurrentTime);
			printf("Current Time:\r\n");
			printf("%02u:%02u:%02u %02u/%02u/20%02u \r\n", CurrentTime.hour, CurrentTime.min, CurrentTime.sec, CurrentTime.month, CurrentTime.day, CurrentTime.year);
	}

	return 0;
}

//OLED functions
static int _F5_Handler (void)
{
	uint32_t val;
	uint8_t val2;

	char InitString[] = "Initialized";

	val = argAsInt(2);
	val2 = argAsInt(3);

	switch(argAsInt(1))
	{
		case 0:
			OLED_ClearDisplay();
			break;

		case 1:
			OLED_DisplayContrast(val);
			break;

		case 2:
			printf("Fill Display with 0x%02X\r\n", val);
			OLED_FillDisplay((uint8_t *)&val, 1);
			break;

		case 3:
			OLED_ClearDisplay();
			OLED_WriteMFString(MF_ASCII_SIZE_5X7, InitString, val, val2);
			OLED_WriteMFString(MF_ASCII_SIZE_7X8, InitString, val, val2+8);
			OLED_WriteMFString(MF_ASCII_SIZE_8X16, InitString, val, val2+16);
			OLED_WriteMFString(MF_ASCII_SIZE_WA, InitString, val, val2+32);
			break;

		case 4:
			OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, InitString, val, val2, OLED_FONT_NORMAL);
			break;

		case 5:
			OLED_WriteMFString_Q(MF_ASCII_SIZE_WA, InitString, 5, 10);
			break;

		case 6:
			OLED_WriteMF_UInt(MF_ASCII_SIZE_7X8, val, 5, 10);
			break;

		case 7:
			 OLED_DisplayRotation(val);

	}

	return 0;
}



//Buzzer Functions
static int _F6_Handler (void)
{
	uint8_t cmd;
	cmd = argAsInt(1);

	if(cmd == 1)
	{
		printf("Beep on\r\n");
		App_Buzzer_on();
	}
	else if(cmd == 0)
	{
		printf("Beep off\r\n");
		App_Buzzer_off();
	}

	return 0;
}

//EEPROM Functions
static int _F7_Handler (void)
{
	uint8_t cmd;
	uint8_t val;
	uint8_t resp;
	uint32_t ver;
	uint8_t i;
	unsigned int temp3[4];
	cmd = argAsInt(1);
	val = argAsInt(2);

	uint8_t DataArray[5];

	DataArray[0] = 0;
	DataArray[1] = 0;
	DataArray[2] = 0;
	DataArray[3] = 0;
	DataArray[4] = 0;

	switch(cmd)
	{
	case 0:
		ver = EELIB_getVersion();
		printf("Version: %ul\r\n", ver);
		break;

	case 1:
		printf("Reading 4 bytes from address %u\r\n", val);
		resp = EEPROM_Read(val, DataArray, 4);
		printf("Response is %u\r\n", resp);
		printf("data[0]: %u\r\n", DataArray[0]);
		printf("data[1]: %u\r\n", DataArray[1]);
		printf("data[2]: %u\r\n", DataArray[2]);
		printf("data[3]: %u\r\n", DataArray[3]);
		//printf("data[4]: %u\r\n", DataArray[4]);
		break;

	case 2:
		DataArray[0] = 0xFA;
		DataArray[1] = 0xFF;
		DataArray[2] = 0x10;
		DataArray[3] = 0xFA;
		printf("writing 4 bytes starting at %u\r\n", val);
		resp = EEPROM_Write(val, DataArray, 4);
		printf("Response is %u\r\n", resp);
		break;

	case 3:
		resp = ReadUID(temp3);
		if(resp == IAP_CMD_SUCCESS)
		{
			printf("UID: 0x%08lX %08lX %08lX %08lX\r\n", temp3[0], temp3[1], temp3[2], temp3[3]);
		}
		break;

	case 4:
		resp = ReadPartID(&ver);
		if(resp == IAP_CMD_SUCCESS)
		{
			printf("PID: 0x%08lX\r\n", ver);
		}
		break;

	case 5:
		resp = ReadBootVersion(&ver);
		if(resp == IAP_CMD_SUCCESS)
		{
			printf("BV: %u.%u\r\n", (uint8_t)((ver>>8)&0xFF), (uint8_t)(ver&0xFF));
		}
		break;

	case 6:
		printf("Calling reinvoke ISP...\r\n");
		ReinvokeISP();
		break;




	}

	return 0;
}

//Timer Functions
static int _F8_Handler (void)
{
	uint8_t cmd;
	cmd = argAsInt(1);
	TimerEvent NewTimerEvent;
	TimeAndDate CurrentTime;


	DS3232M_GetTime(&CurrentTime);

	printf("Start Out: 0x%02X\r\n", TimerGetOutputState());

	switch (cmd)
	{
		case 1:
			//TimerTask();
			printf("Out: 0x%02X\r\n", TimerGetOutputState());
			break;

		case 2:
			NewTimerEvent.EventType = TIMER_TASK_TYPE_TIME_EVENT;
			NewTimerEvent.EventTime[0] = 0xFF;	//Trigger on all days of week
			NewTimerEvent.EventTime[1] = CurrentTime.hour;
			NewTimerEvent.EventTime[2] = CurrentTime.min+1;
			NewTimerEvent.EventOutputState = 1;
			TimerSetEvent(1, 1, &NewTimerEvent);

			NewTimerEvent.EventTime[2] = CurrentTime.min+2;
			NewTimerEvent.EventOutputState = 0;
			TimerSetEvent(1, 2, &NewTimerEvent);

			break;

		case 3:
			TimerGetEvent(1, 1, &NewTimerEvent);
			printf("type: %u\r\n", NewTimerEvent.EventType);
			printf("time[0]: %u\r\n", NewTimerEvent.EventTime[0]);
			printf("time[1]: %u\r\n", NewTimerEvent.EventTime[1]);
			printf("time[2]: %u\r\n", NewTimerEvent.EventTime[2]);
			printf("state: %u\r\n", NewTimerEvent.EventOutputState);
			break;


	}

	return 0;
}

/** @} */
