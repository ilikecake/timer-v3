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
*	\brief		Hardware driver header for the DS3232M RTC chip.
*	\author		Pat Satyshur
*	\version	1.0
*	\date		11/14/2013
*	\copyright	Copyright 2013, Pat Satyshur
*	\ingroup 	hardware
*
*	\defgroup	hardware Hardware Drivers
*
*	@{
*/

#ifndef _DS3232M_H_
#define _DS3232M_H_

#include "stdint.h"
//#include "common_types.h"

#define INCLUDE_DOW_STRINGS

#define DS3232M_SLA_ADDRESS		0x68

//Register map
#define DS3232M_REG_SEC			0x00
#define DS3232M_REG_MIN			0x01
#define DS3232M_REG_HOUR		0x02
#define DS3232M_REG_DAY			0x03
#define DS3232M_REG_MONTH		0x05
#define DS3232M_REG_YEAR		0x06

#define DS3232M_REG_A1_SEC		0x07
#define DS3232M_REG_A1_MIN		0x08
#define DS3232M_REG_A1_HOUR		0x09
#define DS3232M_REG_A1_DATE		0x0A

#define DS3232M_REG_A2_MIN		0x0B
#define DS3232M_REG_A2_HOUR		0x0C
#define DS3232M_REG_A2_DATE		0x0D

#define DS3232M_REG_CONTROL		0x0E
#define DS3232M_REG_STATUS		0x0F
#define DS3232M_REG_AGING		0x10
#define DS3232M_REG_TEMP_HI		0x11
#define DS3232M_REG_TEMP_LO		0x12
#define DS3232M_REG_TEST		0x13

#define DS3232M_INT_MODE_INT		0x00
#define DS3232M_INT_MODE_1KHZ		0x01
#define DS3232M_INT_MODE_1KHZ_BBU	0x02

#ifdef INCLUDE_DOW_STRINGS

//The names of the days of the week.
static char DOW0[] = "Sunday";
static char DOW1[] = "Monday";
static char DOW2[] = "Tuesday";
static char DOW3[] = "Wednesday";
static char DOW4[] = "Thursday";
static char DOW5[] = "Friday";
static char DOW6[] = "Saturday";

static char* DOW[7] = {DOW0, DOW1, DOW2, DOW3, DOW4, DOW5, DOW6};

#endif

//TODO: This should probably go somewhere else...
//TODO: Is there a standard struct I can use for this?
typedef struct TimeAndDate
{
        uint16_t year;		/** The full year (ex: include all digits, like 1982, 2014, etc...) */
        uint8_t month;
        uint8_t day;
        uint8_t dow;
        uint8_t hour;
        uint8_t min;
        uint8_t sec;
} TimeAndDate;

//TODO: Abstract the TWI send function?
//TODO: Add a register modify function

uint8_t DS3232M_Init( void );																//Working

void DS3232M_GetStatus(uint8_t *status, uint8_t *control);									//Done

uint8_t DS3232M_GetOSCFlag(void);															//Done
void DS3232M_ClearOSCFlag(void);															//Done


void DS3232M_SetTime(TimeAndDate *TheTime);													//Done
void DS3232M_GetTime(TimeAndDate *TheTime);													//Done

//TODO: Combine the time and date string functions
void DS3232M_GetTimeString(char *TimeString, uint8_t StringOptions);
void DS3232M_GetDateString(char *TimeString, uint8_t StringOptions);

void DS3232M_SetAlarm(uint8_t AlarmNumber, uint8_t AlarmMasks, TimeAndDate *AlarmTime);		//Done
void DS3232M_EnableAlarm(uint8_t AlarmNumber);												//Done
void DS3232M_DisableAlarm(uint8_t AlarmNumber);												//Done
//returns the alarm time for the chosen alarm number
void DS3232M_GetAlarmTime(uint8_t AlarmNumber, uint8_t *AlarmMasks, TimeAndDate *AlarmTime);//Done
void DS3232M_ClearAlarmFlag(uint8_t AlarmNumber);											//Done
//Returns a bitmask indicating which (if any) alarms are active
uint8_t DS3232M_AlarmsActive(void);															//Done

void DS3232M_32KhzStart(uint8_t BatEnable);													//Done
void DS3232M_32KhzStop(void);																//Done

void DS3232M_SetIntMode(uint8_t IntMode);													//not implemented

uint8_t DS3232M_GetTemp(int8_t *TempLHS, uint8_t *TempRHS);									//Not tested

void DS3232M_Reset(void);																	//Needs work

uint8_t GetDOW(uint16_t Year, uint16_t Month, uint16_t Day);

void WriteSRAM(uint8_t* DataToWrite, uint8_t BytesToWrite);
void ReadSRAM(uint8_t AddressToRead, uint8_t* DataToRead, uint8_t BytesToRead);

void ClearCenturyBit(void);

#endif

/** @} */
