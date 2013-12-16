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
const uint8_t NumCommands = 6;

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

/*
//Write a register to the ADC
static int _F6_Handler (void);
const char _F6_NAME[] PROGMEM 			= "adwrite";
const char _F6_DESCRIPTION[] PROGMEM 	= "write to a register on the ADC";
const char _F6_HELPTEXT[] PROGMEM 		= "adwrite <register> <data>";

//Set up the calibration for the internal temperature sensor
static int _F7_Handler (void);
const char _F7_NAME[] PROGMEM 			= "tempcal";
const char _F7_DESCRIPTION[] PROGMEM 	= "Calibrate the internal temperature sensor";
const char _F7_HELPTEXT[] PROGMEM 		= "'tempcal' has no parameters";

//Test the buzzer
static int _F8_Handler (void);
const char _F8_NAME[] PROGMEM 			= "beep";
const char _F8_DESCRIPTION[] PROGMEM 	= "Test the buzzer";
const char _F8_HELPTEXT[] PROGMEM 		= "beep <time>";

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
	/*
	{ _F6_NAME, 	2,  2,	_F6_Handler,	_F6_DESCRIPTION,	_F6_HELPTEXT	},		//adwrite	
	{ _F7_NAME, 	0,  0,	_F7_Handler,	_F7_DESCRIPTION,	_F7_HELPTEXT	},		//tempcal
	{ _F8_NAME,		1,  1,	_F8_Handler,	_F8_DESCRIPTION,	_F8_HELPTEXT	},		//beep
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

static int _F2_Handler (void)
{
	if (NumberOfArguments() == 0)
	{
		//Output some useful info
		printf("Main Clk: %u Hz\r\n", Chip_Clock_GetMainClockRate());
		printf("SSP0 Clk div: %u\r\n", Chip_Clock_GetSSP0ClockDiv());
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
					DS3232M_GetTemp(&temp, &temp2);
					printf("Temp: %u.%uC\r\n", temp, temp2);
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
	uint8_t temp;
	uint8_t DataToSend[34];
	uint8_t FontData[8];
	uint8_t NewFontData[4];
	int8_t i;

	char InitString[] = "Initialized";
	char BlargString[] = "Blarg";

	val = argAsInt(2);
	val2 = argAsInt(3);

	switch(argAsInt(1))
	{
		case 1:
			printf("reset(%u)\r\n", val);
			OLED_Reset(val);
			break;

		case 2:
			printf("OLED CS(%u)\r\n", val);
			OLED_Select(val);
			break;

		case 3:
			printf("MF CS(%u)\r\n", val);
			OLED_MF_Select(val);
			break;

		case 4:
			printf("OLED DC(%u)\r\n", val);
			OLED_DC(val);
			break;

		case 5:
			printf("Sending OLED init sequence...");
			DataToSend[0] = 0x12;
			OLED_SendCommand(OLED_COMMAND_LOCK, DataToSend, 1);

			//OLED_SendCommand(OLED_SET_DISPLAY_MODE_OFF, NULL, 0);

			OLED_SendCommand(OLED_SLEEP_MODE_ON, NULL, 0);

			DataToSend[0] = 0x1C;
			DataToSend[1] = 0x5B;
			OLED_SendCommand(OLED_SET_COLUMN_ADDRESS, DataToSend, 2);

			DataToSend[0] = 0x00;
			DataToSend[1] = 0x3F;
			OLED_SendCommand(OLED_SET_ROW_ADDRESS, DataToSend, 2);

			DataToSend[0] = 0x91;
			OLED_SendCommand(OLED_SET_CLK_DIVIDER, DataToSend, 1);

			DataToSend[0] = 0x3F;
			OLED_SendCommand(OLED_SET_MUX_RATIO, DataToSend, 1);

			DataToSend[0] = 0x00;
			OLED_SendCommand(OLED_SET_DISPLAY_OFFSET, DataToSend, 1);

			DataToSend[0] = 0x00;
			OLED_SendCommand(OLED_SET_DISPLAY_START_LINE, DataToSend, 1);

			DataToSend[0] = 0x16;
			DataToSend[1] = 0x11;
			OLED_SendCommand(OLED_SET_REMAP, DataToSend, 2);

			DataToSend[0] = 0x00;
			OLED_SendCommand(OLED_SET_GPIO, DataToSend, 1);

			DataToSend[0] = 0x01;
			OLED_SendCommand(OLED_FUNCTION_SELECTION, DataToSend, 1);

			DataToSend[0] = 0xA0;
			DataToSend[0] = 0xFD;
			OLED_SendCommand(OLED_DISPLAY_ENHANCE_A, DataToSend, 2);

			DataToSend[0] = 0x9F;
			OLED_SendCommand(OLED_SET_CONTRAST_CURRENT, DataToSend, 1);

			DataToSend[0] = 0x0F;
			OLED_SendCommand(OLED_MASTER_CONTRAST_CONTROL, DataToSend, 1);

			OLED_SendCommand(OLED_SELECT_DEFAULT_TABLE, NULL, 0);

			DataToSend[0] = 0xE2;
			OLED_SendCommand(OLED_SET_PHASE_LENGTH, DataToSend, 1);

			DataToSend[0] = 0xA2;
			DataToSend[1] = 0x20;
			OLED_SendCommand(OLED_DISPLAY_ENHANCE_B, DataToSend, 1);

			DataToSend[0] = 0x1F;
			OLED_SendCommand(OLED_SET_PRECHARGE_VOLTAGE, DataToSend, 1);

			DataToSend[0] = 0x08;
			OLED_SendCommand(OLED_SET_SECOND_PRECHARGE, DataToSend, 1);

			DataToSend[0] = 0x07;
			OLED_SendCommand(OLED_SET_VCOMH, DataToSend, 1);

			OLED_SendCommand(OLED_SET_DISPLAY_MODE_NORMAL, NULL, 0);

			OLED_SendCommand(OLED_EXIT_PARTIAL_DISPLAY, NULL, 0);

			OLED_SendCommand(OLED_SLEEP_MODE_OFF, NULL, 0);


			printf("Done!\r\n");
			break;

		case 6:
			printf("Fill Display with 0x%02X\r\n", val);
			OLED_FillDisplay(&val, 1);

			//OLED_ClearDisplay();
			/*DataToSend[0] =	0xF0;
			DataToSend[1] =	0xF0;
			DataToSend[2] =	0xF0;
			DataToSend[3] =	0xF0;
			DataToSend[4] =	0xF0;
			OLED_SendCommand(OLED_WRITE_RAM, DataToSend, 5);*/
			break;

		case 7:
			DataToSend[0] = 0x1C;
			DataToSend[1] = 0x1D;
			OLED_SendCommand(OLED_SET_COLUMN_ADDRESS, DataToSend, 2);

			DataToSend[0] = 0x00;
			DataToSend[1] = 0x07;
			OLED_SendCommand(OLED_SET_ROW_ADDRESS, DataToSend, 2);

			DataToSend[0] =	0xFF;
			DataToSend[1] =	0xFF;
			DataToSend[2] =	0xFF;
			DataToSend[3] =	0xFF;
			DataToSend[4] =	0xFF;

			//OLED_SendCommand(OLED_WRITE_RAM, DataToSend, 2);

			DataToSend[0] =	0x00;
			OLED_SendCommand(OLED_WRITE_RAM, DataToSend, 4);
			//vTaskDelay(configTICK_RATE_HZ/2);

			DataToSend[0] =	0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, DataToSend, 4);
			//vTaskDelay(configTICK_RATE_HZ/2);

			DataToSend[0] =	0xF0;
			OLED_SendCommand(OLED_WRITE_RAM, DataToSend, 4);
			//vTaskDelay(configTICK_RATE_HZ/2);

			DataToSend[0] =	0xFF;
			OLED_SendCommand(OLED_WRITE_RAM, DataToSend, 4);
			//vTaskDelay(configTICK_RATE_HZ/2);

			//for(temp=0;temp<20;temp++)
			//{
			//	OLED_SendCommand(OLED_WRITE_RAM, DataToSend, 2);
			//	vTaskDelay(configTICK_RATE_HZ/2);
			//}
			break;


		case 8:
			OLED_ClearDisplay();
			OLED_WriteMFString(MF_ASCII_SIZE_5X7, InitString, val, val2);
			OLED_WriteMFString(MF_ASCII_SIZE_7X8, InitString, val, val2+8);
			OLED_WriteMFString(MF_ASCII_SIZE_8X16, InitString, val, val2+16);
			OLED_WriteMFString(MF_ASCII_SIZE_WA, InitString, val, val2+32);
			break;

		case 9:
			OLED_ClearDisplay();
			break;

		case 10:
			OLED_WriteMFString_WA(MF_ASCII_SIZE_WA, InitString, val, val2);
			//OLED_ClearDisplay();
			//OLED_WriteMFChar(MF_ASCII_SIZE_8X16, 'G', val, val2);
//			MF_GetAsciiChar(MF_ASCII_SIZE_WA, 'G', DataToSend);
//
//			printf("whole: ");
//			for(i=0;i<34;i++)
//			{
//				printf("0x%02X ", DataToSend[i]);
//			}
//			printf("\r\n");
//
//			i=18;
//			MF_GetAsciiChar_4B(MF_ASCII_SIZE_WA, 'G', i, DataToSend);
//			printf("part[%u]: ",i);
//			for(i=0;i<4;i++)
//			{
//				printf("0x%02X ", DataToSend[i]);
//			}
//			printf("\r\n");
//
//			i=6;
//			MF_GetAsciiChar_4B(MF_ASCII_SIZE_WA, 'G', i, DataToSend);
//			printf("part[%u]: ",i);
//			for(i=0;i<4;i++)
//			{
//				printf("0x%02X ", DataToSend[i]);
//			}
//			printf("\r\n");
//
//			i=12;
//			MF_GetAsciiChar_4B(MF_ASCII_SIZE_WA, 'G', i, DataToSend);
//			printf("part[%u]: ",i);
//			for(i=0;i<4;i++)
//			{
//				printf("0x%02X ", DataToSend[i]);
//			}
//			printf("\r\n");
//
//			i=22;
//			MF_GetAsciiChar_4B(MF_ASCII_SIZE_WA, 'G', i, DataToSend);
//			printf("part[%u]: ",i);
//			for(i=0;i<4;i++)
//			{
//				printf("0x%02X ", DataToSend[i]);
//			}
//			printf("\r\n");



			break;

		case 11:

			MF_GetAsciiChar(MF_ASCII_SIZE_WA, 'I', DataToSend);
			printf("I: ");
			for(i=0;i<34;i++)
			{
				printf("0x%02X ", DataToSend[i]);
			}
			printf("\r\n");

			MF_GetAsciiChar(MF_ASCII_SIZE_WA, 'n', DataToSend);
			printf("n: ");
			for(i=0;i<34;i++)
			{
				printf("0x%02X ", DataToSend[i]);
			}
			printf("\r\n");

			MF_GetAsciiChar(MF_ASCII_SIZE_WA, 'i', DataToSend);
			printf("i: ");
			for(i=0;i<34;i++)
			{
				printf("0x%02X ", DataToSend[i]);
			}
			printf("\r\n");

			MF_GetAsciiChar(MF_ASCII_SIZE_WA, 't', DataToSend);
			printf("t: ");
			for(i=0;i<34;i++)
			{
				printf("0x%02X ", DataToSend[i]);
			}
			printf("\r\n");

			break;

		case 12:
			OLED_WriteMFString_Q(MF_ASCII_SIZE_WA, BlargString, 5, 10);
			//ReinvokeISP();
			break;

		case 13:
			OLED_WriteMF_UInt(MF_ASCII_SIZE_7X8, val, 5, 10);
			break;

		case 14:
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





/** @} */
