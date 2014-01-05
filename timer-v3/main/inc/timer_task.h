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
#define TIMER_TASK_CMD_PAUSE	3

#define TIMER_STATUS_OFF		0x00
#define TIMER_STATUS_ON			0x01
#define TIMER_STATUS_PAUSED		0x02	//TODO: Pause the timer task for a certain length of time. We will probably want to set the override status here...

#define TIMER_EEPROM_START_ADDRESS			0x00		/** The address to store the first timer events in EEPROM. Subsequent events will be placed sequentially after this event. */

#define TIMER_OUTPUT_NUMBER					4			/** The number of outputs, starting at one. Setting this to 4 means that there are four outputs*/
#define TIMER_EVENT_NUMBER					6			/** The number of events for each timer, starting at one. Setting this to 6 means there are 6 events*/

#define TIMER_TASK_TYPE_TIME_EVENT			0x01		//Turn the output on or off at a certain time
#define TIMER_TASK_TYPE_REPEATING_EVENT		0x02		//Turn the output on or off on a certain interval (for this type, only one event can be specified)
#define TIMER_TASK_TYPE_SUN_EVENT			0x03		//Turn the output on or off based on sunrise/sunset time
#define TIMER_TASK_TYPE_STEADY_EVENT		0x04		//The output is always on or off (for this type, only one event can be specified)

//A FreeRTOS queue to receive commands for the timer task
xQueueHandle xTimerCommands;


/* EventType:
 *
 * EventTime
 *  For timed events:
 *  	EventTime[0]: Bitmapped DOW for event, defined as 1 << DOW. 0xFF would represent all days of the week
 * 		EventTime[1]: Hour of the event (in 24 hour format)
 * 		EventTime[2]: Minute of the event
 *
 * EventOutputState:
 * 	0 - Output off
 * 	1 - Output on
 */
typedef struct __attribute__((packed)){
	uint8_t EventType;
	uint8_t EventTime[3];
	uint8_t EventOutputState;
} TimerEvent;

uint8_t TimerGetOutputState(void);
uint8_t TimerGetTimerState(void);

//Write all events from the global event list to EEPROM
uint8_t TimerWriteEventsToEEPROM(void);

//Read all events from EEPROM and write them to the global event list
uint8_t TimerReadEventsFromEEPROM(void);

//Read a single event from EEPROM into a temporary EventData struct
uint8_t TimerReadSingleEventFromEEPROM(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData);

//Write a single event from an EventData struct into a single EEPROM event slot
uint8_t TimerWriteSingleEventToEEPROM(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData);

void TimerGetEvent(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData);
void TimerSetEvent(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData);
void TimerClearRamEvent(uint8_t OutputNumber, uint8_t EventNumber);

uint8_t InitTimerTask(void);
void TimerTask(void *pvParameter);


void TimerSetOutput(uint8_t OutputNumber, uint8_t OutputState);

void TimerUpdateOutputs(void);




#endif /* TIMER_TASK_H_ */
