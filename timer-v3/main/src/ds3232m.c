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

//TODO: Add I2C status checking
//TODO: Implement a mutex for the i2c?

I2C_XFER_T DS3232M_I2C;

#define GPIO_PININT					28 						/* GPIO pin number mapped to PININT */
#define GPIO_PININT_PORT			1						/* GPIO port number mapped to PININT */
#define GPIO_PININT_INDEX			0						/* PININT index used for GPIO mapping */
#define PININT_IRQ_HANDLER			FLEX_INT0_IRQHandler	/* PININT IRQ function name */
#define PININT_NVIC_NAME			PIN_INT0_IRQn			/* PININT NVIC interrupt name */

void PININT_IRQ_HANDLER(void)
{

	if(App_GetStatus() != APP_STATUS_INIT)
	{
		OLED_Command CommandToSend;
		Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(GPIO_PININT_INDEX));
		DS3232M_ClearAlarmFlag(2);
		CommandToSend.CommandName = OLED_CMD_TIME_IN;
		xQueueSendFromISR(xOLEDCommands, (void *)&CommandToSend, NULL);

		//Board_LED_Set(3, 1);
	}
	return;
}






//Initalize the DS3232M
uint8_t DS3232M_Init( void )
{
	uint8_t SendData[2];
	TimeAndDate AlarmTime;

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

	AlarmTime.day = 0;
	AlarmTime.dow = 0;
	AlarmTime.hour = 0;
	AlarmTime.min = 0;
	AlarmTime.month = 0;
	AlarmTime.sec = 0;
	AlarmTime.year = 0;
	DS3232M_SetAlarm(2, 0x0E, &AlarmTime);
	DS3232M_EnableAlarm(2);

	//DS3232M_DisableAlarm(2);
	return 0x00;
}

void DS3232M_Reset(void)
{
	uint8_t SendData[2];

	DS3232M_I2C.txSz = 2;
	DS3232M_I2C.rxSz = 0;
	DS3232M_I2C.txBuff = SendData;

	SendData[0] = DS3232M_REG_TEST;
	SendData[1] = 0x80;

	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	return;
}

void DS3232M_GetStatus(uint8_t *status, uint8_t *control)
{
	uint8_t RecieveData[2];
	uint8_t SendData = DS3232M_REG_CONTROL;

	DS3232M_I2C.txBuff = &SendData;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxBuff = RecieveData;
	DS3232M_I2C.rxSz = 2;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	*control = RecieveData[0];
	*status = RecieveData[1];
	return;
}

uint8_t DS3232M_GetOSCFlag(void)
{
	uint8_t StatusReg;
	uint8_t ControlReg;
	DS3232M_GetStatus(&StatusReg, &ControlReg);

	return ((StatusReg & 0x80) >> 7);
}

void DS3232M_ClearOSCFlag(void)
{
	uint8_t RecieveData;
	uint8_t SendData[2];

	//Get the current status register
	SendData[0] = DS3232M_REG_STATUS;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxSz = 1;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = &RecieveData;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	SendData[1] = RecieveData & 0x7F;
	DS3232M_I2C.txSz = 2;
	DS3232M_I2C.rxSz = 0;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = &RecieveData;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	return;
}

void DS3232M_SetTime(TimeAndDate *TheTime)
{
	uint8_t SendData[8];

	SendData[0] = DS3232M_REG_SEC;
	SendData[1] = ((TheTime->sec % 10) | ((TheTime->sec / 10) << 4));
	SendData[2] = ((TheTime->min % 10) | ((TheTime->min / 10) << 4));
	SendData[3] = ((((TheTime->hour % 10) | ((TheTime->hour / 10) << 4))) & 0x3F);		//Note: this sets the RTC in 24 hour mode
	SendData[4] = TheTime->dow;
	SendData[5] = ((TheTime->day % 10) | ((TheTime->day / 10) << 4));
	SendData[6] = ((TheTime->month % 10) | ((TheTime->month / 10) << 4));	//NOTE: this probably clears the century bit. maybe look at this later.
	SendData[7] = ((TheTime->year % 10) | ((TheTime->year / 10) << 4));

	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.txSz = 8;
	DS3232M_I2C.rxSz = 0;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	DS3232M_ClearOSCFlag();

	return;
}

void DS3232M_GetTime(TimeAndDate *TheTime)
{
	uint8_t RecieveData[7];
	uint8_t SendData;

	SendData = DS3232M_REG_SEC;

	DS3232M_I2C.txBuff = &SendData;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxBuff = RecieveData;
	DS3232M_I2C.rxSz = 7;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	//Convert registers in BCD into the time struct
	TheTime->sec = ((RecieveData[0] & 0x0F) + ((RecieveData[0] & 0x70) >> 4)*10 );
	TheTime->min = ((RecieveData[1] & 0x0F) + ((RecieveData[1] & 0x70) >> 4)*10 );

	//Handle hours in 12 or 24 hour mode. The struct will always take 24 hour format
	if( (RecieveData[2] & 0x40) == 0x04)		//12 hour mode
	{
		if((RecieveData[2] & 0x20) == 0x20)		//PM
		{
			TheTime->hour = 12 + (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
		}
		else
		{
			TheTime->hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
		}
	}
	else	//24 Hour Mode
	{
		TheTime->hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x30) >> 4)*10;
	}

	TheTime->dow = (RecieveData[3] & 0x07);
	TheTime->day = ((RecieveData[4] & 0x0F) + ((RecieveData[4] & 0x30) >> 4)*10);
	TheTime->month = ((RecieveData[5] & 0x0F) + ((RecieveData[5] & 0x10) >> 4)*10);
	//TODO: Handle century bit...
	TheTime->year = ((RecieveData[6] & 0x0F) + ((RecieveData[6] & 0xF0) >> 4)*10);

	return;
}

//AlarmMasks format: <DY/DT(1=day, 0=date)> <AxM4> <AxM3> <AxM2> <A1M1>
//Note: A2M1 does not exist (no seconds register)
void DS3232M_SetAlarm(uint8_t AlarmNumber, uint8_t AlarmMasks, TimeAndDate *AlarmTime)
{
	//uint8_t RecieveData;
	uint8_t SendData[5];

	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxSz = 0;

	if(AlarmNumber == 1)
	{
		SendData[0] = DS3232M_REG_A1_SEC;
		SendData[1] = (((AlarmMasks & 0x01) << 7) | (AlarmTime->sec % 10) | ((AlarmTime->sec / 10) << 4));
		SendData[2] = (((AlarmMasks & 0x02) << 6) | (AlarmTime->min % 10) | ((AlarmTime->min / 10) << 4));
		SendData[3] = ((((AlarmMasks & 0x04) << 5) | (AlarmTime->hour % 10) | ((AlarmTime->hour / 10) << 4)) & 0xBF);

		if((AlarmMasks & 0x10) == 0x10)
		{
			//Alarm on day of week
			SendData[4] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->dow));
		}
		else
		{
			//Alarm on date
			SendData[4] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->day % 10) | ((AlarmTime->day / 10) << 4));
		}

		DS3232M_I2C.txSz = 5;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);
	}
	else
	{
		SendData[0] = DS3232M_REG_A2_MIN;
		SendData[1] = (((AlarmMasks & 0x02) << 6) | (AlarmTime->min % 10) | ((AlarmTime->min / 10) << 4));
		SendData[2] = ((((AlarmMasks & 0x04) << 5) | (AlarmTime->hour % 10) | ((AlarmTime->hour / 10) << 4)) & 0xBF);

		if((AlarmMasks & 0x10) == 0x10)
		{
			//Alarm on day of week
			SendData[3] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->dow));
		}
		else
		{
			//Alarm on date
			SendData[3] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->day % 10) | ((AlarmTime->day / 10) << 4));
		}

		DS3232M_I2C.txSz = 4;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);
	}
	return;
}

void DS3232M_EnableAlarm(uint8_t AlarmNumber)
{
	uint8_t RecieveData;
	uint8_t SendData[2];

	if((AlarmNumber == 1) || (AlarmNumber == 2))
	{
		//Get the current control register
		SendData[0] = DS3232M_REG_CONTROL;
		DS3232M_I2C.txSz = 1;
		DS3232M_I2C.rxSz = 1;
		DS3232M_I2C.txBuff = SendData;
		DS3232M_I2C.rxBuff = &RecieveData;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

		//Enable the requested alarm
		SendData[1] = RecieveData | AlarmNumber;
		DS3232M_I2C.txSz = 2;
		DS3232M_I2C.rxSz = 0;
		DS3232M_I2C.txBuff = SendData;
		DS3232M_I2C.rxBuff = &RecieveData;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);
	}
	return;
}

void DS3232M_DisableAlarm(uint8_t AlarmNumber)
{
	uint8_t RecieveData;
	uint8_t SendData[2];

	if((AlarmNumber == 1) || (AlarmNumber == 2))
	{
		//Get the current control register
		SendData[0] = DS3232M_REG_CONTROL;
		DS3232M_I2C.txSz = 1;
		DS3232M_I2C.rxSz = 1;
		DS3232M_I2C.txBuff = SendData;
		DS3232M_I2C.rxBuff = &RecieveData;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

		//Disable the requested alarm
		SendData[1] = (RecieveData & (~AlarmNumber));
		DS3232M_I2C.txSz = 2;
		DS3232M_I2C.rxSz = 0;
		DS3232M_I2C.txBuff = SendData;
		DS3232M_I2C.rxBuff = &RecieveData;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

		//Get the current status register
		SendData[0] = DS3232M_REG_STATUS;
		DS3232M_I2C.txSz = 1;
		DS3232M_I2C.rxSz = 1;
		DS3232M_I2C.txBuff = SendData;
		DS3232M_I2C.rxBuff = &RecieveData;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

		//Clear the interrupt flag for the requested alarm
		SendData[1] = (RecieveData & (~AlarmNumber));
		DS3232M_I2C.txSz = 2;
		DS3232M_I2C.rxSz = 0;
		DS3232M_I2C.txBuff = SendData;
		DS3232M_I2C.rxBuff = &RecieveData;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);
	}
	return;
}

uint8_t DS3232M_AlarmsActive(void)
{
	uint8_t StatusReg;
	uint8_t ControlReg;

	DS3232M_GetStatus(&StatusReg, &ControlReg);

	return (ControlReg & 0x07);
}

void DS3232M_ClearAlarmFlag(uint8_t AlarmNumber)
{
	uint8_t RecieveData;
	uint8_t SendData[2];

	//Get the current status register
	SendData[0] = DS3232M_REG_STATUS;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxSz = 1;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = &RecieveData;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	//Clear the interrupt flag for the requested alarm
	SendData[1] = (RecieveData & (~AlarmNumber));
	DS3232M_I2C.txSz = 2;
	DS3232M_I2C.rxSz = 0;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = &RecieveData;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	return;
}

void DS3232M_GetAlarmTime(uint8_t AlarmNumber, uint8_t *AlarmMasks, TimeAndDate *AlarmTime)
{
	uint8_t RecieveData[4];
	uint8_t SendData;

	//These will always be zero
	AlarmTime->month = 0;
	AlarmTime->year = 0;

	DS3232M_I2C.txBuff = &SendData;
	DS3232M_I2C.rxBuff = RecieveData;
	DS3232M_I2C.txSz = 1;

	//TODO: I should be able to combine the code for alarm 1 and 2 somehow
	if(AlarmNumber == 1)
	{
		SendData = DS3232M_REG_A1_SEC;
		DS3232M_I2C.rxSz = 4;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

		AlarmTime->sec = ((RecieveData[0] & 0x0F) + ((RecieveData[0] & 0x70) >> 4)*10 );
		AlarmTime->min = ((RecieveData[1] & 0x0F) + ((RecieveData[1] & 0x70) >> 4)*10 );

		//Handle hours in 12 or 24 hour mode. We will always return the hours in 24 hour format.
		if( (RecieveData[2] & 0x40) == 0x04)		//12 hour mode
		{
			if((RecieveData[2] & 0x20) == 0x20)		//PM
			{
				AlarmTime->hour = 12 + (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
			}
			else	//AM
			{
				AlarmTime->hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
			}
		}
		else	//24 Hour Mode
		{
			AlarmTime->hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x30) >> 4)*10;
		}

		if((RecieveData[3] & 0x40) == 0x40)
		{
			//Alarm on day of the week
			AlarmTime->dow = (RecieveData[3] & 0x07);
			AlarmTime->day = 0;
		}
		else
		{
			//Alarm on date of the month
			AlarmTime->day = ((RecieveData[3] & 0x0F) + ((RecieveData[3] & 0x30) >> 4)*10);
			AlarmTime->dow = 0;
		}

		*AlarmMasks = ((RecieveData[0] & 0x80) >> 7) | ((RecieveData[1] & 0x80) >> 6) | ((RecieveData[2] & 0x80) >> 5) | ((RecieveData[3] & 0x80) >> 4) | ((RecieveData[3] & 0x40) >> 2);
	}
	else if(AlarmNumber == 2)
	{
		SendData = DS3232M_REG_A2_MIN;
		DS3232M_I2C.rxSz = 3;
		Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

		AlarmTime->sec = 0;
		AlarmTime->min = ((RecieveData[0] & 0x0F) + ((RecieveData[0] & 0x70) >> 4)*10 );

		//Handle hours in 12 or 24 hour mode. We will always return the hours in 24 hour format.
		if( (RecieveData[1] & 0x40) == 0x04)		//12 hour mode
		{
			if((RecieveData[1] & 0x20) == 0x20)		//PM
			{
				AlarmTime->hour = 12 + (RecieveData[1] & 0x0F) + ((RecieveData[1] & 0x10) >> 4)*10;
			}
			else	//AM
			{
				AlarmTime->hour = (RecieveData[1] & 0x0F) + ((RecieveData[1] & 0x10) >> 4)*10;
			}
		}
		else	//24 Hour Mode
		{
			AlarmTime->hour = (RecieveData[1] & 0x0F) + ((RecieveData[1] & 0x30) >> 4)*10;
		}

		if((RecieveData[2] & 0x40) == 0x40)
		{
			//Alarm on day of the week
			AlarmTime->dow = (RecieveData[2] & 0x07);
			AlarmTime->day = 0;
		}
		else
		{
			//Alarm on date of the month
			AlarmTime->day = ((RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x30) >> 4)*10);
			AlarmTime->dow = 0;
		}

		*AlarmMasks = ((RecieveData[0] & 0x80) >> 6) | ((RecieveData[1] & 0x80) >> 5) | ((RecieveData[2] & 0x80) >> 4) | ((RecieveData[2] & 0x40) >> 2);
	}
	return;
}

void DS3232M_32KhzStart(uint8_t BatEnable)
{
	uint8_t RecieveData;
	uint8_t SendData[2];

	//Get the current control register
	SendData[0] = DS3232M_REG_STATUS;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxSz = 1;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = &RecieveData;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	//Enable the 32.768kHz output
	SendData[1] = RecieveData | (1<<3);
	if(BatEnable == 1)
	{
		SendData[1] |= (1<<6);
	}
	DS3232M_I2C.txSz = 2;
	DS3232M_I2C.rxSz = 0;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = &RecieveData;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	return;
}

void DS3232M_32KhzStop(void)
{
	uint8_t RecieveData;
	uint8_t SendData[2];

	//Get the current control register
	SendData[0] = DS3232M_REG_STATUS;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxSz = 1;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = &RecieveData;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	//Enable the 32.768kHz output
	SendData[1] = RecieveData & 0xB7;
	DS3232M_I2C.txSz = 2;
	DS3232M_I2C.rxSz = 0;
	DS3232M_I2C.txBuff = SendData;
	DS3232M_I2C.rxBuff = &RecieveData;
	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);

	return;
}

uint8_t DS3232M_GetTemp(int8_t *TempLHS, uint8_t *TempRHS)
{

	uint8_t RecieveData[2];
	uint8_t SendData;
	//uint8_t ret;

	DS3232M_I2C.txBuff = &SendData;
	DS3232M_I2C.txSz = 1;
	DS3232M_I2C.rxBuff = RecieveData;
	DS3232M_I2C.rxSz = 2;

	SendData = DS3232M_REG_TEMP_HI;

	Chip_I2C_MasterTransfer(DEFAULT_I2C, &DS3232M_I2C);
	//ret = TWIRW(DS3232M_SLA_ADDRESS, &SendData, RecieveData, 1, 2);

	//if(ret == 0)
	//{
		*TempLHS = RecieveData[0];				//LHS is in 2's compliment form
		*TempRHS = (RecieveData[1] >> 6)*25;	//The two MSB of this byte are the decimal portion of the temperature in .25s
		return 0;
		//printf("High: 0x%02X\nLow: 0x%02X\n", RecieveData[0], RecieveData[1]);
	//}
	//else
	//{
	//	printf_P(PSTR("I2C Error (0x%02X)\n"), ret);
	//	return ret;
	//}
}

//Time string should be 8 characters plus the terminating character
void DS3232M_GetTimeString(char *TimeString, uint8_t StringOptions)
{
	TimeAndDate CurrentTime;

	DS3232M_GetTime(&CurrentTime);

	TimeString[5] = ' ';
	TimeString[8] = '\0';

	if(CurrentTime.hour > 12)
	{
		CurrentTime.hour -= 12;
		TimeString[6] = 'P';
		TimeString[7] = 'M';
	}
	else
	{
		TimeString[6] = 'A';
		TimeString[7] = 'M';
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

	return;
}

//Date string should be 10 characters plus the terminating character
void DS3232M_GetDateString(char *DateString, uint8_t StringOptions)
{
	TimeAndDate CurrentTime;

	DS3232M_GetTime(&CurrentTime);

	DateString[2] = '/';
	DateString[5] = '/';

	DateString[6] = '2';
	DateString[7] = '0';

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

	DateString[8] = (char)((CurrentTime.year/10)+48);
	DateString[9] = (char)((CurrentTime.year%10)+48);

	return;
}

/** @} */
