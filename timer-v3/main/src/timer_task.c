/*
 * timer_task.c
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#include "main.h"

//Four outputs, six events per output
TimerEvent TimerEventList[TIMER_OUTPUT_NUMBER][TIMER_EVENT_NUMBER];

//TODO: make some indication of the most recent event for each output. This can be used to determine if an event was skipped...
uint8_t OutputStatus;
uint8_t TimerStatus;

uint8_t TimerGetOutputState(void)
{
	return OutputStatus;
}

uint8_t TimerGetTimerState(void)
{
	return TimerStatus;
}

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
 *
 * OutputNumber: The desired output, starts at zero
 * EventNumber: The desired event, starts at zero
 * EventData: The event data copied from the event list
 */
void TimerSetEvent(uint8_t OutputNumber, uint8_t EventNumber, TimerEvent *EventData)
{
	if( (OutputNumber >= 0) && (OutputNumber < TIMER_OUTPUT_NUMBER) && (EventNumber >= 0) && (EventNumber < TIMER_EVENT_NUMBER) )
	{
		TimerEventList[OutputNumber][EventNumber].EventType = EventData->EventType;
		TimerEventList[OutputNumber][EventNumber].EventTime[0] = EventData->EventTime[0];
		TimerEventList[OutputNumber][EventNumber].EventTime[1] = EventData->EventTime[1];
		TimerEventList[OutputNumber][EventNumber].EventTime[2] = EventData->EventTime[2];
		TimerEventList[OutputNumber][EventNumber].EventOutputState = EventData->EventOutputState;
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

/**Write all events from the global event list to EEPROM
 *  TODO: make this return the maximum EEPROM address used?
 *
 *  Returns 0xFF if there is an error reading, otherwise returns 0
 */
uint8_t TimerWriteEventsToEEPROM(void)
{
	uint8_t i, j, sz;
	uint16_t addr;

	addr = TIMER_EEPROM_START_ADDRESS;		//The address of the current struct in EEPROM
	sz = sizeof(TimerEventList[0][0]);		//The size of the structs, this should always be the same

	for(i=0; i<TIMER_OUTPUT_NUMBER; i++)
	{
		for(j=0; j<TIMER_EVENT_NUMBER; j++)
		{
			if( EEPROM_Write(addr, &TimerEventList[i][j], sz ) != 0)
			{
				return 0xFF;
			}
			addr += sz;
		}
	}

	return 0x00;
}

/**Read all events from EEPROM and write them to the global event list
 *
 *  Returns 0xFF if there is an error reading, otherwise returns 0
 */
uint8_t TimerReadEventsFromEEPROM(void)
{
	uint8_t i, j, sz;
	uint16_t addr;

	addr = TIMER_EEPROM_START_ADDRESS;		//The address of the current struct in EEPROM
	sz = sizeof(TimerEventList[0][0]);		//The size of the structs, this should always be the same

	for(i=0; i<TIMER_OUTPUT_NUMBER; i++)
	{
		for(j=0; j<TIMER_EVENT_NUMBER; j++)
		{
			if( EEPROM_Read(addr, &TimerEventList[i][j], sz ) != 0)
			{
				return 0xFF;
			}
			addr += sz;
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

	if( EEPROM_Write(addr, &EventData, sz ) != 0)
	{
		return 0xFF;
	}

	return 0x00;
}

uint8_t InitTimerTask(void)
{
	int8_t i, j;
	//TODO: Code to read timer events from EEPROM goes here...
	//TODO: Check if timer tasks are not set?

	if(TimerReadEventsFromEEPROM() != 0)
	{
		return 0x01;	//TODO: Make this a specific error code
	}

	TimerStatus = TIMER_STATUS_OFF;

	OutputStatus = 0x00;
	TimerSetOutput(0, 0);
	TimerSetOutput(1, 0);
	TimerSetOutput(2, 0);
	TimerSetOutput(3, 0);


	//Check to make sure the events read from EEPROM are valid
	for(i=0;i<TIMER_OUTPUT_NUMBER;i++)
	{
		for(j=0;j<TIMER_EVENT_NUMBER;j++)
		{
			if( (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_TIME_EVENT) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_REPEATING_EVENT) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_SUN_EVENT) || (*TimerEventList[i][j].EventTime != TIMER_TASK_TYPE_STEADY_EVENT) )
			{
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


	TimerEventList[1][0].EventType = TIMER_TASK_TYPE_STEADY_EVENT;
	TimerEventList[1][0].EventTime[0] = 0;
	TimerEventList[1][0].EventTime[1] = 0;
	TimerEventList[1][0].EventTime[2] = 0;
	TimerEventList[1][0].EventOutputState = 0;

	TimerEventList[0][0].EventType = TIMER_TASK_TYPE_TIME_EVENT;
	TimerEventList[0][0].EventTime[0] = 0xFF;
	TimerEventList[0][0].EventTime[1] = 4;
	TimerEventList[0][0].EventTime[2] = 23;
	TimerEventList[0][0].EventOutputState = 0;

	TimerEventList[2][0].EventType = TIMER_TASK_TYPE_TIME_EVENT;
	TimerEventList[2][0].EventTime[0] = 18;
	TimerEventList[2][0].EventTime[1] = 18;
	TimerEventList[2][0].EventTime[2] = 21;
	TimerEventList[2][0].EventOutputState = 0;

	/*TimerEventList[3][0].EventType = TIMER_TASK_TYPE_STEADY_EVENT;
	TimerEventList[3][0].EventTime[0] = 0;
	TimerEventList[3][0].EventTime[1] = 0;
	TimerEventList[3][0].EventTime[2] = 0;
	TimerEventList[3][0].EventOutputState = 0;*/

	xTimerCommands = xQueueCreate( 2, sizeof( uint8_t ) );

	return 0;
}

void TimerTask(void *pvParameter)
{
	//TimeAndDate CurrentTime;
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
				TimerStatus = TIMER_STATUS_ON;
				TimerUpdateOutputs();
				break;

			case TIMER_TASK_CMD_STOP:
				TimerStatus = TIMER_STATUS_OFF;
				TimerSetOutput(0, 0);
				TimerSetOutput(1, 0);
				TimerSetOutput(2, 0);
				TimerSetOutput(3, 0);
				break;

			case TIMER_TASK_CMD_PAUSE:
				TimerStatus = TIMER_STATUS_PAUSED;
				//Add something here to set the outputs to their override state.
				break;


		}

	}

	return;
}


/** Update all the outputs based on the current time */
void TimerUpdateOutputs(void)
{
	TimeAndDate CurrentTime;
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


