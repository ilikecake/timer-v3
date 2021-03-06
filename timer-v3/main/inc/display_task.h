/*
 * display_task.h
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#ifndef DISPLAY_TASK_H_
#define DISPLAY_TASK_H_



//TODO: put in a list of commands to send with the following struct
#define OLED_CMD_DISPLAY_OFF			0x00
#define OLED_CMD_DISPLAY_ON				0x01
#define OLED_CMD_CLEAR_DISPLAY			0x02
#define OLED_CMD_FILL_DISPLAY			0x03
#define OLED_CMD_WRITE_STRING			0x04		//TODO: make more commands for different fonts, etc...
//The commands above this are probably not needed. Any string display or modification of the OLED should happen in inside the display task.

#define OLED_CMD_BUTTON_IN				0x05
#define OLED_CMD_TIME_IN				0x06

#define TIMEOUT_TYPE_MENU_TO_IDLE		0x01
#define TIMEOUT_TYPE_IDLE_TO_DIM		0x02
#define TIMEOUT_TYPE_OVERRIDE			0x03

#define DIMMING_TYPE_BRIGHT				0x01
#define DIMMING_TYPE_DIM				0x02

//A struct to pass commands to the display task
typedef struct
{
	uint8_t CommandName;
	uint8_t CommandData[5];
	char CommandCharData[16];		//TODO: Do i need this?
} DisplayCommand;

//FreeRTOS queue to receive commands for the display task
xQueueHandle xDisplayCommands;

typedef struct {
	void (*handler) (uint8_t, uint8_t, uint8_t);
	const char *name;
	uint8_t UpItem;
	uint8_t DownItem;
	uint8_t LeftItem;
	uint8_t RightItem;
	uint8_t CenterItem;
	uint8_t X_Start;
	uint8_t Y_Start;
} MenuItem;





void DisplayTaskInit(void);

//uint8_t GetTimeout(uint8_t TimeoutToGet);

void SetTimeout(uint8_t TimeoutToSet, uint8_t TimeoutVal);
void SetDimming(uint8_t DimmingType, uint8_t DimmingVal);	//TODO: Refactor this so that is is more distinct from the below command

void DisplayDimming(uint8_t DimmingVal);

void DisplayTask(void *pvParameters);

#endif /* DISPLAY_TASK_H_ */
