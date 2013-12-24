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
#define OLED_FONT_NORMAL				0x00
#define OLED_FONT_INVERSE				0x01



//TODO: put in a list of commands to send with the following struct
#define OLED_CMD_DISPLAY_OFF			0x00
#define OLED_CMD_DISPLAY_ON				0x01
#define OLED_CMD_CLEAR_DISPLAY			0x02
#define OLED_CMD_FILL_DISPLAY			0x03
#define OLED_CMD_WRITE_STRING			0x04		//TODO: make more commands for different fonts, etc...
#define OLED_CMD_BUTTON_IN				0x05
#define OLED_CMD_TIME_IN				0x06

//A struct to pass messages to the OLED task
typedef struct
{
	uint8_t CommandName;
	uint8_t CommandData[5];
	char CommandCharData[16];
} OLED_Command;

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

//void OLED_FillDisplay(void);
//void OLED_CheckerboardDisplay(void);
void OLED_WriteMFChar(uint8_t CharSize, char CharToWrite, uint8_t ColumnToStart, uint8_t RowToStart);
void OLED_WriteMFString(uint8_t CharSize, char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart);

void OLED_WriteMF_UInt(uint8_t CharSize, uint32_t NumberToWrite, uint8_t ColumnToStart, uint8_t RowToStart);		//TODO: check if this breaks for 0
void OLED_WriteMF_Int(uint8_t CharSize, int32_t NumberToWrite, uint8_t ColumnToStart, uint8_t RowToStart);

void OLED_WriteMFString_WA(uint8_t CharSize, char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart, uint8_t FontOptions);

void MF_GetAsciiChar(uint8_t CharSize, char CharToGet, uint8_t *CharArray);

void MF_GetAsciiChar_4B(uint8_t CharSize, char CharToGet, uint8_t CharStartByte, uint8_t *CharArray);

void OLED_WriteMFString_Q(uint8_t CharSize, char *StringToWrite, uint8_t ColumnToStart, uint8_t RowToStart);

void OLED_DisplayRotation(uint8_t Rotation);
void OLED_DisplayContrast(uint8_t Contrast);

void OLED_WriteLine(uint8_t X_start, uint8_t Y_start, uint8_t X_end, uint8_t Y_end, uint8_t weight, uint8_t prop);


#endif /* NHD28_H_ */
