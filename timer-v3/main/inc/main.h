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

//FreeRTOS queue to send received data to the command interpreter
xQueueHandle xUSBCharReceived;
xQueueHandle xOLEDCommands;


#endif /* MAIN_H_ */
