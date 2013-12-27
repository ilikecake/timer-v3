/*
 * timer_task.c
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#include "main.h"

//Six possible events are defined for each output...
//TODO: make this a compiler directive for easy modificaiton
TimerEvent TimerEventList[4][6];

//TODO: make some indication of the most recent event for each output. This can be used to determine if an event was skipped...
uint8_t OutputStatus;

uint8_t TimerGetOutputState(void)
{
	return OutputStatus;
}

/*
 *
 * OutputNumber: The desired output (1-4)
 * EventNumber: The desired event (1-6)
 * EventData: The event data copied from the event list
 */
void TimerGetEvent(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData)
{
	if( (OutputNumber > 0) && (OutputNumber < 5) && (EventNumber > 0) && (EventNumber < 7) )
	{
		EventData->EventType		= TimerEventList[OutputNumber-1][EventNumber-1].EventType;
		EventData->EventTime[0]		= TimerEventList[OutputNumber-1][EventNumber-1].EventTime[0];
		EventData->EventTime[1]		= TimerEventList[OutputNumber-1][EventNumber-1].EventTime[1];
		EventData->EventTime[2]		= TimerEventList[OutputNumber-1][EventNumber-1].EventTime[2];
		EventData->EventOutputState	= TimerEventList[OutputNumber-1][EventNumber-1].EventOutputState;
	}
	return;
}

/*
 *
 * OutputNumber: The desired output (1-4)
 * EventNumber: The desired event (1-6)
 * EventData: The event data copied from the event list
 */
void TimerSetEvent(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData)
{
	if( (OutputNumber > 0) && (OutputNumber < 5) && (EventNumber > 0) && (EventNumber < 7) )
	{
		TimerEventList[OutputNumber-1][EventNumber-1].EventType = EventData->EventType;
		TimerEventList[OutputNumber-1][EventNumber-1].EventTime[0] = EventData->EventTime[0];
		TimerEventList[OutputNumber-1][EventNumber-1].EventTime[1] = EventData->EventTime[1];
		TimerEventList[OutputNumber-1][EventNumber-1].EventTime[2] = EventData->EventTime[2];
		TimerEventList[OutputNumber-1][EventNumber-1].EventOutputState = EventData->EventOutputState;
	}
	return;
}

uint8_t InitTimerTask(void)
{
	//TODO: Code to read timer events from EEPROM goes here...
	//TODO: Check if timer tasks are not set?

	OutputStatus = 0x00;
	TimerSetOutput(1, 0);
	TimerSetOutput(2, 0);
	TimerSetOutput(3, 0);
	TimerSetOutput(4, 0);

	TimerEventList[0][0].EventType = TIMER_TASK_TYPE_STEADY_EVENT;
	TimerEventList[0][0].EventTime[0] = 0;
	TimerEventList[0][0].EventTime[0] = 0;
	TimerEventList[0][0].EventTime[0] = 0;
	TimerEventList[0][0].EventOutputState = 0;

	TimerEventList[1][0].EventType = TIMER_TASK_TYPE_STEADY_EVENT;
	TimerEventList[1][0].EventTime[0] = 0;
	TimerEventList[1][0].EventTime[0] = 0;
	TimerEventList[1][0].EventTime[0] = 0;
	TimerEventList[1][0].EventOutputState = 0;

	TimerEventList[2][0].EventType = TIMER_TASK_TYPE_STEADY_EVENT;
	TimerEventList[2][0].EventTime[0] = 0;
	TimerEventList[2][0].EventTime[0] = 0;
	TimerEventList[2][0].EventTime[0] = 0;
	TimerEventList[2][0].EventOutputState = 0;

	TimerEventList[3][0].EventType = TIMER_TASK_TYPE_STEADY_EVENT;
	TimerEventList[3][0].EventTime[0] = 0;
	TimerEventList[3][0].EventTime[0] = 0;
	TimerEventList[3][0].EventTime[0] = 0;
	TimerEventList[3][0].EventOutputState = 0;

	xTimerCommands = xQueueCreate( 2, sizeof( uint8_t ) );

	return 0;
}



void TimerTask(void *pvParameter)
{
	TimeAndDate CurrentTime;
	uint8_t i;
	uint8_t j;
	uint8_t TimerCommand;

	//Wake the task here on minute interrupt

	while(1)
	{
		//This task should wake before the OLED task
		xQueueReceive(xTimerCommands, &TimerCommand, portMAX_DELAY);

		if(TimerCommand == TIMER_TASK_CMD_TICK)
		{

			DS3232M_GetTime(&CurrentTime);

			//Change day of week to bitmapped variable
			CurrentTime.dow = (1<<CurrentTime.dow);

			for(j=0;j<4;j++)
			{
				switch(TimerEventList[j][0].EventType)
				{
					case TIMER_TASK_TYPE_REPEATING_EVENT:
						//printf("Repeating Event\r\n");
						//stuff goes here...
						break;

					case TIMER_TASK_TYPE_STEADY_EVENT:
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

					case TIMER_TASK_TYPE_SUN_EVENT:
						//printf("Sun based event\r\n");
						//I need to get a sunrise/sunset table or calculator here....
						break;
				}
			}

		}
	}

	//Call the Display update task here
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


