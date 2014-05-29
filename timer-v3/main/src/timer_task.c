/*
 * timer_task.c
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#include "main.h"

//Four outputs, six events per output
TimerEvent TimerEventList[TIMER_OUTPUT_NUMBER][TIMER_EVENT_NUMBER];

//TimeAndDate SunriseTime;	//The sunrise time, updated daily
//TimeAndDate SunsetTime;		//The sunset time, updated daily
//TODO: add functions for reverse sunrise and sunset, equinoxes

//TimeAndDate CurrentTime;	//A general time struct to be used in any function that needs it. This struct mus be initialized before it is used.

//TODO: Handle the user entering two event for the same output with the same time

struct tm SunriseTime;
struct tm SunsetTime;
struct tm AltSunriseTime;
struct tm AltSunsetTime;
struct tm CurrentTime;

//TODO: make some indication of the most recent event for each output. This can be used to determine if an event was skipped...
uint8_t OutputStatus;
uint8_t TimerStatus;

uint8_t OverrideTimeRemaining;


char *EventNames[12] = {"None", "Steady", "Repeat", "Timer", "Sunrise", "Sunset", "Sunrise1", "Sunset1", "Sunrise2", "Sunset2", "Sunrise3", "Sunset3" };




//A function to start the timer
//Any initialization code that must happen when the timer start should be put here
//void StartTimer(void);


//A function to stop the timer
//Any code that need to happen when the timer is stoped should be put here.
//void StopTimer(void);


//A function to pause the timer
//void PauseTimer(void);



uint8_t TimerGetOutputState(void)
{
	return OutputStatus;
}

uint8_t TimerGetTimerState(void)
{
	if((TimerStatus & TIMER_STATUS_OVERRIDE_MASK) == 0)
	{
		return TimerStatus;
	}
	else
	{
		return TIMER_STATUS_PAUSED;
	}
}

//TODO: Standardize the naming of the pause/override function

uint8_t TimerClearOverrideState(void);

//Call this when a repeating event is initialized, or when a repeating event is triggered. This function will update the time of the next repeating event
void TimerUpdateRepeatingEvent(uint8_t OutputNumber);



//Update the timer status
//Note: This is a local function. Only the timer task can update the timers status
//TODO: Add somthing to write the new value to EEPROM.
void TimerSetStatus(uint8_t NewTimerStatus);

/**Update the sunrise and sunset times stored in memory. This function updates the sunrise and sunset times based off the current time from the RTC.
 * This function should be called when the timer is starts, when a new time is set, and at least once per day before any sunrise or sunset events will occur.
 *
 * 		ForceUpdate:	The function will only update the time if the hours and minutes match the values defined in the
 * 						TIMER_SUN_UPDATE_HOUR and TIMER_SUN_UPDATE_MIN defines. Set the value to 1 to override this check.
 */
void UpdateSunriseAndSunset(uint8_t ForceUpdate)
{
	int8_t Offset;
	time_t TimeVal;

	DS3232M_GetTime(&CurrentTime);

	if( ((CurrentTime.tm_hour == TIMER_SUN_UPDATE_HOUR) && (CurrentTime.tm_min ==TIMER_SUN_UPDATE_MIN)) || (ForceUpdate == 1) )
	{
		Offset = GetUTOffset();

		//We set the sunrise time to the current time because the GetSunriseAndSunset function uses the date from this struct to determine the real sunrise and sunset time
		SunriseTime = CurrentTime;
		GetSunriseAndSunsetTime(&SunriseTime, &SunsetTime);

		//Correct sunrise and sunset time to local time
		SunriseTime.tm_hour = SunriseTime.tm_hour + 24 + Offset;
		if(SunriseTime.tm_hour > 23)
		{
			SunriseTime.tm_hour -= 24;
		}

		SunsetTime.tm_hour = SunsetTime.tm_hour + 24 + Offset;
		if(SunsetTime.tm_hour > 23)
		{
			SunsetTime.tm_hour -= 24;
		}

		DS3232M_GetTime(&AltSunriseTime);
		TimeVal = mktime(&AltSunriseTime);
		TimeVal += 15724800;						//Note: This value is adding 182 days to the current time. This is not exactly correct, but it should be close enough.
		AltSunriseTime = *localtime(&TimeVal);
		GetSunriseAndSunsetTime(&AltSunriseTime, &AltSunsetTime);

		//Correct sunrise and sunset time to local time
		AltSunriseTime.tm_hour = AltSunriseTime.tm_hour + 24 + Offset;
		if(AltSunriseTime.tm_hour > 23)
		{
			AltSunriseTime.tm_hour -= 24;
		}

		AltSunsetTime.tm_hour = AltSunsetTime.tm_hour + 24 + Offset;
		if(AltSunsetTime.tm_hour > 23)
		{
			AltSunsetTime.tm_hour -= 24;
		}

		Offset = GetDST();
		if((Offset & 0x01) == 0x01)
		{
			//DST is in effect, add one to the hours of each time
			SunriseTime.tm_hour += 1;
			SunsetTime.tm_hour += 1;
			AltSunriseTime.tm_hour += 1;
			AltSunsetTime.tm_hour+= 1;
		}
	}

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

		if(EventData->EventType == TIMER_EVENT_TYPE_REPEATING)
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
	addr = EEPROM_ADDRESS_TIMER_EVENTS + sz*(OutputNumber*TIMER_EVENT_NUMBER + EventNumber);		//The address of the current struct in EEPROM

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
	addr = EEPROM_ADDRESS_TIMER_EVENTS + sz*(OutputNumber*TIMER_EVENT_NUMBER + EventNumber);		//The address of the current struct in EEPROM

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
			if( TIMER_IS_VALID_EVENT(TimerEventList[i][j].EventType) == 0 )
			//if( (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_TIME_EVENT) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_REPEATING_EVENT) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_SUNRISE) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_SUNSET) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_STEADY_EVENT) )
			{
				//Clear data from the event
				TimerClearRamEvent(i, j);

				if(j == 0)
				{
					//If this is the first event for a specific output, set the output to off.
					TimerEventList[i][j].EventType = TIMER_EVENT_TYPE_STEADY;
					TimerEventList[i][j].EventOutputState = 0;
				}
				else
				{
					//otherwise set the event type to none
					TimerEventList[i][j].EventType = TIMER_EVENT_TYPE_NONE;
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
	//int8_t i, j;
	//TODO: Code to read timer events from EEPROM goes here...
	//TODO: Check if timer tasks are not set?

	TimerStatus = TIMER_STATUS_OFF;

	OverrideTimeRemaining = 0;

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
	//uint16_t OverrideTime;

	//Wake the task here on minute interrupt

	while(1)
	{
		//This task should wake before the OLED task
		xQueueReceive(xTimerCommands, &TimerCommand, portMAX_DELAY);

		switch(TimerCommand)
		{
			case TIMER_TASK_CMD_TICK:
				if(TimerGetTimerState() ==  TIMER_STATUS_PAUSED)
				{
					OverrideTimeRemaining--;
					if(OverrideTimeRemaining <= 0)
					{
						TimerClearOverrideState();
					}
				}
				if(TimerStatus == TIMER_STATUS_ON)
				{
					TimerUpdateOutputs();
				}
				else if(TimerStatus == TIMER_STATUS_OFF)
				{
					StopTimer();
				}

				break;

			case TIMER_TASK_CMD_START:
				StartTimer();	//TODO: Do I need these here, or should I just call StartTimer to start the timer?
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
				PauseTimer();
				//TimerSetStatus(TIMER_STATUS_PAUSED);
				//Add something here to set the outputs to their override state.
				break;


		}

	}

	return;
}


//should these be internal to this file?
void StartTimer(void)
{
	if(App_GetStatus() != APP_STATUS_OSC_STOPPED)
	{
		//Do not allow the user to start the timer if the time is invalid
		UpdateSunriseAndSunset(1);
		TimerSetStatus(TIMER_STATUS_ON);
		TimerUpdateOutputs();
	}
	else
	{

	}
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
		if(TimerEventList[i][0].EventType == TIMER_EVENT_TYPE_REPEATING)
		{
			//Reinitialize the repeating event
			TimerSetEvent(i, 0, &TimerEventList[i][0]);
		}
	}




	return;
}

void PauseTimer(void)
{
	uint8_t PauseState;
	uint8_t i;

	TimerSetStatus(TIMER_STATUS_PAUSED);
	OverrideTimeRemaining = TimerGetPauseTime();

	PauseState = TimerGetOverrideOutputs();

	for(i=0;i<4;i++)
	{
		if((PauseState & (1<<i)) != 0x00)
		{
			TimerSetOutput(i, 1);
		}
		else
		{
			TimerSetOutput(i, 0);
		}
	}

	return;
}

void TimerSetPauseTime(uint8_t TimeToPause)
{
	//Save new value in EEPROM
	if( EEPROM_Write(EEPROM_ADDRESS_OVERRIDE_TIMEOUT, &TimeToPause, 1 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}
}

//Retrieve the pause time from EEPROM
uint8_t TimerGetPauseTime(void)
{
	uint8_t PauseTime;

	//Save new value in EEPROM
	if( EEPROM_Read(EEPROM_ADDRESS_OVERRIDE_TIMEOUT, &PauseTime, 1 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}

	return PauseTime;
}

void TimerSetOverrideOutputs(uint8_t StateToSet)
{
	//Save new value in EEPROM
	if( EEPROM_Write(EEPROM_ADDRESS_OVERRIDE_OUTPUTS, &StateToSet, 1 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}
	return;
}

uint8_t TimerGetOverrideOutputs(void)
{
	uint8_t PauseState;

	//Save new value in EEPROM
	if( EEPROM_Read(EEPROM_ADDRESS_OVERRIDE_OUTPUTS, &PauseState, 1 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}

	return PauseState;
}


//Reorder the events. The sun based events will
//void TimerReorderEvents(void);

//call this to update time based or sun based events
//TODO: Handle time based events with no days of the week set
void TimerUpdateOutput(uint8_t OutputNumber)
{
	time_t CurrentTimeVal;
	time_t TempTimeVal;
	int32_t diff;
	struct tm TempTime;
	int8_t i;
	int8_t k;

	if(TimerStatus == TIMER_STATUS_ON)
	{
		DS3232M_GetTime(&CurrentTime);

		CurrentTimeVal = mktime(&CurrentTime);
		diff = 0xFFFFFFF;
		k = -1;



		for(i=0; i<TIMER_EVENT_NUMBER; i++)
		{
			if((TimerEventList[OutputNumber][i].EventType >= TIMER_EVENT_FIRST_TIMED) && (TimerEventList[OutputNumber][i].EventType <= TIMER_EVENT_LAST_TIMED))
			{
				//If the DOW variable is zero, somthing bad happened
				//if(TimerEventList[OutputNumber][i].EventTime[0] == 0) App_Die(1);


				TempTime = CurrentTime;

				switch(TimerEventList[OutputNumber][i].EventType)
				{
					case TIMER_EVENT_TYPE_TIMED:
						//Find the last time this event should be triggered
						//TempTime = CurrentTime;
						TempTime.tm_min		= TimerEventList[OutputNumber][i].EventTime[2];
						TempTime.tm_hour	= TimerEventList[OutputNumber][i].EventTime[1];
						//TempTimeVal = mktime(&TempTime);
						break;

					case TIMER_EVENT_TYPE_SUNRISE:
						TempTime.tm_min		= SunriseTime.tm_min;
						TempTime.tm_hour	= SunriseTime.tm_hour;
						break;

					case TIMER_EVENT_TYPE_SUNSET:
						TempTime.tm_min		= SunsetTime.tm_min;
						TempTime.tm_hour	= SunsetTime.tm_hour;
						break;

					case TIMER_EVENT_TYPE_SUNRISE_OPPOSITE:
						TempTime.tm_min		= AltSunriseTime.tm_min;
						TempTime.tm_hour	= AltSunriseTime.tm_hour;
						break;

					case TIMER_EVENT_TYPE_SUNSET_OPPOSITE:
						TempTime.tm_min		= AltSunsetTime.tm_min;
						TempTime.tm_hour	= AltSunsetTime.tm_hour;
						break;

					case TIMER_EVENT_TYPE_SUNRISE_SUMMER:
					case TIMER_EVENT_TYPE_SUNRISE_WINTER:
					case TIMER_EVENT_TYPE_SUNSET_SUMMER:
					case TIMER_EVENT_TYPE_SUNSET_WINTER:
						if((CurrentTime.tm_yday >= 79) && (CurrentTime.tm_yday <= 265))
						{
							//Summer
							if(TimerEventList[OutputNumber][i].EventType == TIMER_EVENT_TYPE_SUNRISE_SUMMER)
							{
								TempTime.tm_min		= SunriseTime.tm_min;
								TempTime.tm_hour	= SunriseTime.tm_hour;
							}
							else if(TimerEventList[OutputNumber][i].EventType == TIMER_EVENT_TYPE_SUNSET_SUMMER)
							{
								TempTime.tm_min		= SunsetTime.tm_min;
								TempTime.tm_hour	= SunsetTime.tm_hour;
							}
							else if(TimerEventList[OutputNumber][i].EventType == TIMER_EVENT_TYPE_SUNRISE_WINTER)
							{
								TempTime.tm_min		= AltSunriseTime.tm_min;
								TempTime.tm_hour	= AltSunriseTime.tm_hour;
							}
							else if(TimerEventList[OutputNumber][i].EventType == TIMER_EVENT_TYPE_SUNSET_WINTER)
							{
								TempTime.tm_min		= AltSunsetTime.tm_min;
								TempTime.tm_hour	= AltSunsetTime.tm_hour;
							}

						}
						else
						{
							//Winter
							if(TimerEventList[OutputNumber][i].EventType == TIMER_EVENT_TYPE_SUNRISE_SUMMER)
							{
								TempTime.tm_min		= AltSunriseTime.tm_min;
								TempTime.tm_hour	= AltSunriseTime.tm_hour;

							}
							else if(TimerEventList[OutputNumber][i].EventType == TIMER_EVENT_TYPE_SUNSET_SUMMER)
							{
								TempTime.tm_min		= AltSunsetTime.tm_min;
								TempTime.tm_hour	= AltSunsetTime.tm_hour;
							}
							else if(TimerEventList[OutputNumber][i].EventType == TIMER_EVENT_TYPE_SUNRISE_WINTER)
							{
								TempTime.tm_min		= SunriseTime.tm_min;
								TempTime.tm_hour	= SunriseTime.tm_hour;
							}
							else if(TimerEventList[OutputNumber][i].EventType == TIMER_EVENT_TYPE_SUNSET_WINTER)
							{
								TempTime.tm_min		= SunsetTime.tm_min;
								TempTime.tm_hour	= SunsetTime.tm_hour;
							}
						}
						break;

				}

				//Note: TempTime and TempTimeVal are the time that the event happens.
				//		1 day increments are subtracted from these times until a day on which the event is supposed to occur is found.
				//		at the end of this function, these variables should represent the most recent time each event was triggered.
				TempTimeVal = mktime(&TempTime);


				//if((TimerEventList[OutputNumber][i].EventType == TIMER_TASK_TYPE_TIME_EVENT) )
				/*if(TimerEventList[OutputNumber][i].EventType == TIMER_EVENT_TYPE_TIMED)
			{
				//Find the last time this event should be triggered
				TempTime = CurrentTime;
				TempTime.tm_min		= TimerEventList[OutputNumber][i].EventTime[2];
				TempTime.tm_hour	= TimerEventList[OutputNumber][i].EventTime[1];
				TempTimeVal = mktime(&TempTime);
			}
			else if(TimerEventList[OutputNumber][i].EventType >= TIMER_EVENT_TYPE_SUNRISE)
			{
				//Find the last time this event should be triggered
				TempTime = CurrentTime;
				TempTime.tm_min		= TimerEventList[OutputNumber][i].EventTime[2];
				TempTime.tm_hour	= TimerEventList[OutputNumber][i].EventTime[1];
				TempTimeVal = mktime(&TempTime);
			}*/


				//If the event happens in the future
				//if(TempTimeVal > CurrentTimeVal)


				//Count back days until the most recent time in the past when this event occured is found. The wile loop continues as long as the event happens in the future (this should only occur once),
				//and the day of the week of the event does not match a day of the week when the event is supposed to occur, this could occur up to 7 times.
				while( (((1 << TempTime.tm_wday)&(TimerEventList[OutputNumber][i].EventTime[0])) == 0) || (TempTimeVal > CurrentTimeVal))
				{
					//Subtract one day from the time and check again
					TempTimeVal -= 86400;
					TempTime = *localtime(&TempTimeVal);
					//NOTE: the function will hang here if the event does not have any days of the week defined (EventTime[0] = 0)
				}

				//if(CurrentTimeVal >= TempTimeVal)	//TODO: Figure out what happens when CurrentTimeVal = TempTimeVal
				//{
					//The event happened in the past
					if( (CurrentTimeVal-TempTimeVal) < diff)
					{
						k = i;
						diff = (CurrentTimeVal-TempTimeVal);
					}
				//}

				//If either of these occur, something bad happened
				if(k == -1) 			App_Die(1);
				if(diff == 0xFFFFFFF)	App_Die(2);

			}
		}




		//k now points to the most recent event, and diff is the elapsed seconds from that event until now.
		if(TimerEventList[OutputNumber][k].EventOutputState != (OutputStatus >> OutputNumber) )
		{
			TimerSetOutput(OutputNumber, TimerEventList[OutputNumber][k].EventOutputState);
		}
	}

	return;
}



/** Update all the outputs based on the current time */
void TimerUpdateOutputs(void)
{
	uint8_t j;

	if(TimerStatus == TIMER_STATUS_ON)
	{
		DS3232M_GetTime(&CurrentTime);

		//Count through the outputs
		for(j=0; j<TIMER_OUTPUT_NUMBER; j++)
		{
			switch(TimerEventList[j][0].EventType)
			{
			case TIMER_EVENT_TYPE_REPEATING:		//Good
				if(TimerEventList[j][0].EventTime[0] == 0)
				{
					//This is the fist time the repeating event has been triggered
					TimerEventList[j][0].EventTime[0] = 1;
					TimerSetOutput(j, TimerEventList[j][1].EventOutputState);
				}
				else
				{
					if((TimerEventList[j][1].EventTime[1] == CurrentTime.tm_hour) && (TimerEventList[j][1].EventTime[2] == CurrentTime.tm_min) )
					{
						//Time matches, set the output to the new state
						TimerSetOutput(j, TimerEventList[j][1].EventOutputState);
					}
				}

				TimerUpdateRepeatingEvent(j);
				break;

			case TIMER_EVENT_TYPE_STEADY:		//Good
				if( (OutputStatus & (1<<(j))) != TimerEventList[j][0].EventOutputState)
				{
					TimerSetOutput(j, TimerEventList[j][0].EventOutputState);
				}
				break;

			case TIMER_EVENT_TYPE_NONE:
				//TODO: Type none and steady off are the same, can I combine these?
				TimerSetOutput(j, 0);
				break;

			case TIMER_EVENT_TYPE_TIMED:
			case TIMER_EVENT_TYPE_SUNRISE:
			case TIMER_EVENT_TYPE_SUNSET:
			case TIMER_EVENT_TYPE_SUNRISE_OPPOSITE:
			case TIMER_EVENT_TYPE_SUNSET_OPPOSITE:
			case TIMER_EVENT_TYPE_SUNRISE_SUMMER:
			case TIMER_EVENT_TYPE_SUNSET_SUMMER:
			case TIMER_EVENT_TYPE_SUNRISE_WINTER:
			case TIMER_EVENT_TYPE_SUNSET_WINTER:
				TimerUpdateOutput(j);
				break;



			//case TIMER_EVENT_TYPE_SUNRISE:
			//case TIMER_EVENT_TYPE_SUNSET:
				//printf("Sun based event\r\n");
				//I need to get a sunrise/sunset table or calculator here....
				//break;
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
	if(NewTimerStatus == TIMER_STATUS_PAUSED)
	{
		TimerStatus = TimerStatus | TIMER_STATUS_OVERRIDE_MASK;
	}
	else
	{
		TimerStatus = NewTimerStatus;
	}

	if( EEPROM_Write(EEPROM_ADDRESS_TIMER_STATUS, &TimerStatus, 1 ) !=0)
	{	//Failed to write to EEPROM
		App_Die(8);
	}
	return;
}

uint8_t TimerClearOverrideState(void)
{
	TimerStatus = TimerStatus & (~TIMER_STATUS_OVERRIDE_MASK);

	if( EEPROM_Write(EEPROM_ADDRESS_TIMER_STATUS, &TimerStatus, 1 ) !=0)
	{	//Failed to write to EEPROM
		App_Die(8);
	}

	return TimerStatus;
}

void TimerUpdateRepeatingEvent(uint8_t OutputNumber)
{
	//Make sure we are looking at a repeating event
	if(TimerEventList[OutputNumber][0].EventType != TIMER_EVENT_TYPE_REPEATING) return;

	DS3232M_GetTime(&CurrentTime);

	TimerEventList[OutputNumber][1].EventTime[1] = CurrentTime.tm_hour + TimerEventList[OutputNumber][0].EventTime[1];
	TimerEventList[OutputNumber][1].EventTime[2] = CurrentTime.tm_min + TimerEventList[OutputNumber][0].EventTime[2];

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

void GetSunriseTime(struct tm *theTime)
{
	theTime->tm_mday	= SunriseTime.tm_mday;
	theTime->tm_mon		= SunriseTime.tm_mon;
	theTime->tm_year	= SunriseTime.tm_year;
	theTime->tm_hour	= SunriseTime.tm_hour;
	theTime->tm_min		= SunriseTime.tm_min;
	theTime->tm_sec		= SunriseTime.tm_sec;
	return;
}

void GetSunsetTime(struct tm *theTime)
{
	theTime->tm_mday	= SunsetTime.tm_mday;
	theTime->tm_mon		= SunsetTime.tm_mon;
	theTime->tm_year	= SunsetTime.tm_year;
	theTime->tm_hour	= SunsetTime.tm_hour;
	theTime->tm_min		= SunsetTime.tm_min;
	theTime->tm_sec		= SunsetTime.tm_sec;
	return;
}

void GetAltSunriseTime(struct tm *theTime)
{
	theTime->tm_mday	= AltSunriseTime.tm_mday;
	theTime->tm_mon		= AltSunriseTime.tm_mon;
	theTime->tm_year	= AltSunriseTime.tm_year;
	theTime->tm_hour	= AltSunriseTime.tm_hour;
	theTime->tm_min		= AltSunriseTime.tm_min;
	theTime->tm_sec		= AltSunriseTime.tm_sec;
	return;
}

void GetAltSunsetTime(struct tm *theTime)
{
	theTime->tm_mday	= AltSunsetTime.tm_mday;
	theTime->tm_mon		= AltSunsetTime.tm_mon;
	theTime->tm_year	= AltSunsetTime.tm_year;
	theTime->tm_hour	= AltSunsetTime.tm_hour;
	theTime->tm_min		= AltSunsetTime.tm_min;
	theTime->tm_sec		= AltSunsetTime.tm_sec;
	return;
}
