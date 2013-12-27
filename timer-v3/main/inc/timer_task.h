/*
 * timer_task.h
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#ifndef TIMER_TASK_H_
#define TIMER_TASK_H_

#define TIMER_TASK_CMD_START	0
#define TIMER_TASK_CMD_STOP		1
#define TIMER_TASK_CMD_TICK		2




#define TIMER_TASK_TYPE_TIME_EVENT			0x01		//Turn the output on or off at a certain time
#define TIMER_TASK_TYPE_REPEATING_EVENT		0x02		//Turn the output on or off on a certain interval (for this type, only one event can be specified)
#define TIMER_TASK_TYPE_SUN_EVENT			0x03		//Turn the output on or off based on sunrise/sunset time
#define TIMER_TASK_TYPE_STEADY_EVENT		0x04		//The output is always on or off (for this type, only one event can be specified)


/* EventType:
 *
 * EventTime
 *  For timed events:
 *  	EventTime[0]: Bitmapped DOW for event, defined as 1 << DOW. 0xFF would represent all days of the week
 * 		EventTime[1]: Hour of the event (in 24 hour format)
 * 		EventTime[2]: Minute of the event
 *
 *
 * EventOutputState:
 * 	0 - Output off
 * 	1 - Output on
 */
typedef struct {
	uint8_t EventType;
	uint8_t EventTime[3];
	uint8_t EventOutputState;
} TimerEvent;

uint8_t TimerGetOutputState(void);

void TimerGetEvent(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData);
void TimerSetEvent(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData);

uint8_t InitTimerTask(void);
void TimerTask(void *pvParameter);


void TimerSetOutput(uint8_t OutputNumber, uint8_t OutputState);




#endif /* TIMER_TASK_H_ */
