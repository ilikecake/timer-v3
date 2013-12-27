/*
 * NHD28.c
 *
 *  Created on: Nov 18, 2013
 *      Author: pat
 */

#include "main.h"

#define OLED_CS_PORT		0
#define OLED_CS_PIN			22
#define OLED_DC_PORT		0
#define OLED_DC_PIN			23
#define OLED_RST_PORT		1
#define OLED_RST_PIN		29

#define OLED_MF_CS_PORT		1
#define OLED_MF_CS_PIN		21


//Global variables
//static char InitString[] = "OLED Initialized";
//static uint8_t MF_WA_Order[] = { 18, 2, 22, 6, 26, 10, 30, 14 };

uint8_t OLED_StatusReg;

void OLED_Init(void)
{
	uint8_t DataToSend[2];

	//Initialize GPIO
	Chip_IOCON_PinMuxSet(LPC_IOCON, OLED_CS_PORT, OLED_CS_PIN, IOCON_FUNC0 | IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, OLED_DC_PORT, OLED_DC_PIN, IOCON_FUNC0 | IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, OLED_RST_PORT, OLED_RST_PIN, IOCON_FUNC0 | IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, OLED_MF_CS_PORT, OLED_MF_CS_PIN, IOCON_FUNC0 | IOCON_MODE_INACT);

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, OLED_CS_PORT, OLED_CS_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, OLED_DC_PORT, OLED_DC_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, OLED_RST_PORT, OLED_RST_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, OLED_MF_CS_PORT, OLED_MF_CS_PIN);

	Chip_GPIO_SetPinState(LPC_GPIO, OLED_CS_PORT, OLED_CS_PIN, true);
	Chip_GPIO_SetPinState(LPC_GPIO, OLED_DC_PORT, OLED_DC_PIN, true);
	Chip_GPIO_SetPinState(LPC_GPIO, OLED_RST_PORT, OLED_RST_PIN, true);
	Chip_GPIO_SetPinState(LPC_GPIO, OLED_MF_CS_PORT, OLED_MF_CS_PIN, true);

	//Send OLED init sequence
	DataToSend[0] = 0x12;
	OLED_SendCommand(OLED_COMMAND_LOCK, DataToSend, 1);

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

#ifdef OLED_COLUMN_INCREMENT
	DataToSend[0] = 0x16;
#else
	DataToSend[0] = 0x17;
#endif
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

	OLED_ClearDisplay();	//Clear the OLED display before we turn it on

	OLED_SendCommand(OLED_SLEEP_MODE_OFF, NULL, 0);

	OLED_SetStatus(OLED_STATUS_ORENTATION_MASK, OLED_STATUS_ORENTATION_DOWN);

	//OLED_WriteMFString(MF_ASCII_SIZE_7X8, InitString, 16, 28);

	return;
}

void OLED_SetStatus(uint8_t StatusMask, uint8_t StatusValue)
{
	OLED_StatusReg &= ((~StatusMask) | StatusValue);	//Set the desired bits to zero
	OLED_StatusReg |= (StatusMask & StatusValue);			//Set the desired bits to one
	return;
}

uint8_t OLED_GetStatus(void)
{
	return OLED_StatusReg;
}

/** 1 to select, 0 to deselect */
void OLED_Select(uint8_t sel)
{
	if(sel == 1)
	{
		Chip_GPIO_SetPinState(LPC_GPIO, OLED_CS_PORT, OLED_CS_PIN, false);
	}
	else
	{
		Chip_GPIO_SetPinState(LPC_GPIO, OLED_CS_PORT, OLED_CS_PIN, true);
	}
	return;
}

void OLED_MF_Select(uint8_t sel)
{
	if(sel == 1)
	{
		Chip_GPIO_SetPinState(LPC_GPIO, OLED_MF_CS_PORT, OLED_MF_CS_PIN, false);
	}
	else
	{
		Chip_GPIO_SetPinState(LPC_GPIO, OLED_MF_CS_PORT, OLED_MF_CS_PIN, true);
	}
	return;
}

/** 1 for data, 0 for command */
void OLED_DC(uint8_t sel)
{
	if(sel == 1)
	{
		Chip_GPIO_SetPinState(LPC_GPIO, OLED_DC_PORT, OLED_DC_PIN, true);
	}
	else
	{
		Chip_GPIO_SetPinState(LPC_GPIO, OLED_DC_PORT, OLED_DC_PIN, false);
	}
	return;
}

/** 1 for reset, 0 for unreset */
void OLED_Reset(uint8_t rst)
{
	if(rst == 1)
	{
		//OLED is reset when the pin is low
		Chip_GPIO_SetPinState(LPC_GPIO, OLED_RST_PORT, OLED_RST_PIN, false);
	}
	else
	{
		Chip_GPIO_SetPinState(LPC_GPIO, OLED_RST_PORT, OLED_RST_PIN, true);
	}
	return;
}

void OLED_SendCommand(uint8_t Command, uint8_t *args, uint8_t arg_length)
{
	OLED_DC(0);
	OLED_Select(1);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, &Command, 1);

	if(arg_length > 0)
	{
		OLED_DC(1);
		Chip_SSP_WriteFrames_Blocking(LPC_SSP0, args, arg_length);
	}
	OLED_Select(0);

	return;
}

void OLED_ClearDisplay(void)
{
	uint8_t BlankPattern;

	BlankPattern = 0x00;
	OLED_FillDisplay(&BlankPattern, 1);

	return;
}

void OLED_FillDisplay(uint8_t *FillPattern, uint8_t FillPaternLength)
{
	uint8_t i;
	uint8_t j;

	uint8_t CommandData[2];

	CommandData[0] = 0x1C;
	CommandData[1] = 0x5B;
	OLED_SendCommand(OLED_SET_COLUMN_ADDRESS, CommandData, 2);

	CommandData[0] = 0x00;
	CommandData[1] = 0x3F;
	OLED_SendCommand(OLED_SET_ROW_ADDRESS, CommandData, 2);

	i = OLED_WRITE_RAM;

	OLED_DC(0);
	OLED_Select(1);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, &i, 1);

	OLED_DC(1);

	for(j=0;j<64/FillPaternLength;j++)
	{
		for(i=0;i<128/FillPaternLength;i++)
		{
			Chip_SSP_WriteFrames_Blocking(LPC_SSP0, FillPattern, FillPaternLength);
		}
	}
	OLED_Select(0);

	return;
}

void OLED_SetWindow(uint8_t StartColumn, uint8_t EndColumn, uint8_t StartRow, uint8_t EndRow)
{
	uint8_t OLED_SendBuffer[2];
	//TODO: Add check for max column and row

	OLED_SendBuffer[0] = 0x1C+StartColumn;
	OLED_SendBuffer[1] = 0x1C+EndColumn;
	OLED_SendCommand(OLED_SET_COLUMN_ADDRESS, OLED_SendBuffer, 2);

	OLED_SendBuffer[0] = 0x00+StartRow;
	OLED_SendBuffer[1] = 0x00+EndRow;
	OLED_SendCommand(OLED_SET_ROW_ADDRESS, OLED_SendBuffer, 2);
	return;
}




//void OLED_FillDisplay(void);
//void OLED_CheckerboardDisplay(void);

//TODO: Make this work with other font sizes
//NOTE: this is in colum increment mode
#ifdef OLED_COLUMN_INCREMENT
void OLED_WriteMFChar(uint8_t CharSize, char CharToWrite, uint8_t ColumnToStart, uint8_t RowToStart)
{
	uint8_t MF_FontBuffer[34];		//TODO: this needs to be bigger later...
	uint8_t OLED_FontBuffer[8];
	int8_t i;
	int8_t j;

	//TODO: add a check to make sure CharSize is an expected value

	//Get the font data from storage
	MF_GetAsciiChar(CharSize, CharToWrite, MF_FontBuffer);

	if(CharSize == MF_ASCII_SIZE_WA)
	{
		printf("size: 0x%02X%02X\r\n", MF_FontBuffer[0], MF_FontBuffer[1]);
		//for(i=0;i<34;i++)
		//{
		//	printf("ch[%u]: 0x%02X\r\n", i, MF_FontBuffer[i]);
		//}

	}

	//Set column addresses
	//Reuse the OLED_FontBuffer so that we don't need to define a new array
	//This is the same for 5x7, 7x8, and 8x16 (i think)
	OLED_FontBuffer[0] = 0x1C+ColumnToStart;
	if(CharSize == MF_ASCII_SIZE_WA)	//Chars are 16 bytes wide (TODO: not really, fix this later)
	{
		OLED_FontBuffer[1] = 0x1F+ColumnToStart;
	}
	else	//Chars are 8 bytes wide
	{
		OLED_FontBuffer[1] = 0x1d+ColumnToStart;
	}
	OLED_SendCommand(OLED_SET_COLUMN_ADDRESS, OLED_FontBuffer, 2);


	OLED_FontBuffer[0] = 0x00+RowToStart;
	if((CharSize == MF_ASCII_SIZE_5X7) ||(CharSize == MF_ASCII_SIZE_7X8))
	{
		OLED_FontBuffer[1] = 0x07+RowToStart;
	}
	else if((CharSize == MF_ASCII_SIZE_8X16) ||(CharSize == MF_ASCII_SIZE_WA))
	{
		OLED_FontBuffer[1] = 0x015+RowToStart;
	}
	OLED_SendCommand(OLED_SET_ROW_ADDRESS, OLED_FontBuffer, 2);

	if(CharSize == MF_ASCII_SIZE_WA)
	{
		for(i=7;i>=0;i--)
		{
			for(j=18;j<35;j+=2)
			{
				OLED_FontBuffer[((j-18)/2)] = ((MF_FontBuffer[j]>>i)&0x01)*0xF0 + ((MF_FontBuffer[j+1]>>i)&0x01)*0x0F;
			}
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 8);
		}

		for(i=7;i>=0;i--)
		{
			for(j=2;j<18;j+=2)
			{
				OLED_FontBuffer[((j-2)/2)] = ((MF_FontBuffer[j]>>i)&0x01)*0xF0 + ((MF_FontBuffer[j+1]>>i)&0x01)*0x0F;
			}
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 8);
		}
	}
	else
	{
		if(CharSize == MF_ASCII_SIZE_8X16)
		{
			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((MF_FontBuffer[8]>>i)&0x01)*0xF0 + ((MF_FontBuffer[9]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((MF_FontBuffer[10]>>i)&0x01)*0xF0 + ((MF_FontBuffer[11]>>i)&0x01)*0x0F;
				OLED_FontBuffer[2] = ((MF_FontBuffer[12]>>i)&0x01)*0xF0 + ((MF_FontBuffer[13]>>i)&0x01)*0x0F;
				OLED_FontBuffer[3] = ((MF_FontBuffer[14]>>i)&0x01)*0xF0 + ((MF_FontBuffer[15]>>i)&0x01)*0x0F;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 4);
			}
		}

		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[0]>>i)&0x01)*0xF0 + ((MF_FontBuffer[1]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[2]>>i)&0x01)*0xF0 + ((MF_FontBuffer[3]>>i)&0x01)*0x0F;
			OLED_FontBuffer[2] = ((MF_FontBuffer[4]>>i)&0x01)*0xF0 + ((MF_FontBuffer[5]>>i)&0x01)*0x0F;
			OLED_FontBuffer[3] = ((MF_FontBuffer[6]>>i)&0x01)*0xF0 + ((MF_FontBuffer[7]>>i)&0x01)*0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 4);
		}
	}
	return;
}
#else
void OLED_WriteMFChar(uint8_t CharSize, char CharToWrite, uint8_t ColumnToStart, uint8_t RowToStart)
{
	uint8_t MF_FontBuffer[34];		//TODO: this needs to be bigger later...
	uint8_t OLED_FontBuffer[8];
	int8_t i;
	//int8_t j;

	//TODO: add a check to make sure CharSize is an expected value

	//Get the font data from storage
	MF_GetAsciiChar(CharSize, CharToWrite, MF_FontBuffer);



	switch(CharSize)
	{
		case MF_ASCII_SIZE_5X7:
		case MF_ASCII_SIZE_7X8:
			OLED_SetWindow(ColumnToStart, ColumnToStart+1, RowToStart, RowToStart+7);
			break;

		case MF_ASCII_SIZE_8X16:
			OLED_SetWindow(ColumnToStart, ColumnToStart+1, RowToStart, RowToStart+15);
			break;

		case MF_ASCII_SIZE_WA:
			OLED_SetWindow(ColumnToStart, ColumnToStart+3, RowToStart, RowToStart+15);
			break;

		default:
			return;
	}

	if(CharSize == MF_ASCII_SIZE_WA)
	{
		//--------------------------------------------------
		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[18]>>i)&0x01)*0xF0 + ((MF_FontBuffer[19]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[20]>>i)&0x01)*0xF0 + ((MF_FontBuffer[21]>>i)&0x01)*0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}

		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[2]>>i)&0x01)*0xF0 + ((MF_FontBuffer[3]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[4]>>i)&0x01)*0xF0 + ((MF_FontBuffer[5]>>i)&0x01)*0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}
		//--------------------------------------------------
		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[22]>>i)&0x01)*0xF0 + ((MF_FontBuffer[23]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[24]>>i)&0x01)*0xF0 + ((MF_FontBuffer[25]>>i)&0x01)*0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}
		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[6]>>i)&0x01)*0xF0 + ((MF_FontBuffer[7]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[8]>>i)&0x01)*0xF0 + ((MF_FontBuffer[9]>>i)&0x01)*0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}
		//--------------------------------------------------
		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[26]>>i)&0x01)*0xF0 + ((MF_FontBuffer[27]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[28]>>i)&0x01)*0xF0 + ((MF_FontBuffer[29]>>i)&0x01)*0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}
		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[10]>>i)&0x01)*0xF0 + ((MF_FontBuffer[11]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[12]>>i)&0x01)*0xF0 + ((MF_FontBuffer[13]>>i)&0x01)*0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}
		//--------------------------------------------------
		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[30]>>i)&0x01)*0xF0 + ((MF_FontBuffer[31]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[32]>>i)&0x01)*0xF0 + ((MF_FontBuffer[33]>>i)&0x01)*0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}
		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[14]>>i)&0x01)*0xF0 + ((MF_FontBuffer[15]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[16]>>i)&0x01)*0xF0 + ((MF_FontBuffer[17]>>i)&0x01)*0x0F;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}
		//--------------------------------------------------











		/*for(i=7;i>=0;i--)
		{
			for(j=18;j<35;j+=2)
			{
				OLED_FontBuffer[((j-18)/2)] = ((MF_FontBuffer[j]>>i)&0x01)*0xF0 + ((MF_FontBuffer[j+1]>>i)&0x01)*0x0F;
			}
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 8);
		}

		for(i=7;i>=0;i--)
		{
			for(j=2;j<18;j+=2)
			{
				OLED_FontBuffer[((j-2)/2)] = ((MF_FontBuffer[j]>>i)&0x01)*0xF0 + ((MF_FontBuffer[j+1]>>i)&0x01)*0x0F;
			}
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 8);
		}*/
	}
	else
	{
		if(CharSize == MF_ASCII_SIZE_8X16)
		{
			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((MF_FontBuffer[8]>>i)&0x01)*0xF0 + ((MF_FontBuffer[9]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((MF_FontBuffer[10]>>i)&0x01)*0xF0 + ((MF_FontBuffer[11]>>i)&0x01)*0x0F;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}

			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((MF_FontBuffer[0]>>i)&0x01)*0xF0 + ((MF_FontBuffer[1]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((MF_FontBuffer[2]>>i)&0x01)*0xF0 + ((MF_FontBuffer[3]>>i)&0x01)*0x0F;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}
			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((MF_FontBuffer[12]>>i)&0x01)*0xF0 + ((MF_FontBuffer[13]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((MF_FontBuffer[14]>>i)&0x01)*0xF0 + ((MF_FontBuffer[15]>>i)&0x01)*0x0F;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}
			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((MF_FontBuffer[4]>>i)&0x01)*0xF0 + ((MF_FontBuffer[5]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((MF_FontBuffer[6]>>i)&0x01)*0xF0 + ((MF_FontBuffer[7]>>i)&0x01)*0x0F;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}
		}
		else
		{

			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((MF_FontBuffer[0]>>i)&0x01)*0xF0 + ((MF_FontBuffer[1]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((MF_FontBuffer[2]>>i)&0x01)*0xF0 + ((MF_FontBuffer[3]>>i)&0x01)*0x0F;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}
			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((MF_FontBuffer[4]>>i)&0x01)*0xF0 + ((MF_FontBuffer[5]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((MF_FontBuffer[6]>>i)&0x01)*0xF0 + ((MF_FontBuffer[7]>>i)&0x01)*0x0F;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}
		}
	}
	return;
}
#endif

void OLED_WriteMFString(uint8_t CharSize, char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart)
{
	uint8_t StringSize;
	uint8_t i;

	StringSize = strlen(StringToWrite);

	for(i=0;i<StringSize;i++)
	{
		if(CharSize == MF_ASCII_SIZE_WA)
		{
			OLED_WriteMFChar(CharSize, StringToWrite[i], ColumnToStart+i*4, RowToStart);
		}
		else
		{
			OLED_WriteMFChar(CharSize, StringToWrite[i], ColumnToStart+i*2, RowToStart);
		}
	}

	return;
}

void OLED_WriteMFString_WA(uint8_t CharSize, char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions)
{
	uint8_t StringSize;
	int8_t i;
	uint8_t FontString1[8];
	int8_t CurrentCharLength;
	uint8_t CurrentColumn;
	uint8_t CurrentCharNumber;
	uint8_t CurrentCharOffset;

	uint8_t OLED_FontBuffer[2];

	//NOTE: this only works for MF_ASCII_SIZE_WA for now...

	if(CharSize != MF_ASCII_SIZE_WA) return;


	StringSize = strlen(StringToWrite);

	CurrentCharNumber = 0;
	CurrentCharOffset = 0;
	CurrentColumn = ColumnToStart;

	MF_GetAsciiChar_4B(CharSize, StringToWrite[CurrentCharNumber], 0, FontString1);
	CurrentCharLength = (FontString1[0] << 8) + FontString1[1];

	i = 0;

	while (1)//Timeout < 50)
	{
		OLED_SetWindow(CurrentColumn, CurrentColumn, RowToStart, RowToStart+15);
		//printf("CCL: %u\r\n", CurrentCharLength);
		//printf("OFS: %u\r\n", CurrentCharOffset);
		//printf("CCN: %u\r\n", CurrentCharNumber);

		//Timeout++;
		MF_GetAsciiChar_4B(CharSize, StringToWrite[CurrentCharNumber], 18+CurrentCharOffset, FontString1);
		//printf("FS1: 0x%02X 0x%02X 0x%02X 0x%02X\r\n", FontString1[0], FontString1[1], FontString1[2], FontString1[3]);

		if((CurrentCharOffset + 4) <= CurrentCharLength)
		{

			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((FontString1[0]>>i)&0x01)*0xF0 + ((FontString1[1]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((FontString1[2]>>i)&0x01)*0xF0 + ((FontString1[3]>>i)&0x01)*0x0F;
				if((FontOptions & OLED_FONT_INVERSE) == OLED_FONT_INVERSE)
				{
					OLED_FontBuffer[0] = ~OLED_FontBuffer[0];
					OLED_FontBuffer[1] = ~OLED_FontBuffer[1];
				}
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}

			MF_GetAsciiChar_4B(CharSize, StringToWrite[CurrentCharNumber], 2+CurrentCharOffset, FontString1);
			//printf("FS1: 0x%02X 0x%02X 0x%02X 0x%02X\r\n", FontString1[0], FontString1[1], FontString1[2], FontString1[3]);
			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((FontString1[0]>>i)&0x01)*0xF0 + ((FontString1[1]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((FontString1[2]>>i)&0x01)*0xF0 + ((FontString1[3]>>i)&0x01)*0x0F;
				if((FontOptions & OLED_FONT_INVERSE) == OLED_FONT_INVERSE)
				{
					OLED_FontBuffer[0] = ~OLED_FontBuffer[0];
					OLED_FontBuffer[1] = ~OLED_FontBuffer[1];
				}
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}

			if((CurrentCharOffset + 4) < CurrentCharLength)
			{
				//printf("OFS+4 @ 1\r\n");
				CurrentCharOffset += 4;
			}
			else if(CurrentCharNumber == StringSize-1)
			{
				//printf("end1\r\n");
				//Timeout = 100;
				return;
			}
			else
			{
				//printf("CCN+1 @ 1\r\n");
				CurrentCharOffset = 0;
				CurrentCharNumber++;

				MF_GetAsciiChar_4B(CharSize, StringToWrite[CurrentCharNumber], 0, FontString1);
				CurrentCharLength = (FontString1[0] << 8) + FontString1[1];

			}
		}
		else
		{
			MF_GetAsciiChar_4B(CharSize, StringToWrite[CurrentCharNumber+1], 18, &FontString1[CurrentCharLength-CurrentCharOffset]);
			//printf("FS1_up: 0x%02X 0x%02X 0x%02X 0x%02X\r\n", FontString1[0], FontString1[1], FontString1[2], FontString1[3]);

			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((FontString1[0]>>i)&0x01)*0xF0 + ((FontString1[1]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((FontString1[2]>>i)&0x01)*0xF0 + ((FontString1[3]>>i)&0x01)*0x0F;
				if((FontOptions & OLED_FONT_INVERSE) == OLED_FONT_INVERSE)
				{
					OLED_FontBuffer[0] = ~OLED_FontBuffer[0];
					OLED_FontBuffer[1] = ~OLED_FontBuffer[1];
				}
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}

			MF_GetAsciiChar_4B(CharSize, StringToWrite[CurrentCharNumber], 2+CurrentCharOffset, FontString1);
			//printf("FS1: 0x%02X 0x%02X 0x%02X 0x%02X\r\n", FontString1[0], FontString1[1], FontString1[2], FontString1[3]);
			MF_GetAsciiChar_4B(CharSize, StringToWrite[CurrentCharNumber+1], 2, &FontString1[CurrentCharLength-CurrentCharOffset]);
			//printf("FS1_up: 0x%02X 0x%02X 0x%02X 0x%02X\r\n", FontString1[0], FontString1[1], FontString1[2], FontString1[3]);
			for(i=7;i>=0;i--)
			{
				OLED_FontBuffer[0] = ((FontString1[0]>>i)&0x01)*0xF0 + ((FontString1[1]>>i)&0x01)*0x0F;
				OLED_FontBuffer[1] = ((FontString1[2]>>i)&0x01)*0xF0 + ((FontString1[3]>>i)&0x01)*0x0F;
				if((FontOptions & OLED_FONT_INVERSE) == OLED_FONT_INVERSE)
				{
					OLED_FontBuffer[0] = ~OLED_FontBuffer[0];
					OLED_FontBuffer[1] = ~OLED_FontBuffer[1];
				}
				OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
			}


			//printf("CCL-OFS: %u\r\n", CurrentCharLength-CurrentCharOffset);

			switch(CurrentCharLength-CurrentCharOffset)
			{

			case 1:
				CurrentCharOffset = 3;
				CurrentCharNumber++;
				break;

			case 2:
				CurrentCharOffset = 2;
				CurrentCharNumber++;
				break;

			case 3:
				CurrentCharOffset = 1;
				CurrentCharNumber++;
				break;
			}

			MF_GetAsciiChar_4B(CharSize, StringToWrite[CurrentCharNumber], 0, FontString1);
			CurrentCharLength = (FontString1[0] << 8) + FontString1[1];

			if(CurrentCharNumber == StringSize)
			{
				//printf("end2\r\n");
				//Timeout = 100;
				return;
			}
		}
		CurrentColumn++;
	}
	return;
}





void MF_GetAsciiChar(uint8_t CharSize, char CharToGet, uint8_t *CharArray)
{
	uint8_t DataToSend[5];
	uint32_t Address;
	uint8_t LengthToReceive;

	if( ((uint8_t)CharToGet >= 0x20) && ((uint8_t)CharToGet <= 0xFF) )
	{
		if(CharSize == MF_ASCII_SIZE_5X7)
		{
			Address = ((uint32_t)(CharToGet)-0x20)*8;
			LengthToReceive = 8;
		}
		else if(CharSize == MF_ASCII_SIZE_7X8)
		{
			Address = ((uint32_t)(CharToGet)-0x20)*8 + 768;
			LengthToReceive = 8;
		}
		else if(CharSize == MF_ASCII_SIZE_8X16)
		{
			Address = ((uint32_t)(CharToGet)-0x20)*16 + 1536;
			LengthToReceive = 16;
		}
		else if(CharSize == MF_ASCII_SIZE_WA)
		{
			Address = ((uint32_t)(CharToGet)-0x20)*34 + 3072;
			LengthToReceive = 34;
		}
		else
		{
			return;
		}

		DataToSend[0] = 0x0B;
		DataToSend[1] = (uint8_t)((Address >> 16) & 0xFF);
		DataToSend[2] = (uint8_t)((Address >> 8) & 0xFF);
		DataToSend[3] = (uint8_t)(Address & 0xFF);
		DataToSend[4] = 0x00;

		OLED_MF_Select(1);
		Chip_SSP_WriteFrames_Blocking(LPC_SSP0, DataToSend, 5);
		Chip_SSP_ReadFrames_Blocking(LPC_SSP0, CharArray, LengthToReceive);
		OLED_MF_Select(0);
	}

	return;
}


void MF_GetAsciiChar_4B(uint8_t CharSize, char CharToGet, uint8_t CharStartByte, uint8_t *CharArray)
{
	uint8_t DataToSend[5];
	uint32_t Address;

	if( ((uint8_t)CharToGet >= 0x20) && ((uint8_t)CharToGet <= 0xFF) )
	{
		if(CharSize == MF_ASCII_SIZE_5X7)
		{
			Address = ((uint32_t)(CharToGet)-0x20)*8;
		}
		else if(CharSize == MF_ASCII_SIZE_7X8)
		{
			Address = ((uint32_t)(CharToGet)-0x20)*8 + 768;
		}
		else if(CharSize == MF_ASCII_SIZE_8X16)
		{
			Address = ((uint32_t)(CharToGet)-0x20)*16 + 1536;
		}
		else if(CharSize == MF_ASCII_SIZE_WA)
		{
			Address = ((uint32_t)(CharToGet)-0x20)*34 + 3072;
		}
		else
		{
			return;
		}

		//TODO: Add check to make sure the charstartbyte is not too big?
		Address += CharStartByte;

		DataToSend[0] = 0x0B;
		DataToSend[1] = (uint8_t)((Address >> 16) & 0xFF);
		DataToSend[2] = (uint8_t)((Address >> 8) & 0xFF);
		DataToSend[3] = (uint8_t)(Address & 0xFF);
		DataToSend[4] = 0x00;

		OLED_MF_Select(1);
		Chip_SSP_WriteFrames_Blocking(LPC_SSP0, DataToSend, 5);
		Chip_SSP_ReadFrames_Blocking(LPC_SSP0, CharArray, 4);
		OLED_MF_Select(0);
	}




	return;
}

void OLED_WriteMF_UInt(uint8_t CharSize, uint32_t NumberToWrite, uint8_t ColumnToStart, uint8_t RowToStart)
{
	char NumberString[10];

	int8_t i;
	uint32_t Divisor[10];
	uint8_t digit;
	uint8_t isStarted;

	Divisor[0] = 1;
	Divisor[1] = 10;
	Divisor[2] = 100;
	Divisor[3] = 1000;
	Divisor[4] = 10000;
	Divisor[5] = 100000;
	Divisor[6] = 1000000;
	Divisor[7] = 10000000;
	Divisor[8] = 100000000;
	Divisor[9] = 1000000000;

	isStarted = 0xFF;


	//printf("blarg\r\n");


	for(i=9;i>=0;i--)
	{
		digit = (uint8_t)(NumberToWrite/Divisor[i]);
		//printf("num: %lu\r\n", NumberToWrite);
		//printf("%u: %u\r\n", i, digit);

		if((digit > 0) && (isStarted == 0xFF))
		{
			//printf("start\r\n");
			isStarted = i;
		}

		if(isStarted != 0xFF)
		{
			//printf("ch: %c\r\n", (char)(digit + 48));
			NumberString[isStarted-i] = (char)(digit + 48);
			//printf("sub: %lu\r\n", (Divisor[i]*(uint32_t)digit));
			NumberToWrite = NumberToWrite - (Divisor[i]*(uint32_t)digit);
		}
	}


	NumberString[isStarted+1] = '\0';

	//printf("out: ");
	//printf(NumberString);
	//printf("\r\n");


	OLED_WriteMFString(CharSize, NumberString, ColumnToStart, RowToStart);
	return;
}

//TODO: write this function...
void OLED_WriteMF_Int(uint8_t CharSize, int32_t NumberToWrite, uint8_t ColumnToStart, uint8_t RowToStart)
{




	return;
}









void OLED_WriteMFString_Q(uint8_t CharSize, char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart)
{
	OLED_Command CommandToSend;
	uint8_t sz;

	sz = strlen (StringToWrite);

	CommandToSend.CommandName = OLED_CMD_WRITE_STRING;
	CommandToSend.CommandData[0] = ColumnToStart;
	CommandToSend.CommandData[1] = RowToStart;
	CommandToSend.CommandData[2] = CharSize;

	if(sz >=16 )
	{
		memcpy(CommandToSend.CommandCharData, StringToWrite, 16);
	}
	else
	{
		memcpy(CommandToSend.CommandCharData, StringToWrite, sz);
	}

	xQueueSend(xOLEDCommands, (void *)&CommandToSend, portMAX_DELAY);

	return;
}

//Note: the display will have to be redrawn after this function is called
//TODO: update the status in this function
void OLED_DisplayRotation(uint8_t Rotation)
{
	uint8_t DataToSend[2];

	if(Rotation == 1)
	{
		OLED_SetStatus(OLED_STATUS_ORENTATION_MASK, OLED_STATUS_ORENTATION_DOWN);
		DataToSend[0] = 0x17;
	}
	else
	{
		OLED_SetStatus(OLED_STATUS_ORENTATION_MASK, OLED_STATUS_ORENTATION_UP);
		DataToSend[0] = 0x05;
	}
	DataToSend[1] = 0x11;
	OLED_SendCommand(OLED_SET_REMAP, DataToSend, 2);

	return;
}


void OLED_DisplayContrast(uint8_t Contrast)
{
	uint8_t DataToSend;
	DataToSend = Contrast;
	OLED_SendCommand(OLED_SET_CONTRAST_CURRENT, &DataToSend, 1);

	return;
}

void OLED_WriteLine(uint8_t X_start, uint8_t Y_start, uint8_t X_end, uint8_t Y_end, uint8_t weight, uint8_t prop)
{
	uint8_t i;
	uint8_t j;
	int16_t tempval;		//TODO: combine this with j?
	uint8_t OLED_PixelBuffer[4];

	j = 0;

	if(X_start == X_end)
	{
		//Vertical line
		OLED_SetWindow(X_start/4, X_end/4, Y_start, Y_end);

		switch(X_start & 0x03)
		{
			case 0:
				OLED_PixelBuffer[0] = 0xF0;
				OLED_PixelBuffer[1] = 0x00;
				break;

			case 1:
				OLED_PixelBuffer[0] = 0x0F;
				OLED_PixelBuffer[1] = 0x00;
				break;

			case 2:
				OLED_PixelBuffer[0] = 0x00;
				OLED_PixelBuffer[1] = 0xF0;
				break;

			case 3:
				OLED_PixelBuffer[0] = 0x00;
				OLED_PixelBuffer[1] = 0x0F;
				break;
		}

		for(i=0;i<(Y_end-Y_start);i++)
		{
			OLED_SendCommand(OLED_WRITE_RAM, OLED_PixelBuffer, 2);
		}
	}
	else if(Y_start == Y_end)
	{
		//Horizontal line
		OLED_SetWindow(X_start/4, X_end/4, Y_start, Y_start);

		switch(X_start & 0x03)
		{
			case 0:
				OLED_PixelBuffer[0] = 0xFF;
				OLED_PixelBuffer[1] = 0xFF;
				break;

			case 1:
				OLED_PixelBuffer[0] = 0x0F;
				OLED_PixelBuffer[1] = 0xFF;
				break;

			case 2:
				OLED_PixelBuffer[0] = 0x00;
				OLED_PixelBuffer[1] = 0xFF;
				break;

			case 3:
				OLED_PixelBuffer[0] = 0x00;
				OLED_PixelBuffer[1] = 0x0F;
				break;
		}
		OLED_SendCommand(OLED_WRITE_RAM, OLED_PixelBuffer, 2);

		j = ((X_end-X_start+1)-(4-(X_start & 0x03)));	//This is the number of pixels to write after the first set of four is written

		for(i=1; i<=j/4; i++)
		{
			OLED_PixelBuffer[0] = 0xFF;
			OLED_PixelBuffer[1] = 0xFF;
			OLED_SendCommand(OLED_WRITE_RAM, OLED_PixelBuffer, 2);
		}

		switch( j & 0x03 )
		{
			case 0:
				break;

			case 1:
				OLED_PixelBuffer[0] = 0xF0;
				OLED_PixelBuffer[1] = 0x00;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_PixelBuffer, 2);
				break;

			case 2:
				OLED_PixelBuffer[0] = 0xFF;
				OLED_PixelBuffer[1] = 0x00;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_PixelBuffer, 2);
				break;

			case 3:
				OLED_PixelBuffer[0] = 0xFF;
				OLED_PixelBuffer[1] = 0xF0;
				OLED_SendCommand(OLED_WRITE_RAM, OLED_PixelBuffer, 2);
				break;
		}
	}
	else
	{

		/*
		j = Y_start;


		for(i=0; i<((((X_end/4)-(X_start/4))+1)*4); i++)
		{
			if( i < (X_start & 0x03))
			{
				//do nothing
			}
			else
			{
				tempval = (((int16_t)Y_end-(int16_t)Y_start)*i);
				tempval = tempval/(X_end-X_start);
				if(tempval == j)
				{
					switch( i & 0x03 )
					{
						case 0:
							OLED_PixelBuffer[0] |= 0xF0;
							break;

						case 1:
							OLED_PixelBuffer[0] |= 0x0F;
							break;

						case 2:
							OLED_PixelBuffer[1] |= 0xF0;
							break;

						case 3:
							OLED_PixelBuffer[1] |= 0x0F;
							break;
					}
				}
				else
				{


				}


			}





		}

		//OLED_SetWindow(X_start/4, X_end/4, Y_start, Y_end);

		tempval = ((int16_t)Y_end-(int16_t)Y_start);
		*/















		//Uh-oh
	}

	return;
}


