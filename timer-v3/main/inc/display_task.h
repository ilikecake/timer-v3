/*
 * display_task.h
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#ifndef DISPLAY_TASK_H_
#define DISPLAY_TASK_H_


typedef struct {
	void (*handler) (uint8_t, uint8_t);
	const char *name;
	uint8_t UpItem;
	uint8_t DownItem;
	uint8_t LeftItem;
	uint8_t RightItem;
	uint8_t CenterItem;
} MenuItem;





void DisplayTaskInit(void);

void DisplayTask(void *pvParameters);

#endif /* DISPLAY_TASK_H_ */
