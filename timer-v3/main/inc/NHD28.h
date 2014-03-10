/*
 * NHD28.h
 *
 *  Created on: Nov 18, 2013
 *      Author: pat
 */

#ifndef NHD28_H_
#define NHD28_H_

#define OLED_CS_PORT		0
#define OLED_CS_PIN			22
#define OLED_DC_PORT		0
#define OLED_DC_PIN			23
#define OLED_RST_PORT		1
#define OLED_RST_PIN		29

#define OLED_MF_CS_PORT		1
#define OLED_MF_CS_PIN		21

#undef OLED_COLUMN_INCREMENT

//Commands
#define OLED_ENABLE_GRAYSCALE_TABLE		0x00
#define OLED_SET_COLUMN_ADDRESS			0x15
#define OLED_WRITE_RAM					0x5C
#define OLED_READ_RAM					0x5D
#define OLED_SET_ROW_ADDRESS			0x75
#define OLED_SET_REMAP					0xA0
#define OLED_SET_DISPLAY_START_LINE		0xA1
#define OLED_SET_DISPLAY_OFFSET			0xA2
#define OLED_SET_DISPLAY_MODE_OFF		0xA4
#define OLED_SET_DISPLAY_MODE_ON		0xA5	//Note: this means all the pixels are on, normal operation should use 0xA6 or 0xA7
#define OLED_SET_DISPLAY_MODE_NORMAL	0xA6
#define OLED_SET_DISPLAY_MODE_INVERSE	0xA7
#define OLED_ENABLE_PARTIAL_DISPLAY		0xA8
#define OLED_EXIT_PARTIAL_DISPLAY		0xA9
#define OLED_FUNCTION_SELECTION			0xAB
#define OLED_SLEEP_MODE_ON				0xAE
#define OLED_SLEEP_MODE_OFF				0xAF
#define OLED_SET_PHASE_LENGTH			0xB1
#define OLED_SET_CLK_DIVIDER			0xB3
#define OLED_DISPLAY_ENHANCE_A			0xB4
#define OLED_SET_GPIO					0xB5
#define OLED_SET_SECOND_PRECHARGE		0xB6
#define OLED_SET_GRAYSCALE_TABLE		0xB8
#define OLED_SELECT_DEFAULT_TABLE		0xB9
#define OLED_SET_PRECHARGE_VOLTAGE		0xBB
#define OLED_SET_VCOMH					0xBE
#define OLED_SET_CONTRAST_CURRENT		0xC1
#define OLED_MASTER_CONTRAST_CONTROL	0xC7
#define OLED_SET_MUX_RATIO				0xCA
#define OLED_DISPLAY_ENHANCE_B			0xD1
#define OLED_COMMAND_LOCK				0xFD

#define MF_ASCII_SIZE_5X7				0x00
#define MF_ASCII_SIZE_7X8				0x01
#define MF_ASCII_SIZE_8X16				0x03
#define MF_ASCII_SIZE_WA				0x04

//Status of OLED
#define OLED_STATUS_ORENTATION_MASK		0x01
#define OLED_STATUS_ORENTATION_UP		0x00
#define OLED_STATUS_ORENTATION_DOWN		0x01

//Font options
#define OLED_FONT_NORMAL				0x00	//Normal text
#define OLED_FONT_INVERSE				0x01	//Black text on a lighted background
#define OLED_FONT_BOX					0x02	//Draws a box around the text. This will not do anything if OLED_FONT_INVERSE is also requested



//Minimum and maximum pixel numbers
#define OLED_X_MIN		0
#define OLED_X_MAX		255
#define OLED_Y_MIN		0
#define OLED_Y_MAX		63

//Font options struct
typedef struct
{
	uint8_t CharSize;			/** The size of the characters to display. See the MF_ASCII_SIZE_ defines for a list of options */
	uint8_t XStart;				/** The starting X (horizontal) location of the string in pixels. X ranges from 0-255. This is where the padding(if any) will start */
	uint8_t YStart;				/** The starting Y (vertical) location of the string in pixels. Y ranges from 0-63. This is where the padding(if any) will start */
	uint8_t StartPadding;		/** The number of pixels to append to the start of the string. This number is included in the PixelLength return*/
	uint8_t EndPadding;			/** The number of pixels to append to the end of the string. This number is included in the PixelLength return*/
	uint8_t TopPadding;			/** The number of pixels to append to the top of the string. This number is included in the PixelHeight return*/
	uint8_t BottomPadding;		/** The number of pixels to append to the bottom of the string. This number is included in the PixelHeight return*/
	uint8_t CharacterSpacing;	/** Spacing (in pixels) between characters */
	uint8_t Brightness;			/** The brightness of the active pixels. This value should range from 0 (dim) to 15 (bright). Note: this appears to actually be the brightness of the black, so a value of 0x0F is black and a value of 0 is white*/
	uint8_t FontOptions;		/** Input to the write function. See the OLED_FONT_ defines for a list of options */
	uint8_t StartColumn;		/** Set by the write function to the starting column of the string, data passed to the write function in this variable is ignored and overwritten */
	uint8_t EndColumn;			/** Set by the write function to the ending column of the string, data passed to the write function in this variable is ignored and overwritten */
	uint8_t PixelLength;		/** Set by the write function to the pixel length of the string, the pixel length includes any padding, but does not include unused pixels in the starting and ending row. Data passed to the write function in this variable is ignored and overwritten */
	uint8_t PixelHeight;		/** Set by the write function to the height of the string in pixels. This includes padding on the top and bottom of the string */
} MF_StringOptions;

//Font options struct
typedef struct
{
	uint8_t XStart;				/** The X (horizontal) coordinate of the starting point of the line */
	uint8_t XEnd;				/** The X (horizontal) coordinate of the ending point of the line */
	uint8_t YStart;				/** The Y (vertical) coordinate of the starting point of the line */
	uint8_t YEnd;				/** The Y (vertical) coordinate of the ending point of the line */
	uint8_t LineWeight;			/** The thickness of the line to be drawn in pixels. The center of the line will be specified by the X and Y coordinates. */
	uint8_t LinePattern;		/** The pattern to draw on the line. Each bit in this variable represents one pixel in the line. To draw a solid line, set this to 0xFF. */
	uint8_t LineOptions;		/** Line options for the line to draw. */
} MF_LineOptions;



void OLED_Init(void);

void OLED_SetStatus(uint8_t StatusMask, uint8_t StatusValue);
uint8_t OLED_GetStatus(void);

/** 1 to select, 0 to deselect */
void OLED_Select(uint8_t sel);
void OLED_MF_Select(uint8_t sel);

/** 1 for data, 0 for command */
void OLED_DC(uint8_t sel);

/** 1 for reset, 0 for unreset */
void OLED_Reset(uint8_t rst);

void OLED_SendCommand(uint8_t Command, uint8_t *args, uint8_t arg_length);

void OLED_ClearDisplay(void);
void OLED_FillDisplay(uint8_t *FillPattern, uint8_t FillPaternLength);

void OLED_SetWindow(uint8_t StartColumn, uint8_t EndColumn, uint8_t StartRow, uint8_t EndRow);
void OLED_ClearWindow(uint8_t StartColumn, uint8_t EndColumn, uint8_t StartRow, uint8_t EndRow);

//void OLED_FillDisplay(void);
//void OLED_CheckerboardDisplay(void);
void OLED_WriteMFChar(uint8_t CharSize, const char CharToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions);
void OLED_WriteMFString(uint8_t CharSize, const char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions);




//This function gets a single pixel column (constant X slice) of the font data from the MF chip
//For 5x7 and 7x8, the returned CharArray is 1 byte long, for 8x16 and width adjusted, the CharArray is 2 bytes.
//Column starts at zero
//TODO: Maybe give this function a less dumb name later?
void MF_GetASCIICharPixelColumn(const char CharToGet, uint8_t *CharArray, uint8_t Column, uint8_t CharSize);

/*This function takes ColumnData and writes it to the OLED display
 * 	The window and ram pointer must be set up before calling this function.
 *  ColumnHeight is the height (Y direction) of the column to be written. A ColumnHeight of 1 will write one pixel in Y by four pixels in X
 *  The column data must be four bits per ColumnHeight. Bit zero is the first (leftmost) pixel in the column.
 *  ColumnBrightness is the brightness of the column. THis variable should range between 0(0x00) (dim) and 15(0x0F) (bright).
 *
 *  Note: The ColumnData format is not the same as how the data is written to the device. This function rearranged the data before writing.
 */
void OLED_WriteColumn(uint8_t *ColumnData, uint8_t ColumnHeight, uint8_t ColumnBrightness);




void OLED_WriteColumn2(uint8_t *ColumnData, uint8_t PixelStart, MF_StringOptions *StringOptions);

void OLED_WriteMFString2(const char *StringToWrite, MF_StringOptions *StringOptions);
void OLED_WriteMF_UInt2(uint32_t NumberToWrite, uint8_t Padding, MF_StringOptions *StringOptions);
void OLED_WriteMF_Int2(void* NumberToWrite, uint8_t Padding, uint8_t isSigned, MF_StringOptions *StringOptions);


void OLED_WriteMF_UInt(uint8_t CharSize, uint32_t NumberToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions, uint8_t Padding);		//TODO: check if this breaks for 0
void OLED_WriteMF_Int(uint8_t CharSize, int32_t NumberToWrite, uint8_t ColumnToStart, uint8_t RowToStart);

void OLED_WriteMFString_WA(uint8_t CharSize, const char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions);

void MF_GetAsciiChar(uint8_t CharSize, char CharToGet, uint8_t *CharArray);

void MF_GetAsciiChar_4B(uint8_t CharSize, char CharToGet, uint8_t CharStartByte, uint8_t *CharArray);

void MF_GetAsciiChar_4B_2(uint8_t CharSize, char CharToGet, uint8_t CharStartByte, uint8_t *CharArray);


uint32_t MF_GetCharAddress(uint8_t CharSize, char CharToGet);
uint8_t MF_GetWACharWidth(char CharToGet);


void OLED_WriteMFString_Q(uint8_t CharSize, char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart);

void OLED_DisplayRotation(uint8_t Rotation);
void OLED_DisplayContrast(uint8_t Contrast);

void OLED_WriteLine(uint8_t X_start, uint8_t Y_start, uint8_t X_end, uint8_t Y_end, uint8_t weight, uint8_t prop);

void OLED_WriteLine2(MF_LineOptions *TheLine);

#endif /* NHD28_H_ */
