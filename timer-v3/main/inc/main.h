/*
 * main.h
 *
 *  Created on: Nov 4, 2013
 *      Author: pat
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "libeeprom.h"

//FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//USB includes
#include "app_usbd_cfg.h"
#include "cdc_vcom.h"

//Hardware specific includes
#include "board.h"

#include "config.h"
#include "hardware.h"
#include "command.h"
#include "commands.h"
#include "i2c.h"
#include "cdc_vcom.h"
#include "ds3232m.h"
#include "dfu.h"
#include "NHD28.h"
#include "iap.h"
#include "timer_task.h"
#include "display_task.h"
#include "sun.h"


//#define ERROR_CODE_1			1
//#define ERROR_CODE_2			2
//#define ERROR_CODE_3			3
//#define ERROR_CODE_4			4
//#define ERROR_CODE_5			5
//#define ERROR_CODE_6			6
//#define ERROR_CODE_7			7

//EEPROM addresses map
#define EEPROM_ADDRESS_HW_STATUS			0x00
#define EEPROM_ADDRESS_DISPLAY_STATUS		0x01
#define EEPROM_ADDRESS_TIMER_STATUS			0x02
#define EEPROM_ADDRESS_IDLE_TO_DIM_TIMEOUT	0x03
#define EEPROM_ADDRESS_MENU_TO_IDLE_TIMEOUT	0x04
#define EEPROM_ADDRESS_OVERRIDE_TIMEOUT		0x05
#define EEPROM_ADDRESS_BRIGHT_DIMMING_VAL	0x06
#define EEPROM_ADDRESS_DIM_DIMMING_VAL		0x07

#define EEPROM_ADDRESS_LAT_LHS_MSB			0x08
#define EEPROM_ADDRESS_LAT_LHS_LSB			0x09
#define EEPROM_ADDRESS_LAT_RHS_MSB			0x0A
#define EEPROM_ADDRESS_LAT_RHS_LSB			0x0B

#define EEPROM_ADDRESS_LONG_LHS_MSB			0x0C
#define EEPROM_ADDRESS_LONG_LHS_LSB			0x0D
#define EEPROM_ADDRESS_LONG_RHS_MSB			0x0E
#define EEPROM_ADDRESS_LONG_RHS_LSB			0x0F

#define EEPROM_ADDRESS_OVERRIDE_OUTPUTS		0x10


#define EEPROM_ADDRESS_TIMER_EVENTS			0x80


extern uint32_t ISP_Request;

xTaskHandle TaskList[5];

//TODO: move these to their specific .h files?
xQueueHandle xUSBCharReceived;
//xQueueHandle xButtonPress;
extern xQueueHandle xDisplayCommands;
extern xQueueHandle xTimerCommands;








#endif /* MAIN_H_ */
