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

extern char* DOW[7];

#endif

//Hardware specific function headers
uint8_t DS3232M_ReadReg(uint8_t Reg, uint8_t* Data);
uint8_t DS3232M_WriteReg(uint8_t Reg, uint8_t Data);
uint8_t DS3232M_ModifyReg(uint8_t Reg, uint8_t Value, uint8_t Bitmask);
uint8_t DS3232M_WriteBytes(uint8_t* DataToWrite, uint8_t BytesToWrite);
uint8_t DS3232M_ReadBytes(uint8_t AddressToRead, uint8_t* DataToRead, uint8_t BytesToRead);
uint8_t DS3232M_Init( void );

//Non-hardware specific function headers

/** Resets all registers on the DS3232M to their defaults states */
void DS3232M_Reset(void);

/** Gets the two status registers from the DS3232M
 *   *status:	A pointer to the status register
 *   *control:	A pointer to the control register
 */
void DS3232M_GetStatus(uint8_t *status, uint8_t *control);

/**Returns the Oscillator stop flag from the DS3232M.
 * This flag is 1 if the oscillator has stopped at some point in the past.
 * This flag should be used to determine whether the time stored in the device is valid.
 *
 * returns: The oscillator stop flag. 1 = oscillator has stopped, 0 = oscillator has not stopped
 */
uint8_t DS3232M_GetOSCFlag(void);

/**Clears the oscillator stop flag on the DS3232M.
 * This flag should be cleared when writing the time to the register.
 */
void DS3232M_ClearOSCFlag(void);

void DS3232M_SetTime(struct tm * TheTime);

void DS3232M_GetTime(struct tm * timeptr);

//TODO: Combine the time and date string functions
//TODO: Remove these functions?
void DS3232M_GetTimeString(char *TimeString, uint8_t StringOptions);
void DS3232M_GetDateString(char *TimeString, uint8_t StringOptions);

void DS3232M_SetAlarm(uint8_t AlarmNumber, uint8_t AlarmMasks, struct tm *AlarmTime);
void DS3232M_EnableAlarm(uint8_t AlarmNumber);
void DS3232M_DisableAlarm(uint8_t AlarmNumber);

//returns the alarm time for the chosen alarm number
void DS3232M_GetAlarmTime(uint8_t AlarmNumber, uint8_t *AlarmMasks, struct tm *AlarmTime);

void DS3232M_ClearAlarmFlag(uint8_t AlarmNumber);

//Returns a bitmask indicating which (if any) alarms are active
uint8_t DS3232M_AlarmsActive(void);

void DS3232M_32KhzStart(uint8_t BatEnable);
void DS3232M_32KhzStop(void);

void DS3232M_SetIntMode(uint8_t IntMode);													//not implemented

void DS3232M_GetTemp(int8_t *TempLHS, uint8_t *TempRHS);

uint8_t GetDOW(uint16_t Year, uint16_t Month, uint16_t Day);

//Functions to deal with time zones
void SetUTOffset (int8_t Offset);
int8_t GetUTOffset(void);

//Functions to deal with DST
void SetDST(uint8_t DST_Bit);
uint8_t GetDST(void);

void GetDSTStartAndEnd(uint16_t year, time_t* DST_Start, time_t* DST_End);

//Returns 1 if TheTime is a DST date, 0 otherwise.
uint8_t IsDSTDate(struct tm * timeptr);

uint8_t DaysInTheMonth(uint8_t month, uint16_t year);

#endif

/** @} */
