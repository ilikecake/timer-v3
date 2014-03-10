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

extern uint32_t ISP_Request;

xTaskHandle TaskList[5];

//TODO: move these to their specific .h files?
xQueueHandle xUSBCharReceived;
//xQueueHandle xButtonPress;
extern xQueueHandle xDisplayCommands;
extern xQueueHandle xTimerCommands;








#endif /* MAIN_H_ */
