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

//TODO: Put the status variables in EEPROM
#define TIMER_STATUS_OFF		0x00
#define TIMER_STATUS_ON			0x01
#define TIMER_STATUS_PAUSED		0x02	//TODO: Pause the timer task for a certain length of time. We will probably want to set the override status here...

//#define TIMER_EEPROM_START_ADDRESS			0x00		/** The address to store the first timer events in EEPROM. Subsequent events will be placed sequentially after this event. */

#define TIMER_OUTPUT_NUMBER					4			/** The number of outputs, starting at one. Setting this to 4 means that there are four outputs*/
#define TIMER_EVENT_NUMBER					6			/** The number of events for each timer, starting at one. Setting this to 6 means there are 6 events*/

//Defines the type of events. TODO: rename these variables later...
#define TIMER_EVENT_TYPE_NONE				0x00		//No event
#define TIMER_EVENT_TYPE_STEADY				0x01		//The output is always on or off (for this type, only one event can be specified)
#define TIMER_EVENT_TYPE_REPEATING			0x02		//Turn the output on or off on a certain interval (for this type, only one event can be specified)
#define TIMER_EVENT_TYPE_TIMED				0x03		//Turn the output on or off at a certain time
#define TIMER_EVENT_TYPE_SUNRISE			0x04		//Turn the output on or off based on todays sunrise time
#define TIMER_EVENT_TYPE_SUNSET				0x05		//Turn the output on or off based on todays sunset time
#define TIMER_EVENT_TYPE_SUNRISE_OPPOSITE	0x06		//Turn the output on or off based on the sunrise time half a year from now
#define TIMER_EVENT_TYPE_SUNSET_OPPOSITE	0x07		//Turn the output on or off based on the sunset time half a year from now
#define TIMER_EVENT_TYPE_SUNRISE_SUMMER		0x08		//Turn the output on or off based on the summer sunrise time
#define TIMER_EVENT_TYPE_SUNSET_SUMMER		0x09		//Turn the output on or off based on the summer sunset time
#define TIMER_EVENT_TYPE_SUNRISE_WINTER		0x0A		//Turn the output on or off based on the winter sunrise time
#define TIMER_EVENT_TYPE_SUNSET_WINTER		0x0B		//Turn the output on or off based on the winter sunset time

#define TIMER_EVENT_FIRST				TIMER_EVENT_TYPE_NONE
#define TIMER_EVENT_LAST				TIMER_EVENT_TYPE_SUNSET_WINTER
#define TIMER_EVENT_FIRST_TIMED			TIMER_EVENT_TYPE_TIMED
#define TIMER_EVENT_LAST_TIMED			TIMER_EVENT_TYPE_SUNSET_WINTER

extern char *EventNames[12];





//To define a sunrise or sunset event, one of the first two defines must be ored with one of the later defines.
//#define TIMER_SUN_BASED_SUNRISE				0x00		//Signifies the event will occur based on a sunrise
//#define TIMER_SUN_BASED_SUNSET				0x01		//Signifies the event will occur based on a sunset
//#define TIMER_SUN_BASED_NORMAL				0x02		//The actual sunrise or sunset will trigger the event
//#define TIMER_SUN_BASED_OPPOSITE			0X04		//The opposite sunrise or sunset will trigger the event. This means that the sunrise or sunset time that is an equal number of days from the equinox in the other direction will be used. Using this feature will 'invert' the seasons.
//#define TIMER_SUN_BASED_LATER				0x08		//Always pick the later of the two possible sunrise/sunset times. This will trigger the event on the normal or opposite sunrise/sunset, whichever is later.
//#define TIMER_SUN_BASED_EARLIER				0x10		//Always pick the earlier of the two possible sunrise/sunset times. This will trigger the event on the normal or opposite sunrise/sunset, whichever is earlier.

//TODO: make the above sun_based defines ored with the task_type defines. Add a define to allow a random hour/minute use eventTime[1], and [2] to hold hours and minutes to offset. If a random event is desired, make those two values random, and update everytime the event is triggered. Allow both random minutes, random hours, or both.

//defines to test event types
#define TIMER_IS_VALID_EVENT(x) ( (x >= TIMER_EVENT_FIRST) && (x <= TIMER_EVENT_LAST) ? (1) : (0) )
#define TIMER_EVENT_USES_DOW(x) ( (x == TIMER_EVENT_TYPE_TIMED) ? (1) : (0) )



#define TIMER_SUN_UPDATE_HOUR		3
#define TIMER_SUN_UPDATE_MIN		0


//A FreeRTOS queue to receive commands for the timer task
xQueueHandle xTimerCommands;

/* EventType:
 *
 * EventTime
 *  For timed events:
 *  	EventTime[0]: Bitmapped DOW for event, defined as 1 << DOW. 0xFF would represent all days of the week. Bit position 1 is Sunday.
 * 		EventTime[1]: Hour of the event (in 24 hour format)
 * 		EventTime[2]: Minute of the event
 *
 * 	For sun based events
 * 		EventTime[0]: Bitmapped DOW for event, defined as 1 << DOW. 0xFF would represent all days of the week. Bit position 1 is Sunday.
 * 		EventTime[1]: The setup bits for the sun based event. See the TIMER_SUN_BASED_ defines for more information.
 * 		EventTime[2]: The number of hours to delay the event (signed)
 *
 * 	For repeating events
 * 		Event0
 * 			EventTime[0]: Indicates if the repeating event has been started. Initalized to zero when the event is programed, and set to 1 after the event has occured for the first time.
 * 			EventTime[1]: Hour to delay
 * 			EventTime[2]: Minutes to delay
 * 		Event1
 *			EventTime[0]: The next value of the output
 * 			EventTime[1]: The hours of the next output toggle
 * 			EventTime[2]: The minutes of the next output toggle
 *
 *
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


void StartTimer(void);
void StopTimer(void);

uint8_t TimerGetOutputState(void);
uint8_t TimerGetTimerState(void);

//Write all events from the global event list to EEPROM
uint8_t TimerWriteEventsToEEPROM(void);

//Read all events from EEPROM and write them to the global event list
uint8_t TimerReadEventsFromEEPROM(void);

//Validate the event list in RAM
void TimerValidateEventList(void);

//Call after loading events from EEPROM or when the user updates an event
//void TimerSetupEventList(void);

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

void TimerUpdateOutput(uint8_t OutputNumber);
void TimerUpdateOutputs(void);

void UpdateSunriseAndSunset(uint8_t ForceUpdate);		//Call this to update the calculated sunrise and sunset times.
void GetSunriseTime(struct tm *theTime);
void GetSunsetTime(struct tm *theTime);
void GetAltSunriseTime(struct tm *theTime);
void GetAltSunsetTime(struct tm *theTime);


#endif /* TIMER_TASK_H_ */
