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

//FreeRTOS queue to send received data to the command interpreter
xTaskHandle TaskList[4];

//TODO: move these to their specific .h files?
xQueueHandle xUSBCharReceived;
extern xQueueHandle xDisplayCommands;
extern xQueueHandle xTimerCommands;

//Defines for the OLED display task
#define MENU_TO_IDLE_TIME			4000
#define IDLE_TO_DIM_TIME			3000

//TODOL Change these to pixels instead of columns
#define IDLE_TIME_ROW				37
#define IDLE_TIME_COLUMN			4
#define IDLE_DATE_ROW				37
#define IDLE_DATE_COLUMN			35
#define IDLE_STATUS_ROW				19
#define IDLE_STATUS_COLUMN			13

#define DISPLAY_STATUS_IDLE_DIM		0x00
#define DISPLAY_STATUS_IDLE_BRIGHT	0x01
#define DISPLAY_STATUS_MENU			0x02




#endif /* MAIN_H_ */
