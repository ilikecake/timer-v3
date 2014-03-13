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
*	\brief		Hardware driver for the DS3232M RTC chip.
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

#include "main.h"


#ifdef INCLUDE_DOW_STRINGS

//The names of the days of the week.
static char DOW0[] = "Sunday";
static char DOW1[] = "Monday";
static char DOW2[] = "Tuesday";
static char DOW3[] = "Wednesday";
static char DOW4[] = "Thursday";
static char DOW5[] = "Friday";
static char DOW6[] = "Saturday";

char* DOW[7] = {DOW0, DOW1, DOW2, DOW3, DOW4, DOW5, DOW6};

#endif



//TODO: Add I2C status checking
//TODO: Implement a mutex for the i2c?
//TODO: Write some sort of writereg function to abstract the I2C stuff from the rest of the code. (done?)
//TODO: Convert this code to use UTC and offset for timezones/DST (No)

I2C_XFER_T DS3232M_I2C;

#define GPIO_PININT					28 						/* GPIO pin number mapped to PININT */
#define GPIO_PININT_PORT			1						/* GPIO port number mapped to PININT */
#define GPIO_PININT_INDEX			0						/* PININT index used for GPIO mapping */
#define PININT_IRQ_HANDLER			FLEX_INT0_IRQHandler	/* PININT IRQ function name */
#define PININT_NVIC_NAME			PIN_INT0_IRQn			/* PININT NVIC interrupt name */

#define YEAR_MSB_SRAM_ADDRESS		0xFE	/** The address to put the two most significant bits in the year register. (ex: for 1982, 19 would go into this register.) */
#define USE_DST_ADDRESS				0xFD	/** The address to put the use DST flags. A 1 in the first bit indicates that DST should be used, a 1 in the 2nd bit indicated that DST is active. */
#define DST_OFFSET_ADDRESS			0xFC	/** The address to put the DST offset. This amount will always be added to the reported time. If no DST is desired, this should be set to zero. */
#define UT_OFFSET_ADDRESS			0xFB	/** The offset (in hours) that should be added to the Universal Time to get the local time. The offset will be added to the time when it is retrieved from the device, hence, for CST, this number should be -6 */


//Hardware specific functions

//Handler for the alarm interrupt
void PININT_IRQ_HANDLER(void)
{

	if(App_GetStatus() != APP_STATUS_INIT)
	{
		DisplayCommand CommandToSend;
		uint8_t TimerCommand = TIMER_TASK_CMD_TICK;
		Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_PININT_INDEX));
		DS3232M_ClearAlarmFlag(2);
		CommandToSend.CommandName = OLED_CMD_TIME_IN;
		xQueueSendFromISR(xTimerCommands, (void *)&TimerCommand, NULL);
		xQueueSendFromISR(xDisplayCommands, (void *)&CommandToSend, NULL);

		//Board_LED_Set(3, 1);
	}
	return;
}

//Reads the value from a single register
uint8_t DS3232M_ReadReg(uint8_t Reg, uint8_t* Data)
{
	uint8_t SendData;

	SendData = Reg;

	DS3232M_I2C.txBuff = &SendData;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxBuff = Data;
	DS3232M_I2C.rxSz = 1;
	return (Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C));
}

//Writes data to one register. Will overwrite the contents of the register
uint8_t DS3232M_WriteReg(uint8_t Reg, uint8_t Data)
{
	uint8_t SendData[2];

	SendData[0] = Reg;
	SendData[1] = Data;
	DS3232M_I2C.txSz = 2;
	DS3232M_I2C.rxSz = 0;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = NULL;
	return (Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C));
}

//Modify the contents of a single register. Only the bits marked with a 1 in the Bitmask register will be set based on their values in the Value register.
uint8_t DS3232M_ModifyReg(uint8_t Reg, uint8_t Value, uint8_t Bitmask)
{
	uint8_t SendData[2];
	uint8_t ReceiveData;
	uint8_t stat;

	//Get the current register
	SendData[0] = Reg;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxSz = 1;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = &ReceiveData;
	stat = Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);
	if(stat != I2C_STATUS_DONE) return stat;

	//Write the new value to the register
	SendData[0] = Reg;
	SendData[1] = (ReceiveData & (~Bitmask)) | (Value & Bitmask);
	DS3232M_I2C.txSz = 2;
	DS3232M_I2C.rxSz = 0;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = NULL;
	return Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);
}

/**Writes an arbitrary number of bytes to the DS3232M.
 *  DataToWrite: A pointer to the array of data to write.
 *  			 Note: Position zero in this array should be the address to write the data
 *
 *  BytesToWrite: The number of bytes to write (including the address byte).
 */
uint8_t DS3232M_WriteBytes(uint8_t* DataToWrite, uint8_t BytesToWrite)
{
	DS3232M_I2C.txSz = BytesToWrite;
	DS3232M_I2C.rxSz = 0;
	DS3232M_I2C.txBuff = DataToWrite;
	DS3232M_I2C.rxBuff = NULL;
	return (Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C));
}

/**Read an arbitrary number of bytes from the DS3232M starting at AddressToRead */
uint8_t DS3232M_ReadBytes(uint8_t AddressToRead, uint8_t* DataToRead, uint8_t BytesToRead)
{
	uint8_t addr = AddressToRead;

	DS3232M_I2C.txBuff = &addr;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxBuff = DataToRead;
	DS3232M_I2C.rxSz = BytesToRead;
	return Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);
}

//Initalize the DS3232M
uint8_t DS3232M_Init( void )
{
	uint8_t SendData[2];
	struct tm AlarmTime;
	//TimeAndDate AlarmTime;

	//Set up pins for 32kHz, int, and reset
	// - RTC-INT:	P1.28	(Should be pulled up, a low on this pin indicates an interrupt)
	// - RTC-32kHz:	P1.31	(Push-pull output from the RTC, outputs a 32.768kHz square wave, low when unused)
	// - RTC-RST:	P0.07  	(This is an output from the device. It indicates that the voltage has fallen below a certain threshold. The RTC has an internal pullup on this pin)
	//Set all pins as GPIO inputs
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 7, IOCON_FUNC0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 28, IOCON_FUNC0|IOCON_MODE_PULLUP);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 31, IOCON_FUNC0);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 7);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 28);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 31);

	/* Enable PININT clock */
	//TODO: Probably put this in the main initalization...
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_PINT);

	/* Configure interrupt channel for the GPIO pin in SysCon block */
	Chip_SYSCTL_SetPinInterrupt(GPIO_PININT_INDEX, GPIO_PININT_PORT, GPIO_PININT);

	/* Configure channel interrupt as edge sensitive and falling edge interrupt */
	Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(GPIO_PININT_INDEX));
	Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(GPIO_PININT_INDEX));

	NVIC_ClearPendingIRQ(PININT_NVIC_NAME);
	NVIC_EnableIRQ(PININT_NVIC_NAME);


	DS3232M_I2C.slaveAddr = DS3232M_SLA_ADDRESS;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.txSz = 2;
	DS3232M_I2C.rxSz = 0;

	//Set up control register
	// -Enable interrupt on INT/SQW pin, disable square wave output
	// -Disable pending interrupts
	SendData[0] = DS3232M_REG_CONTROL;
	SendData[1] = 0x04;

	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	DS3232M_32KhzStop();

	DS3232M_DisableAlarm(1);

	AlarmTime.tm_mday = 0;
	AlarmTime.tm_wday = 0;
	AlarmTime.tm_hour = 0;
	AlarmTime.tm_min = 0;
	AlarmTime.tm_mon = 0;
	AlarmTime.tm_sec = 0;
	AlarmTime.tm_year = 0;
	DS3232M_SetAlarm(2, 0x0E, &AlarmTime);
	DS3232M_EnableAlarm(2);

	//Returns 1 if the OSC has stopped, zero otherwise.
	return DS3232M_GetOSCFlag();
}



//Universal functions
void DS3232M_Reset(void)
{
	//Sending 0x80 to the test register resets all registers to their default states.
	DS3232M_WriteReg(DS3232M_REG_TEST, 0x80);
	return;
}

void DS3232M_GetStatus(uint8_t *status, uint8_t *control)
{
	DS3232M_ReadReg(DS3232M_REG_CONTROL, control);
	DS3232M_ReadReg(DS3232M_REG_STATUS, status);
	return;
}

uint8_t DS3232M_GetOSCFlag(void)
{
	uint8_t StatusReg;
	DS3232M_ReadReg(DS3232M_REG_STATUS, &StatusReg);
	return ((StatusReg & 0x80) >> 7);
}

void DS3232M_ClearOSCFlag(void)
{
	//Clear the MSB of the status register
	DS3232M_ModifyReg(DS3232M_REG_STATUS, 0x00, 0x80);
	return;
}


/** Set the time
 *
 * Note: it is assumed that the time is entered in local time (including DST, if applicable)
 */
void DS3232M_SetTime(struct tm * TheTime)
{
	uint8_t SendData[8];
	uint16_t YearMSD;
	uint16_t YearLSD;
	uint8_t AlarmState;

	AlarmState = DS3232M_AlarmsActive();
	DS3232M_DisableAlarm(1);
	DS3232M_DisableAlarm(2);

	//DOW is set automatically based on the date entered.
	mktime(TheTime);	//Fill the day of the year and day of the week values
	//TheTime->dow = GetDOW(TheTime->year, (uint16_t)(TheTime->month), (uint16_t)(TheTime->day));

	SendData[0] = DS3232M_REG_SEC;
	SendData[1] = ((TheTime->tm_sec % 10) | ((TheTime->tm_sec / 10) << 4));
	SendData[2] = ((TheTime->tm_min % 10) | ((TheTime->tm_min / 10) << 4));
	SendData[3] = ((((TheTime->tm_hour % 10) | ((TheTime->tm_hour / 10) << 4))) & 0x3F);				//Note: this sets the RTC in 24 hour mode
	SendData[4] = TheTime->tm_wday;
	SendData[5] = ((TheTime->tm_mday % 10) | ((TheTime->tm_mday / 10) << 4));
	SendData[6] = ((((TheTime->tm_mon+1) % 10) | (((TheTime->tm_mon+1) / 10) << 4))) & 0x7F;				//NOTE: This clears the century bit. The two most significant digits of the year are saved in SRAM. When the date is read, the century bit is checked and the century is updated if required.

	YearMSD = TheTime->tm_year/100;
	YearLSD = TheTime->tm_year - (YearMSD*100);

	SendData[7] = ((YearLSD % 10) | ((YearLSD / 10) << 4));

	DS3232M_WriteBytes(SendData, 8);

	//Write the two most significant digits of the year (ex: 19 for 1982, 20 for 2013, etc...) to SRAM
	 DS3232M_WriteReg(YEAR_MSB_SRAM_ADDRESS, (uint8_t)(YearMSD));

	//Clear the osc flag to indicate that the time is valid
	DS3232M_ClearOSCFlag();

	//Update the DST bits
	//We assume that if the user entered a new time, they already applied the DST correction if required.
	//Here we will check if the clock is using DST and if so, update the DST bit to indicate that DST has already been applied
	DS3232M_ReadReg(USE_DST_ADDRESS, &SendData[0]);		//Read the DST config bits into array 0.
	if((SendData[0] & 0x01) == 0x01)
	{
		//The clock is using DST
		SendData[0] = (IsDSTDate(TheTime) << 1) | 0x01;
		DS3232M_WriteReg(USE_DST_ADDRESS, SendData[0]);
	}

	//Reenable alarms
	if((AlarmState & 0x01) == 0x01)
	{
		DS3232M_EnableAlarm(1);
	}
	if((AlarmState & 0x02) == 0x02)
	{
		DS3232M_EnableAlarm(2);
	}
	return;
}

//void DS3232M_GetTime(TimeAndDate *TheTime)
void DS3232M_GetTime(struct tm * timeptr)
{
	uint8_t RecieveData[7];
	//uint8_t SendData;

	DS3232M_ReadBytes(DS3232M_REG_SEC, RecieveData, 7);

	//SendData = DS3232M_REG_SEC;

	//DS3232M_I2C.txBuff = &SendData;
	//DS3232M_I2C.txSz = 1;
	//DS3232M_I2C.rxBuff = RecieveData;
	//DS3232M_I2C.rxSz = 7;
	//Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	//Convert registers in BCD into the time struct
	//TheTime->sec = ((RecieveData[0] & 0x0F) + ((RecieveData[0] & 0x70) >> 4)*10 );
	//TheTime->min = ((RecieveData[1] & 0x0F) + ((RecieveData[1] & 0x70) >> 4)*10 );
	timeptr->tm_sec = ((RecieveData[0] & 0x0F) + ((RecieveData[0] & 0x70) >> 4)*10 );
	timeptr->tm_min = ((RecieveData[1] & 0x0F) + ((RecieveData[1] & 0x70) >> 4)*10 );

	//Handle hours in 12 or 24 hour mode. The struct will always take 24 hour format
	if( (RecieveData[2] & 0x40) == 0x04)		//12 hour mode
	{
		if((RecieveData[2] & 0x20) == 0x20)		//PM
		{
			//TheTime->hour = 12 + (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
			timeptr->tm_hour = 12 + (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
		}
		else
		{
			//TheTime->hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
			timeptr->tm_hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
		}
	}
	else	//24 Hour Mode
	{
		//TheTime->hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x30) >> 4)*10;
		timeptr->tm_hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x30) >> 4)*10;
	}

	//TheTime->dow = (RecieveData[3] & 0x07);
	timeptr->tm_wday = (RecieveData[3] & 0x07);

	//TheTime->day = ((RecieveData[4] & 0x0F) + ((RecieveData[4] & 0x30) >> 4)*10);
	timeptr->tm_mday = ((RecieveData[4] & 0x0F) + ((RecieveData[4] & 0x30) >> 4)*10);

	//TheTime->month = ((RecieveData[5] & 0x0F) + ((RecieveData[5] & 0x10) >> 4)*10);
	timeptr->tm_mon = ((RecieveData[5] & 0x0F) + ((RecieveData[5] & 0x10) >> 4)*10) - 1;

	//Assemble the year
	//We switch to the tm struct from the standard C library. For this struct we change the meaning of the year register to the years since 1900 to match the standard definition
	//The YEAR_MSB_ADDRESS is now the number of centuries since 1900, for 2014, the value should be 1

	//TheTime->year = ((RecieveData[6] & 0x0F) + ((RecieveData[6] & 0xF0) >> 4)*10);	//The two LSD of the year
	timeptr->tm_year = ((RecieveData[6] & 0x0F) + ((RecieveData[6] & 0xF0) >> 4)*10);

	//If the century bit is one, the year most significant digits are incremented by 1. The century bit is then set back to zero.
	DS3232M_ReadReg(YEAR_MSB_SRAM_ADDRESS, &RecieveData[1]);

	if((RecieveData[5] & 0x7F) == 0x80)
	{
		//Increment the year MSD in SRAM and clear the century bit
		RecieveData[0] = YEAR_MSB_SRAM_ADDRESS;
		RecieveData[1] += 1;
		DS3232M_WriteReg(YEAR_MSB_SRAM_ADDRESS, RecieveData[1]);
		//WriteSRAM(RecieveData, 1);
		DS3232M_ModifyReg(DS3232M_REG_MONTH, 0, 0x80);		//Clear the century bit
		//ClearCenturyBit();
	}

	//TheTime->year += (100*RecieveData[1]);
	timeptr->tm_year += (100*RecieveData[1]);

	//Handle DST if required

	DS3232M_ReadReg(USE_DST_ADDRESS, &RecieveData[0]);		//Read the DST config bits into array 0.
	RecieveData[1] = IsDSTDate(timeptr);

	timeptr->tm_isdst = IsDSTDate(timeptr);

	/**NOTE: This will not work if this attempts to correct the hours and causes the hours to overflow.
	 *       The most general form of this function would require the function to check hours, days, months, and years.
	 *       However, this function should work okay, as long as the GetTime function is not called at < 1 hour to midnight.
	 */
	if((RecieveData[0] & 0x01) == 0x01)
	{
		//DST should be applied
		if(((RecieveData[0] >> 1) & 0x01) != RecieveData[1])
		{
			//DST has not been applied
			if(RecieveData[1] == 0x01)
			{
				//It is now DST time and we have not added an hour
				timeptr->tm_hour += 1;
				//TheTime->hour += 1;										//Add one to the hours struct
				DS3232M_WriteReg(USE_DST_ADDRESS, 0x03);				//indicate that we have applied DST

			}
			else
			{
				//It is no longer DST, but DST is still applied;
				//TheTime->hour -= 1;										//SUbtract one from the hours struct
				timeptr->tm_hour -= 1;
				DS3232M_WriteReg(USE_DST_ADDRESS, 0x01);				//indicate that we have removed DST

			}
			//RecieveData[2] = ((((TheTime->hour % 10) | ((TheTime->hour / 10) << 4))) & 0x3F);	//Put the hours into BCD format.
			RecieveData[2] = ((((timeptr->tm_hour % 10) | ((timeptr->tm_hour / 10) << 4))) & 0x3F);	//Put the hours into BCD format.
			DS3232M_WriteReg(DS3232M_REG_HOUR, RecieveData[2]);									//Write hours back to the device
		}
	}

	//mktime(&timeptr);	//Fill the day of the year and day of the week values
	//TheTime->DST_Bit = RecieveData[0];

	return;
}

//AlarmMasks format: <DY/DT(1=day, 0=date)> <AxM4> <AxM3> <AxM2> <A1M1>
//Note: A2M1 does not exist (no seconds register)
//TODO: look at DOW register in this function
void DS3232M_SetAlarm(uint8_t AlarmNumber, uint8_t AlarmMasks, struct tm *AlarmTime)
{
	//uint8_t RecieveData;
	uint8_t SendData[5];

	//DS3232M_I2C.txBuff = SendData;
	//DS3232M_I2C.rxSz = 0;

	if(AlarmNumber == 1)
	{
		SendData[0] = DS3232M_REG_A1_SEC;
		SendData[1] = (((AlarmMasks & 0x01) << 7) | (AlarmTime->tm_sec % 10) | ((AlarmTime->tm_sec / 10) << 4));
		SendData[2] = (((AlarmMasks & 0x02) << 6) | (AlarmTime->tm_min % 10) | ((AlarmTime->tm_min / 10) << 4));
		SendData[3] = ((((AlarmMasks & 0x04) << 5) | (AlarmTime->tm_hour % 10) | ((AlarmTime->tm_hour / 10) << 4)) & 0xBF);

		if((AlarmMasks & 0x10) == 0x10)
		{
			//Alarm on day of week
			SendData[4] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->tm_wday));
		}
		else
		{
			//Alarm on date
			SendData[4] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->tm_mday % 10) | ((AlarmTime->tm_mday / 10) << 4));
		}
		DS3232M_WriteBytes(SendData, 5);
	}
	else
	{
		SendData[0] = DS3232M_REG_A2_MIN;
		SendData[1] = (((AlarmMasks & 0x02) << 6) | (AlarmTime->tm_min % 10) | ((AlarmTime->tm_min / 10) << 4));
		SendData[2] = ((((AlarmMasks & 0x04) << 5) | (AlarmTime->tm_hour % 10) | ((AlarmTime->tm_hour / 10) << 4)) & 0xBF);

		if((AlarmMasks & 0x10) == 0x10)
		{
			//Alarm on day of week
			SendData[3] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->tm_wday));
		}
		else
		{
			//Alarm on date
			SendData[3] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->tm_mday % 10) | ((AlarmTime->tm_mday / 10) << 4));
		}
		DS3232M_WriteBytes(SendData, 4);
	}
	return;
}

void DS3232M_EnableAlarm(uint8_t AlarmNumber)
{
	if((AlarmNumber == 1) || (AlarmNumber == 2))
	{
		DS3232M_ModifyReg(DS3232M_REG_CONTROL, AlarmNumber, AlarmNumber);
	}
	return;
}

void DS3232M_DisableAlarm(uint8_t AlarmNumber)
{
	if((AlarmNumber == 1) || (AlarmNumber == 2))
	{
		DS3232M_ModifyReg(DS3232M_REG_CONTROL, 0, AlarmNumber);
		DS3232M_ModifyReg(DS3232M_REG_STATUS, 0, AlarmNumber);
	}
	return;
}

uint8_t DS3232M_AlarmsActive(void)
{
	uint8_t ControlReg;

	DS3232M_ReadReg(DS3232M_REG_CONTROL, &ControlReg);
	return (ControlReg & 0x07);
}

void DS3232M_ClearAlarmFlag(uint8_t AlarmNumber)
{
	if((AlarmNumber == 1) || (AlarmNumber == 2))
	{
		DS3232M_ModifyReg(DS3232M_REG_STATUS, 0, AlarmNumber);
	}
	return;
}

void DS3232M_GetAlarmTime(uint8_t AlarmNumber, uint8_t *AlarmMasks, struct tm *AlarmTime)//TimeAndDate *AlarmTime)
{
	uint8_t RecieveData[4];

	//These will always be zero
	AlarmTime->tm_mon = 0;
	AlarmTime->tm_year = 0;

	if(AlarmNumber == 1)
	{
		DS3232M_ReadBytes(DS3232M_REG_A1_SEC, RecieveData, 4);
	}
	else if(AlarmNumber == 2)
	{
		/**Note: we read this register instead of the first alarm 2 register to align the data from alarm 2 with the data from alarm 1.
		 * 		 Because alarm 2 has no seconds register, the first byte of data in RecieveData for alarm 2 will not contain valid data.
		 * 		 The other three bytes should be parsed identically between the two alarms
		 */
		DS3232M_ReadBytes(DS3232M_REG_A1_DATE, RecieveData, 4);
	}
	else
	{
		return;
	}

	if(AlarmNumber == 1)
	{
		AlarmTime->tm_sec = ((RecieveData[0] & 0x0F) + ((RecieveData[0] & 0x70) >> 4)*10 );
	}
	else
	{
		AlarmTime->tm_sec = 0;
	}

	AlarmTime->tm_min = ((RecieveData[1] & 0x0F) + ((RecieveData[1] & 0x70) >> 4)*10 );

	//Handle hours in 12 or 24 hour mode. We will always return the hours in 24 hour format.
	if( (RecieveData[2] & 0x40) == 0x04)		//12 hour mode
	{
		if((RecieveData[2] & 0x20) == 0x20)		//PM
		{
			AlarmTime->tm_hour = 12 + (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
		}
		else	//AM
		{
			AlarmTime->tm_hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
		}
	}
	else	//24 Hour Mode
	{
		AlarmTime->tm_hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x30) >> 4)*10;
	}

	if((RecieveData[3] & 0x40) == 0x40)
	{
		//Alarm on day of the week
		AlarmTime->tm_wday = (RecieveData[3] & 0x07);
		AlarmTime->tm_mday = 0;
	}
	else
	{
		//Alarm on date of the month
		AlarmTime->tm_mday = ((RecieveData[3] & 0x0F) + ((RecieveData[3] & 0x30) >> 4)*10);
		AlarmTime->tm_wday = 0;
	}

	*AlarmMasks = ((RecieveData[0] & 0x80) >> 7) | ((RecieveData[1] & 0x80) >> 6) | ((RecieveData[2] & 0x80) >> 5) | ((RecieveData[3] & 0x80) >> 4) | ((RecieveData[3] & 0x40) >> 2);

	return;
}

void DS3232M_32KhzStart(uint8_t BatEnable)
{
	DS3232M_ModifyReg(DS3232M_REG_STATUS, (((BatEnable & 0x01) << 6) | 0x08), 0x48);
	return;
}

void DS3232M_32KhzStop(void)
{
	DS3232M_ModifyReg(DS3232M_REG_STATUS, 0, 0x48);
	return;
}

void DS3232M_GetTemp(int8_t *TempLHS, uint8_t *TempRHS)
{
	uint8_t RecieveData[2];

	DS3232M_ReadBytes(DS3232M_REG_TEMP_HI, RecieveData, 2);

	*TempLHS = RecieveData[0];				//LHS is in 2's compliment form
	*TempRHS = (RecieveData[1] >> 6)*25;	//The two MSB of this byte are the decimal portion of the temperature in .25s
	return;
}

//Time string should be 8 characters plus the terminating character
void DS3232M_GetTimeString(char *TimeString, uint8_t StringOptions)
{
	uint8_t i;
	struct tm CurrentTime;

	DS3232M_GetTime(&CurrentTime);

	strftime(TimeString, 8, "%I:%M %p", &CurrentTime);
	TimeString[8] = '\0';



	/*TimeString[5] = ' ';
	TimeString[8] = '\0';

	//Determine AM/PM
	if(CurrentTime.hour > 11)
	{
		TimeString[6] = 'P';
		TimeString[7] = 'M';
	}
	else
	{
		TimeString[6] = 'A';
		TimeString[7] = 'M';
	}

	//Format hours
	if(CurrentTime.hour > 12)
	{
		CurrentTime.hour -= 12;
	}
	else if(CurrentTime.hour == 0)
	{
		CurrentTime.hour = 12;
	}

	if(CurrentTime.hour > 9)
	{
		TimeString[0] = '1';
		CurrentTime.hour -= 10;
	}
	else
	{
		TimeString[0] = ' ';
	}
	TimeString[1] = (char)(CurrentTime.hour+48);
	TimeString[2] = ':';
	TimeString[3] = (char)((CurrentTime.min/10)+48);
	TimeString[4] = (char)((CurrentTime.min%10)+48);

	if(StringOptions == 1)
	{
		if(TimeString[0] == ' ')
		{
			for(i=0;i<8;i++)
			{
				TimeString[i] = TimeString[i+1];
			}
		}
	}*/

	return;
}

//Date string should be 10 characters plus the terminating character
void DS3232M_GetDateString(char *DateString, uint8_t StringOptions)
{
	struct tm CurrentTime;
	//TimeAndDate CurrentTime;
	uint16_t TempVar;

	DS3232M_GetTime(&CurrentTime);

	strftime(DateString, 10, "%m/%d/%Y", &CurrentTime);
	DateString[10] = '\0';
/*
	DateString[2] = '/';
	DateString[5] = '/';

	//DateString[6] = '2';
	//DateString[7] = '0';

	DateString[10] = '\0';

	if(CurrentTime.month >= 10)
	{
		DateString[0] = '1';
	}
	else
	{
		DateString[0] = ' ';
	}
	DateString[1] = (char)((CurrentTime.month%10)+48);

	DateString[3] = (char)((CurrentTime.day/10)+48);
	DateString[4] = (char)((CurrentTime.day%10)+48);


	TempVar = CurrentTime.year/100;
	DateString[6] = (char)((TempVar/10)+48);
	DateString[7] = (char)((TempVar%10)+48);

	TempVar = CurrentTime.year - (TempVar*100);
	DateString[8] = (char)((TempVar/10)+48);
	DateString[9] = (char)((TempVar%10)+48);
*/
	return;
}

/**Returns the day of the week for a given day, month and year
 *  Method from http://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Other_methods_.28using_tables_or_computational_devices.29
 *  Year:	The four digit year (ex: 1982)
 *  Month:	The month number (ex: 9 for September)
 *  Day:	The day in the month (ex: 14)
 *
 *  Returns: The day of the week, with 0 = Sunday, 1 = Monday, etc...
 */
uint8_t GetDOW(uint16_t Year, uint16_t Month, uint16_t Day)
{
	static uint16_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	Year -= Month < 3;
	return (Year + Year/4 - Year/100 + Year/400 + t[Month-1] + Day) % 7;
}

/**Set or clear the DST bits
 * This function does not change the time, it simply updates the DST bits in SRAM to indicate whether DST is in use.
 * The current time of the device should already be corrected for daylight savings (if required) before this function is called.
 */
void SetDST(uint8_t DST_Bit)
{
	//TimeAndDate TheTime;
	struct tm TheTime;

	DS3232M_GetTime(&TheTime);

	if(DST_Bit == 1)
	{
		if(IsDSTDate(&TheTime) == 0x01)
		{
			//DST time is requested, and DST is currently active
			DS3232M_WriteReg(USE_DST_ADDRESS, 0x03);
		}
		else
		{
			//DST time is requested, and DST is not currently active
			DS3232M_WriteReg(USE_DST_ADDRESS, 0x01);
		}
	}
	else
	{
		DS3232M_WriteReg(USE_DST_ADDRESS, 0x00);
	}

	return;
}

uint8_t GetDST(void)
{
	uint8_t DST_Bit;
	DS3232M_ReadReg(USE_DST_ADDRESS, &DST_Bit);
	return DST_Bit;
}

//There should be an easier way to do this...
/*void GetDSTStartAndEnd(TimeAndDate *TheTime, uint8_t* DSTStartDay, uint8_t* DSTEndDay)
{
	uint8_t TempDOW;

	TempDOW = GetDOW(TheTime->year, 3, 1);	//Get the day of the week for March 1st.
	//DST starts on the 2nd Sunday of March.
	if(TempDOW == 0)
	{
		*DSTStartDay = 8;
	}
	else
	{
		*DSTStartDay = (15 - TempDOW);
	}

	TempDOW = GetDOW(TheTime->year, 11, 1);	//Get the day of the week for November 1st.
	//DST ends on the 1st Sunday of November.
	if(TempDOW == 0)
	{
		*DSTEndDay = 1;
	}
	else
	{
		*DSTEndDay = (8 - TempDOW);
	}
	return;
}*/

//uint8_t IsDSTDate(TimeAndDate *TheTime)
uint8_t IsDSTDate(struct tm * timeptr)
{
	uint32_t SecFromStartOfMonth;
	uint32_t DST_SecFromStartOfMonth;
	uint8_t StartOfDSTDay;
	uint8_t EndOfDSTDay;

	time_t StartOfDST;
	time_t EndOfDST;
	time_t NowTime;

	NowTime = mktime(timeptr);
	GetDSTStartAndEnd2(timeptr->tm_year, &StartOfDST, &EndOfDST);

	if((NowTime > StartOfDST) && (NowTime < EndOfDST))
	{
		return 1;
	}
	return 0;


	//if(TheTime->month == 3)
	/*if(timeptr->tm_mon == 2)		//Note: tm_mon starts at 0 so tm_mon = 2 is march
	{
		GetDSTStartAndEnd(TheTime, &StartOfDSTDay, &EndOfDSTDay);
		DST_SecFromStartOfMonth = 7200 + (StartOfDSTDay-1)*86400;
		SecFromStartOfMonth = ((TheTime->day-1) * 86400) + (TheTime->hour * 3600) + (TheTime->min *60) + TheTime->sec;

		if(SecFromStartOfMonth > DST_SecFromStartOfMonth)
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
	}
	else if(TheTime->month == 11)
	{
		GetDSTStartAndEnd(TheTime, &StartOfDSTDay, &EndOfDSTDay);
		DST_SecFromStartOfMonth = 7200 + (EndOfDSTDay-1)*86400;
		SecFromStartOfMonth = ((TheTime->day-1) * 86400) + (TheTime->hour * 3600) + (TheTime->min *60) + TheTime->sec;

		if(SecFromStartOfMonth > DST_SecFromStartOfMonth)
		{
			return 0x00;
		}
		else
		{
			return 0x01;
		}
	}
	else if( (TheTime->month > 3) && (TheTime->month < 11) )
	{
		return 0x01;
	}
	else
	{
		return 0x00;
	}*/
}

void SetUTOffset (int8_t Offset)
{
	DS3232M_WriteReg(UT_OFFSET_ADDRESS, (uint8_t)Offset);
	return;
}

int8_t GetUTOffset(void)
{
	uint8_t UT_Offset;;
	DS3232M_ReadReg(UT_OFFSET_ADDRESS, &UT_Offset);
	return (int8_t)UT_Offset;
}

uint8_t DaysInTheMonth(uint8_t month, uint16_t year)
{
	if(month == 2)
	{
		if( ((year % 4) == 0) && (((year % 100) != 0) || (((year % 100) == 0) && ((year % 400) == 0))) )
		{
			return 29;
		}
		else
		{
			return 28;
		}
	}
	else if( (month == 4) || (month == 7) || (month == 9) || (month == 11) )
	{
		return 30;
	}
	else
	{
		return 31;
	}
}

/*uint8_t TimeAndDateCompare(TimeAndDate TimeAndDate1, TimeAndDate TimeAndDate2, uint8_t CompareList)
{
	if(TimeAndDate1.year > TimeAndDate2.year)
	{
		return 1;
	}
	else if(TimeAndDate1.year < TimeAndDate2.year)
	{
		return 2;
	}
	else
	{
		if(TimeAndDate1.month > TimeAndDate2.month)
		{
			return 1;
		}
		else if(TimeAndDate1.month < TimeAndDate2.month)
		{
			return 2;
		}
		else
		{
			if(TimeAndDate1.day > TimeAndDate2.day)
			{
				return 1;
			}
			else if(TimeAndDate1.day < TimeAndDate2.day)
			{
				return 2;
			}
			else
			{
				if(TimeAndDate1.hour > TimeAndDate2.hour)
				{
					return 1;
				}
				else if(TimeAndDate1.hour < TimeAndDate2.hour)
				{
					return 2;
				}
				else
				{
					if(TimeAndDate1.min > TimeAndDate2.min)
					{
						return 1;
					}
					else if(TimeAndDate1.min < TimeAndDate2.min)
					{
						return 2;
					}
					else
					{
						if(TimeAndDate1.sec > TimeAndDate2.sec)
						{
							return 1;
						}
						else if(TimeAndDate1.sec < TimeAndDate2.sec)
						{
							return 2;
						}
						else
						{
							return 0;
						}
					}
				}
			}
		}
	}
}*/

//year should be the number of years since 1900
void GetDSTStartAndEnd2(uint16_t year, time_t* DST_Start, time_t* DST_End)
{
	struct tm timevalue;

	//Set the time for 3/1/year 2:00:00
	timevalue.tm_hour	= 2;
	timevalue.tm_mday	= 1;
	timevalue.tm_min	= 0;
	timevalue.tm_sec	= 0;
	timevalue.tm_mon	= 2;		//March
	timevalue.tm_year	= year;

	//We call this function here to fill in the tm_wday value
	*DST_Start = mktime(&timevalue);

	if(timevalue.tm_wday == 0)
	{
		timevalue.tm_mday = 8;
	}
	else
	{
		timevalue.tm_mday = 15-timevalue.tm_wday;
	}
	*DST_Start = mktime(&timevalue);

	//Set the time for 11/1/year 2:00:00
	timevalue.tm_hour	= 2;
	timevalue.tm_mday	= 1;
	timevalue.tm_min	= 0;
	timevalue.tm_sec	= 0;
	timevalue.tm_mon	= 10;		//November
	timevalue.tm_year	= year;

	//We call this function here to fill in the tm_wday value
	*DST_End = mktime(&timevalue);

	if(timevalue.tm_wday == 0)
	{
		timevalue.tm_mday = 1;
	}
	else
	{
		timevalue.tm_mday = 8-timevalue.tm_wday;
	}
	*DST_End = mktime(&timevalue);

	return;
}


/** @} */
