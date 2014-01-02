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

/*
 * StartColumn and StartRow are zero based.
 * To write to a single column or row, the start value and end value should be the same. *
 */
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

void OLED_ClearWindow(uint8_t StartColumn, uint8_t EndColumn, uint8_t StartRow, uint8_t EndRow)
{
	uint8_t OLED_SendBuffer[2];
	uint16_t i;
	uint16_t SizeToFill;
	//uint8_t j;
	//TODO: Add check for max column and row

	OLED_SendBuffer[0] = 0x1C+StartColumn;
	OLED_SendBuffer[1] = 0x1C+EndColumn;
	OLED_SendCommand(OLED_SET_COLUMN_ADDRESS, OLED_SendBuffer, 2);

	OLED_SendBuffer[0] = 0x00+StartRow;
	OLED_SendBuffer[1] = 0x00+EndRow;
	OLED_SendCommand(OLED_SET_ROW_ADDRESS, OLED_SendBuffer, 2);

	SizeToFill = (EndRow-StartRow+1)*(EndColumn-StartColumn+1);

	for(i=0;i<SizeToFill;i++)
	{
		OLED_SendBuffer[0] = 0x00;
		OLED_SendBuffer[1] = 0x00;
		OLED_SendCommand(OLED_WRITE_RAM, OLED_SendBuffer, 2);
	}

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
void OLED_WriteMFChar(uint8_t CharSize, const char CharToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions)
{
	//TODO: make this function return the length of the characters written in pixels
	//TODO: make this function take pixel location instead of column locations

	uint8_t MF_FontBuffer[34];
	uint8_t OLED_FontBuffer[8];
	int8_t i;
	int8_t j;

	//TODO: do i need all four of these?
	uint8_t LoopSize;
	uint8_t StartVal;
	uint8_t FontBase;
	uint8_t FontBaseOffset;

	uint8_t CalcFontBase;

	//Get the font data from storage
	MF_GetAsciiChar(CharSize, CharToWrite, MF_FontBuffer);


	//Set the windows for the character
	//This switch also makes sure that CharSize is valid
	switch(CharSize)
	{
		case MF_ASCII_SIZE_5X7:
		case MF_ASCII_SIZE_7X8:
			OLED_SetWindow(ColumnToStart, ColumnToStart+1, RowToStart, RowToStart+7);

			for(j=0;j<2;j++)
			{
				for(i=7;i>=0;i--)
				{
					OLED_FontBuffer[0] = ((MF_FontBuffer[(j*4)]>>i)&0x01)*0xF0 + ((MF_FontBuffer[(j*4)+1]>>i)&0x01)*0x0F;
					OLED_FontBuffer[1] = ((MF_FontBuffer[(j*4)+2]>>i)&0x01)*0xF0 + ((MF_FontBuffer[(j*4)+3]>>i)&0x01)*0x0F;

					if((FontOptions & OLED_FONT_INVERSE) == OLED_FONT_INVERSE)
					{
						OLED_FontBuffer[0] = ~OLED_FontBuffer[0];
						OLED_FontBuffer[1] = ~OLED_FontBuffer[1];
					}
					OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
				}
			}
			return;

		case MF_ASCII_SIZE_8X16:
			LoopSize = 4;
			StartVal = 2;
			FontBase = StartVal;
			FontBaseOffset = 0;
			OLED_SetWindow(ColumnToStart, ColumnToStart+1, RowToStart, RowToStart+15);
			break;

		case MF_ASCII_SIZE_WA:
			LoopSize = 8;
			StartVal = 4;
			FontBase = StartVal;
			FontBaseOffset = 2;
			OLED_SetWindow(ColumnToStart, ColumnToStart+3, RowToStart, RowToStart+15);
			break;

		default:
			return;
	}

	//For the 8X16 and width adjusted fonts, the pixels are drawn out of order
	//For 8x16 fonts, the order of CalcFontBase is 8,0,12,4
	//for width adjusted fonts, the order of CalcFontBase is 18,2,22,6,26,10,30,14
	for(j=0;j<LoopSize;j++)
	{
		CalcFontBase = ((FontBase*4)+FontBaseOffset);

		for(i=7;i>=0;i--)
		{
			OLED_FontBuffer[0] = ((MF_FontBuffer[CalcFontBase]>>i)&0x01)*0xF0 + ((MF_FontBuffer[(CalcFontBase+1)]>>i)&0x01)*0x0F;
			OLED_FontBuffer[1] = ((MF_FontBuffer[(CalcFontBase+2)]>>i)&0x01)*0xF0 + ((MF_FontBuffer[(CalcFontBase+3)]>>i)&0x01)*0x0F;

			if((FontOptions & OLED_FONT_INVERSE) == OLED_FONT_INVERSE)
			{
				OLED_FontBuffer[0] = ~OLED_FontBuffer[0];
				OLED_FontBuffer[1] = ~OLED_FontBuffer[1];
			}
			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}

		if((j&0x01) == 0x01)
		{
			//j is odd
			FontBase += (StartVal+1);
		}
		else
		{
			//j is even
			FontBase -= StartVal;
		}
	}


	//OLED_WriteLine((Horiz*4)-2, Vert-1, (Horiz*4)-2, Vert+9, 1, 1);
	//OLED_WriteLine((Horiz*4+MenuItemSize*8)+1, Vert-1, (Horiz*4+MenuItemSize*8)+1, Vert+9, 1, 1);
	//OLED_WriteLine((Horiz*4)-2, Vert-1, (Horiz*4+MenuItemSize*8)+1, Vert-1, 1, 1);
	//OLED_WriteLine((Horiz*4)-2, Vert+9, (Horiz*4+MenuItemSize*8)+1, Vert+9, 1, 1);

	return;
}
#endif

void OLED_WriteMFString(uint8_t CharSize, const char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions)
{
	uint8_t StringSize;
	uint8_t i;

	StringSize = strlen(StringToWrite);

	for(i=0;i<StringSize;i++)
	{
		if(CharSize == MF_ASCII_SIZE_WA)
		{
			OLED_WriteMFChar(CharSize, StringToWrite[i], ColumnToStart+i*4, RowToStart, FontOptions);
		}
		else
		{
			OLED_WriteMFChar(CharSize, StringToWrite[i], ColumnToStart+i*2, RowToStart, FontOptions);
		}
	}

	return;
}


void OLED_WriteColumn(uint8_t *ColumnData, uint8_t ColumnHeight, uint8_t ColumnBrightness)
{
	uint8_t i;
	uint8_t CurrentColumnData;
	uint8_t OLED_FontBuffer[2];

	ColumnBrightness &= 0x0F;		//Column brightness must be <= 0x0F

	CurrentColumnData = ColumnData[0];
	for(i=0; i<ColumnHeight; i++)
	{
		OLED_FontBuffer[0] = ((((CurrentColumnData & 0x01)*ColumnBrightness) << 4) & 0xF0) | ((((CurrentColumnData >> 1) & 0x01)*ColumnBrightness) & 0x0F);
		OLED_FontBuffer[1] = (((((CurrentColumnData>>2) & 0x01)*ColumnBrightness) << 4) & 0xF0) | ((((CurrentColumnData>>3) & 0x01)*ColumnBrightness) & 0x0F);
		OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);

		if((i & 0x01) == 0x01)
		{
			//i is odd
			CurrentColumnData = ColumnData[(i/2)+1];
		}
		else
		{
			//i is even
			CurrentColumnData = CurrentColumnData >> 4;
		}
	}

	return;
}

/**Writes a column of data to the OLED
 *  ColumnData is formatted in the MF convention. ColumnData is always a multiple of four bytes long.
 *  Each bit in ColumnData represents one pixel. The MSB of each byte represents the bottom pixel in that byte.One full byte of ColumnData therefore represents eight vertical pixels.
 *  Each set of four ColumnData bytes represent one column of data that is eight pixels high by four pixels wide.
 *  The first byte of each set of four ColumnData bytes (byte 0, 4, 8, etc...) represent the left most vertical line. The other bytes in each set are arranged left to right.
 *  The first four bytes in ColumnData are the bottom pixels in the column.
 *
 *  PixelStart is a mask of the bits to be written. A zero in bit 0-3 of this mask will make that vertical pixel always zero.
 *
 *	StringOptions.PixelHeight sets the height of the column to be written.
 *  if StringOptions.FontOptions includes OLED_FONT_INVERSE, the pixels will be inverted before writing.
 *
 *  Note: the set window command needs to be called to set the proper window before this function is called.
 */
void OLED_WriteColumn2(uint8_t *ColumnData, uint8_t PixelStart, MF_StringOptions *StringOptions)
{
	uint8_t OLED_FontBuffer[2];
	int8_t i;
	int8_t j;
	uint8_t ColumnBrightness;				//TODO: make this part of StringOptions
	uint8_t ColumnHeight;					//The height of the current column to write
	uint8_t StopRow;

	ColumnBrightness = (StringOptions->Brightness) & 0x0F;
	StopRow = ((StringOptions->PixelHeight-1)/8)*4;
	//StopRow = (StringOptions->PixelHeight-1)/8;

	for(j=0; j <= StopRow ; j+=4)
	//for(j=0; j <= StopRow ; j++)
	{
		if(j == StopRow)
		{
			//ColumnHeight = StringOptions->PixelHeight - (2*j);
			ColumnHeight = ((8 - (StringOptions->PixelHeight & 0x07)) & 0x07);
		}
		else
		{
			//ColumnHeight = 8;
			ColumnHeight = 0;
		}
		printf("Col: %u\r\n", ColumnHeight);


		//for(i=ColumnHeight-1; i>=0; i--)
		for(i=7; i>=ColumnHeight; i--)
		{
			OLED_FontBuffer[0] = ((ColumnData[j]>>i) & 0x01)*(ColumnBrightness << 4) + ((ColumnData[j+1]>>i) & 0x01)*(ColumnBrightness);
			OLED_FontBuffer[1] = ((ColumnData[j+2]>>i) & 0x01)*(ColumnBrightness << 4) + ((ColumnData[j+3]>>i) & 0x01)*(ColumnBrightness);
			if((StringOptions->FontOptions & OLED_FONT_INVERSE) == OLED_FONT_INVERSE)
			{
				OLED_FontBuffer[0] = ~OLED_FontBuffer[0];
				OLED_FontBuffer[1] = ~OLED_FontBuffer[1];
			}

			//Mask bits that should always be zero
			OLED_FontBuffer[0] &= (((PixelStart & 0x01)*0xF0) + (((PixelStart >> 1) & 0x01)*0x0F));
			OLED_FontBuffer[1] &= ((((PixelStart >> 2) & 0x01)*0xF0) + (((PixelStart >> 3) & 0x01)*0x0F));

			OLED_SendCommand(OLED_WRITE_RAM, OLED_FontBuffer, 2);
		}
	}

	return;
}



//TODO: make this function wrap text???
void OLED_WriteMFString2(const char *StringToWrite, MF_StringOptions *StringOptions)
{
	int8_t i;
	uint8_t TotalPixelLength = 0;	//This variable tracks the total length (X direction) of the pixels written

	//In the current OLED configuration, the display is separated into 4 pixel columns (in the X direction)
	//An entire column slice (four X pixels by one Y pixel) must be written at the same time.

	//TODO: I can probably cut down on the number of variables used here eventually
	//Display Variables
	uint8_t CurrentColumn = 0;						//The current column on the OLED.
	uint8_t CurrentPixel = 0;						//The current pixel in the column. Each column is four pixels wide, so this variable is 0-3 only.
	uint8_t CurrentPixelInFont = 0;
	uint8_t VerticalPixelStart;						//The vertical location to start writing characters.
	uint8_t PixelData[8];							//Pixel data to be sent to the write column function.
	uint8_t MF_FontData[8];							//Pixel data received from the MF chip.
	uint8_t MaxPixelInFont;

	uint8_t FontDataLocation = 0;					//The location in the MF_FontData array. The font data is fetched in 4 byte chunks, so this variable should be 0-3 only.
	uint8_t FontDataOffset = 0;						//The offset from the start of the font in the MF chip memory.
	uint8_t PixelMask;								//The pixel mask for writing columns. This should always be 0x0F except for the first and last call to the write column function.

	//String variables
	uint8_t CharNumber = 0;							//The current character to be written to the OLED.
	uint8_t StringLength = strlen(StringToWrite);	//The total length of the string to be written in characters.

	//Initialize variables for the vertical position and height
	//The padding will be added at the end of this function, but some of the variables must be initialized first.
	//Also, since this is the first time we check the CharSize variable, add a check to return if this variable is invalid.
	VerticalPixelStart = (StringOptions->YStart) + (StringOptions->BottomPadding);
	if(StringOptions->CharSize == MF_ASCII_SIZE_5X7)
	{
		StringOptions->PixelHeight = 8;
		MaxPixelInFont = 5;
	}
	else if(StringOptions->CharSize == MF_ASCII_SIZE_7X8)
	{
		StringOptions->PixelHeight = 8;
		MaxPixelInFont = 7;
	}
	else if(StringOptions->CharSize == MF_ASCII_SIZE_8X16)
	{
		StringOptions->PixelHeight = 14;
		MaxPixelInFont = 8;
	}
	else if(StringOptions->CharSize == MF_ASCII_SIZE_WA)
	{
		StringOptions->PixelHeight = 14;
		//MaxPixelInFont is set below for each character
	}
	else
	{
		return;
	}
	/**Ending condition:
	 *  VerticalPixelStart is set to the vertical position to start writing text.
	 *  StringOptions->PixelHeight is set to the height of the characters without vertical padding.
	 */



	//Handle vertical padding
	//TODO: combine this into the main code?
	//TODO: make this happen last.
	/*if((StringOptions->FontOptions & OLED_FONT_INVERSE) == OLED_FONT_INVERSE)
	{
		//TODO: Fill top and bottom padding
	}

	VerticalPixelStart = (StringOptions->YStart) + (StringOptions->BottomPadding);
	StringOptions->PixelHeight = (StringOptions->BottomPadding) + (StringOptions->TopPadding);
	if(StringOptions->CharSize == MF_ASCII_SIZE_5X7)
	{
		StringOptions->PixelHeight += 7;
	}
	else if(StringOptions->CharSize == MF_ASCII_SIZE_7X8)
	{
		StringOptions->PixelHeight += 8;
	}
	else if( (StringOptions->CharSize == MF_ASCII_SIZE_8X16) || (StringOptions->CharSize == MF_ASCII_SIZE_WA))
	{
		StringOptions->PixelHeight += 16;
	}*/
	/**Ending condition:
	 *  Display: if inverse font is requested, padding above and below the text is active
	 *
	 *  VerticalPixelStart is set to the vertical position to start writing text
	 *  StringOptions->PixelHeight is set to the total height of the string (including padding)
	 */



	//Handle initial padding padding in X
	CurrentColumn = (StringOptions->XStart) / 4;
	StringOptions->StartColumn = CurrentColumn;
	CurrentPixel = StringOptions->XStart - (StringOptions->StartColumn*4);		//current pixel

	//The bits before the start of the padding should always be zero
	PixelMask = ((0x0F << CurrentPixel) & 0x0F);

	//CurrentPixel will always be 0-3 when starting this for loop
	for(i=0;i<StringOptions->StartPadding;i++)
	{
		PixelData[CurrentPixel] = 0x00;
		CurrentPixel++;
		TotalPixelLength++;

		if(CurrentPixel >= 4)
		{
			OLED_SetWindow(CurrentColumn, CurrentColumn, VerticalPixelStart, VerticalPixelStart+StringOptions->PixelHeight);
			OLED_WriteColumn2(PixelData, PixelMask, StringOptions);
			PixelMask = 0x0F;		//We only mask the first and last column
			CurrentPixel = 0;
			CurrentColumn++;		//TODO: Add a check to make sure we don't exceed the maximum column?
		}
	}
	/**Ending condition:
	 *  Display: if inverse font is requested, the initial padding is active
	 *
	 *  CurrentColum is the column where the first character starts
	 *  CurrentPixel is the pixel in that column (0-3) where the first character starts
	 *  TotalPixelLength is equal to the length of the initial padding
	 *  PixelData is filled with any initial pixels needed, otherwise it is filled with zeros
	 */


	//Write the actual string characters to the display.
	if((StringOptions->CharSize == MF_ASCII_SIZE_7X8) || (StringOptions->CharSize == MF_ASCII_SIZE_5X7) || (StringOptions->CharSize == MF_ASCII_SIZE_8X16) || (StringOptions->CharSize == MF_ASCII_SIZE_WA))
	{

		//Setup initial variables based on the char size
		MF_GetAsciiChar_4B_2(StringOptions->CharSize, StringToWrite[CharNumber], FontDataOffset, MF_FontData);
		if(StringOptions->CharSize == MF_ASCII_SIZE_WA)
		{
			MaxPixelInFont = MF_GetWACharWidth(StringToWrite[CharNumber]);
			printf("CharWidth for %c is %u\r\n", StringToWrite[CharNumber], MaxPixelInFont);
		}

		FontDataLocation = 0;
		CurrentPixelInFont = 0;
		while(1)
		{

			if(CurrentPixel == 4)
			{
				//The PixelData buffer is full, write data the the OLED
				OLED_SetWindow(CurrentColumn, CurrentColumn, VerticalPixelStart, VerticalPixelStart+StringOptions->PixelHeight);
				OLED_WriteColumn2(PixelData, PixelMask, StringOptions);
				if(PixelMask != 0x0F)
				{
					//We only mask the first and last column)
					PixelMask = 0x0F;
				}
				CurrentPixel = 0;
				CurrentColumn++;		//TODO: Add a check to make sure we don't exceed the maximum column
			}
			else if(CurrentPixelInFont == (MaxPixelInFont + StringOptions->CharacterSpacing))
			{
				//Go to the next character
				CurrentPixelInFont = 0;

				printf("end of char\r\n");

				if(FontDataLocation != 0)
				{
					CharNumber++;
					if(CharNumber >= StringLength) break;
					FontDataOffset = 0;
					FontDataLocation = 0;
					MF_GetAsciiChar_4B_2(StringOptions->CharSize, StringToWrite[CharNumber], FontDataOffset, MF_FontData);
					if(StringOptions->CharSize == MF_ASCII_SIZE_WA)
					{
						MaxPixelInFont = MF_GetWACharWidth(StringToWrite[CharNumber]);
						printf("CharWidth for %c is %u\r\n", StringToWrite[CharNumber], MaxPixelInFont);
					}
				}
			}
			else if(FontDataLocation > 3)
			{
				//We need to get more data from the MF chip.
				if( ((StringOptions->CharSize == MF_ASCII_SIZE_WA) && (FontDataOffset < 16)) || ((StringOptions->CharSize != MF_ASCII_SIZE_WA) &&  (FontDataOffset < 4)) )
				{
					//Get the next 4 bytes from the current character
					FontDataOffset += 4;
				}
				else
				{
					//Go to the next character
					CharNumber++;
					if(CharNumber >= StringLength) break;

					if(StringOptions->CharSize == MF_ASCII_SIZE_WA)
					{
						MaxPixelInFont = MF_GetWACharWidth(StringToWrite[CharNumber]);
						printf("CharWidth for %c is %u\r\n", StringToWrite[CharNumber], MaxPixelInFont);
					}

					FontDataOffset = 0;
				}
				FontDataLocation = 0;
				MF_GetAsciiChar_4B_2(StringOptions->CharSize, StringToWrite[CharNumber], FontDataOffset, MF_FontData);
			}
			else if((CurrentPixelInFont < (MaxPixelInFont + StringOptions->CharacterSpacing)) && (CurrentPixelInFont >= MaxPixelInFont))
			{
				//We have written the entire character, but the we need to write spacing between the characters.
				PixelData[CurrentPixel] = 0x00;
				if( (StringOptions->CharSize == MF_ASCII_SIZE_8X16) || (StringOptions->CharSize == MF_ASCII_SIZE_WA))
				{
					PixelData[CurrentPixel+4] = 0x00;
				}
				CurrentPixel++;
				TotalPixelLength++;
				CurrentPixelInFont++;
			}
			else
			{
				//Put the next line of the character data in the OLED buffer.
				PixelData[CurrentPixel] = MF_FontData[FontDataLocation];
				if( (StringOptions->CharSize == MF_ASCII_SIZE_8X16) || (StringOptions->CharSize == MF_ASCII_SIZE_WA))
				{
					PixelData[CurrentPixel+4] = MF_FontData[FontDataLocation+4];
				}
				CurrentPixelInFont++;
				FontDataLocation++;
				CurrentPixel++;
				TotalPixelLength++;
			}
		}
	}

	//Handle final padding padding in X
	for(i=0;i<StringOptions->EndPadding;i++)
	{
		if(CurrentPixel >= 4)
		{
			OLED_SetWindow(CurrentColumn, CurrentColumn, VerticalPixelStart, VerticalPixelStart+StringOptions->PixelHeight);
			OLED_WriteColumn2(PixelData, PixelMask, StringOptions);
			CurrentPixel = 0;
			CurrentColumn++;		//Add a check to make sure we don't exceed the maximum column
		}
		PixelData[CurrentPixel] = 0;
		PixelData[CurrentPixel+4] = 0;
		CurrentPixel++;
		TotalPixelLength++;
	}

	if(CurrentPixel != 0)
	{
		PixelMask = 0;
		for(i=CurrentPixel;i<4;i++)
		{
			PixelData[i] = 0;
			PixelMask |= (1<<i);
		}
		//Mask the unused pixels at the end of the last column
		PixelMask = ~PixelMask;
		OLED_SetWindow(CurrentColumn, CurrentColumn, VerticalPixelStart, VerticalPixelStart+StringOptions->PixelHeight);
		OLED_WriteColumn2(PixelData, PixelMask, StringOptions);
		CurrentColumn++;
	}

	CurrentColumn--;
	/**Ending condition:
	 *  Display: if inverse font is requested, the final padding is active
	 *
	 *  CurrentColum is the last column written
	 *  TotalPixelLength is the length of the string written in pixels, including the pre and post padding, but not including the whitespace(if any) in the first and last column.
	 */

	StringOptions->PixelLength = TotalPixelLength;
	StringOptions->EndColumn = CurrentColumn;

	return;
}


/**Gets four bytes of data from the MF chip
 * If CharSize is 5x7 or 7x8, CharArray is 4 bytes long, if CharSize is 8x16 or WA, CharArray is 8 bytes long
 * CharStartByte is how many bytes to offset from the begining of the character data.
 *   Note:	for a WA character, the first two bytes are not part of the bitmap.
 * 			This function takes this into account, and CharStartByte should be called the same way as the other sizes.
 */
void MF_GetAsciiChar_4B_2(uint8_t CharSize, char CharToGet, uint8_t CharStartByte, uint8_t *CharArray)
{
	if(CharSize == MF_ASCII_SIZE_8X16)
	{
		MF_GetAsciiChar_4B(CharSize, CharToGet, CharStartByte+8, CharArray);
		MF_GetAsciiChar_4B(CharSize, CharToGet, CharStartByte, &CharArray[4]);
	}
	else if (CharSize == MF_ASCII_SIZE_WA)
	{
		MF_GetAsciiChar_4B(CharSize, CharToGet, CharStartByte+18, CharArray);
		MF_GetAsciiChar_4B(CharSize, CharToGet, CharStartByte+2, &CharArray[4]);
	}
	else
	{
		MF_GetAsciiChar_4B(CharSize, CharToGet, CharStartByte, CharArray);
	}
	return;
}


uint8_t MF_GetWACharWidth(char CharToGet)
{
	uint32_t MF_CharAddress;
	uint8_t DataToSend[5];
	uint8_t DataToReceive[2];

	MF_CharAddress = MF_GetCharAddress(MF_ASCII_SIZE_WA, CharToGet);

	DataToSend[0] = 0x0B;
	DataToSend[1] = (uint8_t)((MF_CharAddress >> 16) & 0xFF);
	DataToSend[2] = (uint8_t)((MF_CharAddress >> 8) & 0xFF);
	DataToSend[3] = (uint8_t)(MF_CharAddress & 0xFF);
	DataToSend[4] = 0x00;

	OLED_MF_Select(1);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP0, DataToSend, 5);
	Chip_SSP_ReadFrames_Blocking(LPC_SSP0, DataToReceive, 2);
	OLED_MF_Select(0);

	return DataToReceive[1];
}


void OLED_WriteMFString_WA(uint8_t CharSize, const char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions)
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


uint32_t MF_GetCharAddress(uint8_t CharSize, char CharToGet)
{
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
			return 0;
		}
		return Address;
	}

	return 0;
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

void OLED_WriteMF_UInt(uint8_t CharSize, uint32_t NumberToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions, uint8_t Padding)
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

	//TODO: add a check to make sure the string does not get too big...
	//TODO: add a check to see if we need to pad
	if(Padding > isStarted)
	{
		digit = Padding-isStarted-1;
		for(i=isStarted; i>=0; i--)
		{
			NumberString[i+digit] = NumberString[i];
		}
		for(i=0;i<digit;i++)
		{
			NumberString[i] = '0';
		}
		NumberString[Padding] = '\0';
	}
	else
	{
		NumberString[isStarted+1] = '\0';
	}

	OLED_WriteMFString(CharSize, NumberString, ColumnToStart, RowToStart, FontOptions);
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
	//int16_t tempval;		//TODO: combine this with j?
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



//Note: this only works for horizontal and vertical lines so far
//TODO: line weight and pattern are not implemented
//TODO: Implement the line weight with a wrapper and multiple calls to this function -- this will not work for vertical lines, this will probably have to be implemented internally
void OLED_WriteLine2(MF_LineOptions *TheLine)
{
	uint8_t i;
	uint8_t j;
	uint8_t k;
	uint8_t OLED_PixelBuffer[4];
	uint8_t PixelBrightness = 0x0F;		//TODO: make this an input later...

	uint8_t ModifiedPattern;

	j = 0;

	if(TheLine->XStart == TheLine->XEnd)
	{
		//Vertical line
		OLED_SetWindow(TheLine->XStart/4, TheLine->XEnd/4, TheLine->YStart, TheLine->YEnd);

		OLED_PixelBuffer[2] = 0x00;
		OLED_PixelBuffer[3] = 0x00;

		switch(TheLine->XStart & 0x03)
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

		for(i=0;i<(TheLine->YEnd-TheLine->YStart+1);i++)
		{
			if( ((TheLine->LinePattern >> (i & 0x07)) & 0x01) == 0)
			{
				OLED_SendCommand(OLED_WRITE_RAM, &OLED_PixelBuffer[2], 2);
			}
			else
			{
				OLED_SendCommand(OLED_WRITE_RAM, OLED_PixelBuffer, 2);
			}
		}
	}
	else if(TheLine->YStart == TheLine->YEnd)
	{
		//Horizontal line

		OLED_SetWindow(TheLine->XStart/4, TheLine->XEnd/4, TheLine->YStart, TheLine->YEnd);
		ModifiedPattern = ((TheLine->LinePattern) >> (4 - (TheLine->XStart & 0x03))) + ((TheLine->LinePattern) << (8-(4 -( TheLine->XStart & 0x03))));
		//printf("mod: %u, Pattern: 0x%02X, mod 0x%02X\r\n", (TheLine->XStart & 0x03), TheLine->LinePattern, ModifiedPattern);

		j = (TheLine->XEnd/4) - (TheLine->XStart/4);	//The total number of columns to write

		for(i=0; i<=j; i++)
		{
			if((i & 0x01) == 0x01)
			{
				OLED_PixelBuffer[0] = (((ModifiedPattern >> 0) & 0x01) * (PixelBrightness << 4)) + (((ModifiedPattern >> 1) & 0x01) * PixelBrightness);
				OLED_PixelBuffer[1] = (((ModifiedPattern >> 2) & 0x01) * (PixelBrightness << 4)) + (((ModifiedPattern >> 3) & 0x01) * PixelBrightness);
			}
			else
			{
				OLED_PixelBuffer[0] = (((ModifiedPattern >> 4) & 0x01) * (PixelBrightness << 4)) + (((ModifiedPattern >> 5) & 0x01) * PixelBrightness);
				OLED_PixelBuffer[1] = (((ModifiedPattern >> 6) & 0x01) * (PixelBrightness << 4)) + (((ModifiedPattern >> 7) & 0x01) * PixelBrightness);
			}


			if(i == 0)
			{
				//Mask pixels in the first column
				switch(TheLine->XStart & 0x03)
				{
				case 0:
					break;

				case 1:
					OLED_PixelBuffer[0] &= 0x0F;
					OLED_PixelBuffer[1] &= 0xFF;
					break;

				case 2:
					OLED_PixelBuffer[0] &= 0x00;
					OLED_PixelBuffer[1] &= 0xFF;
					break;

				case 3:
					OLED_PixelBuffer[0] &= 0x00;
					OLED_PixelBuffer[1] &= 0x0F;
					break;
				}
			}

			if(i == j)
			{
				//Mask pixels in the last column
				switch((TheLine->XEnd+1) & 0x03)
				{
				case 0:
					break;

				case 1:
					OLED_PixelBuffer[0] &= 0xF0;
					OLED_PixelBuffer[1] &= 0x00;
					break;

				case 2:
					OLED_PixelBuffer[0] &= 0xFF;
					OLED_PixelBuffer[1] &= 0x00;
					break;

				case 3:
					OLED_PixelBuffer[0] &= 0xFF;
					OLED_PixelBuffer[1] &= 0xF0;
					break;
				}
			}
			OLED_SendCommand(OLED_WRITE_RAM, OLED_PixelBuffer, 2);
			printf("Writing 0: 0x%02X, 1: 0x%02X\r\n", OLED_PixelBuffer[0], OLED_PixelBuffer[1]);
		}
	}
	else
	{
		//Uh-oh
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

	}

	return;
}



