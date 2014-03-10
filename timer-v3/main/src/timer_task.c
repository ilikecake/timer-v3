/*
 * timer_task.c
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#include "main.h"

//Four outputs, six events per output
TimerEvent TimerEventList[TIMER_OUTPUT_NUMBER][TIMER_EVENT_NUMBER];

TimeAndDate SunriseTime;	//The sunrise time, updated daily
TimeAndDate SunsetTime;		//The sunset time, updated daily
//TODO: add functions for reverse sunrise and sunset, equinoxes

TimeAndDate CurrentTime;	//A general time struct to be used in any function that needs it. This struct mus be initialized before it is used.

//TODO: make some indication of the most recent event for each output. This can be used to determine if an event was skipped...
uint8_t OutputStatus;
uint8_t TimerStatus;

//A function to start the timer
//Any initialization code that must happen when the timer start should be put here
void StartTimer(void);


//A function to stop the timer
//Any code that need to happen when the timer is stoped should be put here.
void StopTimer(void);


uint8_t TimerFindMostRecentEvent(uint8_t OutputNumber);



uint8_t TimerGetOutputState(void)
{
	return OutputStatus;
}

uint8_t TimerGetTimerState(void)
{
	return TimerStatus;
}

//Call this when a repeating event is initialized, or when a repeating event is triggered. This function will update the time of the next repeating event
void TimerUpdateRepeatingEvent(uint8_t OutputNumber);



//Update the timer status
//Note: This is a local function. Only the timer task can update the timers status
//TODO: Add somthing to write the new value to EEPROM.
void TimerSetStatus(uint8_t NewTimerStatus);

//This function should only be called once per day or when the timer is started
void UpdateSunriseAndSunset(void)
{
	int8_t UT_Offset;

	UT_Offset = GetUTOffset();

	//Add code to convert to local time

	//We set the sunrise time to the current time because the GetSunriseAndSunset function uses the date from this struct to determine the real sunrise and sunset time
	DS3232M_GetTime(&SunriseTime);

	//if((SunriseTime.hour == 1) && (SunriseTime.min == 0))
	//{
	GetSunriseAndSunsetTime(&SunriseTime, &SunsetTime);


	//Correct sunrise and sunset time to local time
	SunriseTime.hour = SunriseTime.hour + 24 + UT_Offset;
	if(SunriseTime.hour > 23)
	{
		SunriseTime.hour -= 24;
	}

	SunsetTime.hour = SunsetTime.hour + 24 + UT_Offset;
	if(SunsetTime.hour > 23)
	{
		SunsetTime.hour -= 24;
	}


	//}
	return;
}

/**The following functions are the only functions that should update the TimerEventList[][] directly. All other functions should call these functions when required */

/*Get a timer event from RAM
 *
 * OutputNumber: The desired output, starts at zero
 * EventNumber: The desired event, Starts at zero
 * EventData: The event data copied from the event list
 */
void TimerGetEvent(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData)
{
	if( (OutputNumber >= 0) && (OutputNumber < TIMER_OUTPUT_NUMBER) && (EventNumber >= 0) && (EventNumber < TIMER_EVENT_NUMBER) )
	{
		EventData->EventType		= TimerEventList[OutputNumber][EventNumber].EventType;
		EventData->EventTime[0]		= TimerEventList[OutputNumber][EventNumber].EventTime[0];
		EventData->EventTime[1]		= TimerEventList[OutputNumber][EventNumber].EventTime[1];
		EventData->EventTime[2]		= TimerEventList[OutputNumber][EventNumber].EventTime[2];
		EventData->EventOutputState	= TimerEventList[OutputNumber][EventNumber].EventOutputState;
	}
	return;
}

/*Set a timer event in RAM
 * This function saves an event in the event list. The event list should only be updated using this function.
 * OutputNumber: The desired output, starts at zero
 * EventNumber: The desired event, starts at zero
 * EventData: The event data to save to the event list
 */
void TimerSetEvent(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData)
{
	//Make sure the output and event number are valid
	if( (OutputNumber >= 0) && (OutputNumber < TIMER_OUTPUT_NUMBER) && (EventNumber >= 0) && (EventNumber < TIMER_EVENT_NUMBER) )
	{
		//Write data to the event list
		TimerEventList[OutputNumber][EventNumber].EventType = EventData->EventType;
		TimerEventList[OutputNumber][EventNumber].EventTime[0] = EventData->EventTime[0];
		TimerEventList[OutputNumber][EventNumber].EventTime[1] = EventData->EventTime[1];
		TimerEventList[OutputNumber][EventNumber].EventTime[2] = EventData->EventTime[2];
		TimerEventList[OutputNumber][EventNumber].EventOutputState = EventData->EventOutputState;

		if(EventData->EventType == TIMER_TASK_TYPE_REPEATING_EVENT)
		{
			//The repeating event must be event zero
			if(EventNumber != 0) return;
			TimerEventList[OutputNumber][EventNumber].EventTime[0] = 0;
			TimerEventList[OutputNumber][1].EventOutputState = EventData->EventOutputState;



			//Event 1 is used to store the next time to trigger the event. setup that information here.
			TimerClearRamEvent(OutputNumber, 1);
			//TimerUpdateRepeatingEvent(OutputNumber);
		}






	}
	return;
}

/*Clear a timer event in RAM
 * All timer event information is set to zero.
 *
 * OutputNumber: The desired output, starts at zero
 * EventNumber: The desired event, starts at zero
 */
void TimerClearRamEvent(uint8_t OutputNumber, uint8_t EventNumber)
{
	if( (OutputNumber >= 0) && (OutputNumber < TIMER_OUTPUT_NUMBER) && (EventNumber >= 0) && (EventNumber < TIMER_EVENT_NUMBER) )
	{
		TimerEventList[OutputNumber][EventNumber].EventType			= 0;
		TimerEventList[OutputNumber][EventNumber].EventTime[0]		= 0;
		TimerEventList[OutputNumber][EventNumber].EventTime[1]		= 0;
		TimerEventList[OutputNumber][EventNumber].EventTime[2]		= 0;
		TimerEventList[OutputNumber][EventNumber].EventOutputState	= 0;
	}
	return;
}

/**Read all events from EEPROM and write them to the global event list
 *
 *  Returns 0xFF if there is an error reading, otherwise returns 0
 */
uint8_t TimerReadEventsFromEEPROM(void)
{
	uint8_t i, j;

	for(i=0; i<TIMER_OUTPUT_NUMBER; i++)
	{
		for(j=0; j<TIMER_EVENT_NUMBER; j++)
		{
			if( TimerReadSingleEventFromEEPROM(i, j, &TimerEventList[i][j]) != 0)
			{
				return 0xFF;
			}
		}
	}

	return 0x00;
}

/**Write all events from the global event list to EEPROM
 *  TODO: make this return the maximum EEPROM address used?
 *
 *  Returns 0xFF if there is an error reading, otherwise returns 0
 */
uint8_t TimerWriteEventsToEEPROM(void)
{
	uint8_t i, j;

	for(i=0; i<TIMER_OUTPUT_NUMBER; i++)
	{
		for(j=0; j<TIMER_EVENT_NUMBER; j++)
		{
			if(TimerWriteSingleEventToEEPROM(i, j, &TimerEventList[i][j]) != 0)
			{
				return 0xFF;
			}
		}
	}

	return 0x00;
}

/**Read a single event from EEPROM into a temporary EventData struct
 * 	OutputNumber: Starts at zero, indicates the output number of the event to get.
 * 	EventNumber: Starts at zero, indicates the event number to get.
 * 	EventData: A pointer to the TimerEvent struct to put the data
 *
 *  Returns 0xFF if there is an error reading, otherwise returns 0
 */
uint8_t TimerReadSingleEventFromEEPROM(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent* EventData)
{
	uint8_t sz;
	uint16_t addr;

	sz = sizeof(*EventData);																	//The size of the TimerEvent struct
	addr = TIMER_EEPROM_START_ADDRESS + sz*(OutputNumber*TIMER_EVENT_NUMBER + EventNumber);		//The address of the current struct in EEPROM

	if( EEPROM_Read(addr, EventData, sz ) != 0)
	{
		return 0xFF;
	}

	return 0x00;
}

/**Write a single event from an EventData struct into a single EEPROM event slot
 * 	OutputNumber: Starts at zero, indicates the output number of the event to set.
 * 	EventNumber: Starts at zero, indicates the event number to set.
 * 	EventData: A pointer to the TimerEvent struct to copy to EEPROM.
 *
 *  Returns 0xFF if there is an error reading, otherwise returns 0
 */
uint8_t TimerWriteSingleEventToEEPROM(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData)
{
	uint8_t sz;
	uint16_t addr;

	sz = sizeof(*EventData);																	//The size of the TimerEvent struct
	addr = TIMER_EEPROM_START_ADDRESS + sz*(OutputNumber*TIMER_EVENT_NUMBER + EventNumber);		//The address of the current struct in EEPROM

	if( EEPROM_Write(addr, EventData, sz ) != 0)
	{
		return 0xFF;
	}

	return 0x00;
}

/**Call this function when the event list is loaded from EEPROM or the user updates an event.
 */
void TimerValidateEventList(void)
{
	int8_t i;
	int8_t j;

	for(i=0;i<TIMER_OUTPUT_NUMBER;i++)
	{
		for(j=0;j<TIMER_EVENT_NUMBER;j++)
		{
			if( TimerEventList[i][j].EventType > TIMER_TASK_TYPE_STEADY_EVENT)
			//if( (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_TIME_EVENT) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_REPEATING_EVENT) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_SUNRISE) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_SUNSET) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_STEADY_EVENT) )
			{
				//Clear data from the event
				TimerClearRamEvent(i, j);

				if(j == 0)
				{
					//If this is the first event for a specific output, set the output to off.
					TimerEventList[i][j].EventType = TIMER_TASK_TYPE_STEADY_EVENT;
					TimerEventList[i][j].EventOutputState = 0;
				}
				else
				{
					//otherwise set the event type to none
					TimerEventList[i][j].EventType = TIMER_TASK_EVENT_TYPE_NONE;
					TimerEventList[i][j].EventOutputState = 0;
				}
			}
		}
	}

	return;
}

/** End of events that update TimerEventList[][] */

/*void TimerSetupEventList(void)
{
	int8_t i;
	int8_t j;

	for(i=0;i<TIMER_OUTPUT_NUMBER;i++)
	{
		switch(TimerEventList[i][0].EventType)
		{
			case TIMER_TASK_TYPE_REPEATING_EVENT:
				TimerSetOutput(i, TimerEventList[i][0].EventOutputState);


				break;

			case TIMER_TASK_TYPE_STEADY_EVENT:
				TimerSetOutput(i, TimerEventList[i][0].EventOutputState);
				break;











		}






		for(j=0;j<TIMER_EVENT_NUMBER;j++)
		{
			if( TimerEventList[i][j].EventType > TIMER_TASK_TYPE_STEADY_EVENT)
			//if( (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_TIME_EVENT) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_REPEATING_EVENT) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_SUNRISE) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_SUNSET) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_STEADY_EVENT) )
			{
				//Clear data from the event
				TimerClearRamEvent(i, j);

				if(j == 0)
				{
					//If this is the first event for a specific output, set the output to off.
					TimerEventList[i][j].EventType = TIMER_TASK_TYPE_STEADY_EVENT;
					TimerEventList[i][j].EventOutputState = 0;
				}
				else
				{
					//otherwise set the event type to none
					TimerEventList[i][j].EventType = TIMER_TASK_EVENT_TYPE_NONE;
					TimerEventList[i][j].EventOutputState = 0;
				}
			}
		}
	}

	return;
}*/


uint8_t InitTimerTask(void)
{
	int8_t i, j;
	//TODO: Code to read timer events from EEPROM goes here...
	//TODO: Check if timer tasks are not set?

	TimerStatus = TIMER_STATUS_OFF;

	OutputStatus = 0x00;
	TimerSetOutput(0, 0);
	TimerSetOutput(1, 0);
	TimerSetOutput(2, 0);
	TimerSetOutput(3, 0);

	if(TimerReadEventsFromEEPROM() != 0)
	{
		return 0x01;	//TODO: Make this a specific error code
	}

	TimerValidateEventList();

	xTimerCommands = xQueueCreate( 2, sizeof( uint8_t ) );

	return 0;
}

void TimerTask(void *pvParameter)
{
	////TimeAndDate CurrentTime;
	//uint8_t i;
	//uint8_t j;
	uint8_t TimerCommand;

	//Wake the task here on minute interrupt

	while(1)
	{
		//This task should wake before the OLED task
		xQueueReceive(xTimerCommands, &TimerCommand, portMAX_DELAY);

		switch(TimerCommand)
		{
			case TIMER_TASK_CMD_TICK:
				TimerUpdateOutputs();
				break;

			case TIMER_TASK_CMD_START:
				StartTimer();
				//Board_LED_Set(3, 0);
				//TimerSetStatus(TIMER_STATUS_ON);
				//TimerStatus = TIMER_STATUS_ON;
				//TimerUpdateOutputs();
				break;

			case TIMER_TASK_CMD_STOP:
				StopTimer();
				//Board_LED_Set(3, 1);
				//TimerSetStatus(TIMER_STATUS_OFF);
				//TimerSetOutput(0, 0);
				//TimerSetOutput(1, 0);
				//TimerSetOutput(2, 0);
				//TimerSetOutput(3, 0);
				break;

			case TIMER_TASK_CMD_PAUSE:
				TimerSetStatus(TIMER_STATUS_PAUSED);
				//Add something here to set the outputs to their override state.
				break;


		}

	}

	return;
}

void StartTimer(void)
{
	TimerSetStatus(TIMER_STATUS_ON);
	TimerUpdateOutputs();
	return;
}

void StopTimer(void)
{
	uint8_t i;
	//Board_LED_Set(3, 1);
	TimerSetStatus(TIMER_STATUS_OFF);
	TimerSetOutput(0, 0);
	TimerSetOutput(1, 0);
	TimerSetOutput(2, 0);
	TimerSetOutput(3, 0);

	for(i=0;i<TIMER_OUTPUT_NUMBER;i++)
	{
		if(TimerEventList[i][0].EventType == TIMER_TASK_TYPE_REPEATING_EVENT)
		{
			//Reinitialize the repeating event
			TimerSetEvent(i, 0, &TimerEventList[i][0]);
		}
	}




	return;
}

//Reorder the events. The sun based events will
//void TimerReorderEvents(void);

/** Update all the outputs based on the current time */
void TimerUpdateOutputs(void)
{
	//TimeAndDate CurrentTime;
	uint8_t j;
	uint8_t i;

	if(TimerStatus == TIMER_STATUS_ON)
	{
		DS3232M_GetTime(&CurrentTime);

		//Change day of week to bitmapped variable
		CurrentTime.dow = (1<<CurrentTime.dow);

		//Count through the outputs
		for(j=0; j<4; j++)
		{
			switch(TimerEventList[j][0].EventType)
			{
			case TIMER_TASK_TYPE_REPEATING_EVENT:		//Good
				if(TimerEventList[j][0].EventTime[0] == 0)
				{
					//This is the fist time the repeating event has been triggered
					TimerEventList[j][0].EventTime[0] = 1;
					TimerSetOutput(j, TimerEventList[j][1].EventOutputState);
				}
				else
				{
					if((TimerEventList[j][1].EventTime[1] == CurrentTime.hour) && (TimerEventList[j][1].EventTime[2] == CurrentTime.min) )
					{
						//Time matches, set the output to the new state
						TimerSetOutput(j, TimerEventList[j][1].EventOutputState);
					}
				}

				TimerUpdateRepeatingEvent(j);
				break;

			case TIMER_TASK_TYPE_STEADY_EVENT:		//Good
				//printf("Steady Event: Output %u state %u\r\n", j, TimerEventList[j][0].EventOutputState);

				if( (OutputStatus & (1<<(j))) != TimerEventList[j][0].EventOutputState)
				{
					TimerSetOutput(j, TimerEventList[j][0].EventOutputState);
				}
				break;

			case TIMER_TASK_TYPE_TIME_EVENT:
				//printf("Timed event\r\n");
				for(i=0; i<6; i++)
				{
					if( ((TimerEventList[j][i].EventTime[0] & CurrentTime.dow) == CurrentTime.dow)  && (TimerEventList[j][i].EventTime[1] == CurrentTime.hour) && (TimerEventList[j][i].EventTime[2] == CurrentTime.min) )
					{
						//Time matches, set the output to the new state
						//printf("Match %u output to %u\r\n", i, TimerEventList[j][i].EventOutputState);
						TimerSetOutput(j, TimerEventList[j][i].EventOutputState);
					}
				}
				break;

			case TIMER_TASK_TYPE_SUNRISE:
			case TIMER_TASK_TYPE_SUNSET:
				//printf("Sun based event\r\n");
				//I need to get a sunrise/sunset table or calculator here....
				break;
			}
		}

	}

	return;
}

//OutputNumber is 0-3
void TimerSetOutput(uint8_t OutputNumber, uint8_t OutputState)
{
	//Update the status register
	if(OutputState == 1)
	{
		OutputStatus |= (1<<(OutputNumber));
	}
	else if(OutputState == 0)
	{
		OutputStatus &= ~(1<<(OutputNumber));
	}

	//Actually update the outputs here (eventually)

	return;
}

void TimerSetStatus(uint8_t NewTimerStatus)
{
	TimerStatus = NewTimerStatus;
	//Write status ot EEPROM
}

void TimerUpdateRepeatingEvent(uint8_t OutputNumber)
{
	//Make sure we are looking at a repeating event
	if(TimerEventList[OutputNumber][0].EventType != TIMER_TASK_TYPE_REPEATING_EVENT) return;

	DS3232M_GetTime(&CurrentTime);

	TimerEventList[OutputNumber][1].EventTime[1] = CurrentTime.hour + TimerEventList[OutputNumber][0].EventTime[1];
	TimerEventList[OutputNumber][1].EventTime[2] = CurrentTime.min + TimerEventList[OutputNumber][0].EventTime[2];

	if(TimerEventList[OutputNumber][1].EventTime[2] > 59)
	{
		TimerEventList[OutputNumber][1].EventTime[2] -= 60;
		TimerEventList[OutputNumber][1].EventTime[1]++;
	}

	if(TimerEventList[OutputNumber][1].EventTime[1] > 23)
	{
		TimerEventList[OutputNumber][1].EventTime[1] -= 24;
	}

	if(TimerEventList[OutputNumber][1].EventOutputState == 1)
	{
		TimerEventList[OutputNumber][1].EventOutputState = 0;
	}
	else
	{
		TimerEventList[OutputNumber][1].EventOutputState = 1;
	}
	return;
}

void GetSunriseTime(TimeAndDate *theTime)
{
	theTime->day	= SunriseTime.day;
	theTime->month	= SunriseTime.month;
	theTime->year	= SunriseTime.year;
	theTime->hour	= SunriseTime.hour;
	theTime->min	= SunriseTime.min;
	theTime->sec	= SunriseTime.sec;
	return;
}

void GetSunsetTime(TimeAndDate *theTime)
{
	theTime->day	= SunsetTime.day;
	theTime->month	= SunsetTime.month;
	theTime->year	= SunsetTime.year;
	theTime->hour	= SunsetTime.hour;
	theTime->min	= SunsetTime.min;
	theTime->sec	= SunsetTime.sec;
	return;
}

uint8_t TimerFindMostRecentEvent(uint8_t OutputNumber)
{
	TimeAndDate EventTime;
	uint8_t i;
	uint8_t EventNumber;

	EventNumber = 0xFF;

	//TODO: make sure OutputNumber is valid

	DS3232M_GetTime(&CurrentTime);

	//We only compare the hour and min
	EventTime.day	= CurrentTime.day;
	EventTime.month	= CurrentTime.month;
	EventTime.year	= CurrentTime.year;
	EventTime.sec	= CurrentTime.sec;

	for(i=0;i<TIMER_EVENT_NUMBER;i++)
	{
		if(TimerEventList[OutputNumber][i].EventType == TIMER_TASK_TYPE_TIME_EVENT)
		{
			EventTime.hour = TimerEventList[OutputNumber][i].EventTime[1];
			EventTime.min = TimerEventList[OutputNumber][i].EventTime[2];
		}

		//Check to see if the event is supposed to happen today
		if((TimerEventList[OutputNumber][i].EventTime[0] & (1<< CurrentTime.dow)) != 0)
		{
			if(TimeAndDateCompare(CurrentTime, EventTime, 0) != 2)
			{
				EventNumber = i;




			}
		}




		TimerEventList[OutputNumber][TIMER_EVENT_NUMBER];
	}





}

