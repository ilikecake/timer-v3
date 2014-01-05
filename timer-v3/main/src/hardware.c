/*
 * hardware.c
 *
 *  Created on: Dec 13, 2013
 *      Author: pat
 */

#include "main.h"

#define BUZZ_TIMER						LPC_TIMER16_1
#define BUZZ_TIMER_IRQ_HANDLER			TIMER16_1_IRQHandler
#define BUZZ_TIMER_NVIC_NAME			TIMER_16_1_IRQn

//Setup for the button pins
//Note: if the GPIO function for the pin is not function 0, the init function will have to be modified
#define BUTTON_1_PORT					0
#define BUTTON_1_PIN					18
#define BUTTON_1_PININT_INDEX			1
#define BUTTON_1_IRQ_HANDLER			FLEX_INT1_IRQHandler	/* IRQ function name */
#define BUTTON_1_NVIC_NAME				PIN_INT1_IRQn			/* NVIC interrupt name */

#define BUTTON_2_PORT				0
#define BUTTON_2_PIN					19
#define BUTTON_2_PININT_INDEX		2
#define BUTTON_2_IRQ_HANDLER			FLEX_INT2_IRQHandler	/* IRQ function name */
#define BUTTON_2_NVIC_NAME			PIN_INT2_IRQn			/* NVIC interrupt name */

#define BUTTON_3_PORT				1
#define BUTTON_3_PIN					16
#define BUTTON_3_PININT_INDEX		3
#define BUTTON_3_IRQ_HANDLER			FLEX_INT3_IRQHandler	/* IRQ function name */
#define BUTTON_3_NVIC_NAME			PIN_INT3_IRQn			/* NVIC interrupt name */

#define BUTTON_4_PORT				1
#define BUTTON_4_PIN				19
#define BUTTON_4_PININT_INDEX		4
#define BUTTON_4_IRQ_HANDLER		FLEX_INT4_IRQHandler	/* IRQ function name */
#define BUTTON_4_NVIC_NAME			PIN_INT4_IRQn			/* NVIC interrupt name */

#define BUTTON_5_PORT				1
#define BUTTON_5_PIN				25
#define BUTTON_5_PININT_INDEX		5
#define BUTTON_5_IRQ_HANDLER		FLEX_INT5_IRQHandler	/* IRQ function name */
#define BUTTON_5_NVIC_NAME			PIN_INT5_IRQn			/* NVIC interrupt name */




uint8_t ProgramStatus;


void App_SSP_Init (void)
{
	//Setup pin muxing for SPI pins.
	//Note: CS pins are not set up here
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 10, (IOCON_FUNC2 | IOCON_MODE_PULLUP));	/* SCK0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8, (IOCON_FUNC1 | IOCON_MODE_PULLUP));	/* MISO0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 9, (IOCON_FUNC1 | IOCON_MODE_PULLUP));	/* MOSI0 */

	Chip_SSP_Init(LPC_SSP0);
	Chip_SSP_SetFormat(LPC_SSP0, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_CPHA1_CPOL1);
	Chip_SSP_SetMaster(LPC_SSP0, 1);		//Set to master mode
	Chip_SSP_SetBitRate(LPC_SSP0, 8000000);	//Set SSP0 frequency to 8MHz
	Chip_SSP_Enable(LPC_SSP0);

	return;
}

void App_Button_Init(void)
{

	//Set all button pins to GPIO input with pullup
	Chip_IOCON_PinMuxSet(LPC_IOCON, BUTTON_1_PORT, BUTTON_1_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP));
	Chip_IOCON_PinMuxSet(LPC_IOCON, BUTTON_2_PORT, BUTTON_2_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP));
	Chip_IOCON_PinMuxSet(LPC_IOCON, BUTTON_3_PORT, BUTTON_3_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP));
	Chip_IOCON_PinMuxSet(LPC_IOCON, BUTTON_4_PORT, BUTTON_4_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP));
	Chip_IOCON_PinMuxSet(LPC_IOCON, BUTTON_5_PORT, BUTTON_5_PIN, (IOCON_FUNC0 | IOCON_MODE_PULLUP));

	Chip_GPIO_SetPinDIRInput(LPC_GPIO, BUTTON_1_PORT, BUTTON_1_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, BUTTON_2_PORT, BUTTON_2_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, BUTTON_3_PORT, BUTTON_3_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, BUTTON_4_PORT, BUTTON_4_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, BUTTON_5_PORT, BUTTON_5_PIN);


	//TODO: Probably put this in the main initalization...
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_PINT);

	//Setup GPIO interrupts for each button

	/* Configure interrupt channel for the GPIO pin in SysCon block */
	Chip_SYSCTL_SetPinInterrupt(BUTTON_1_PININT_INDEX, BUTTON_1_PORT, BUTTON_1_PIN);
	Chip_SYSCTL_SetPinInterrupt(BUTTON_2_PININT_INDEX, BUTTON_2_PORT, BUTTON_2_PIN);
	Chip_SYSCTL_SetPinInterrupt(BUTTON_3_PININT_INDEX, BUTTON_3_PORT, BUTTON_3_PIN);
	Chip_SYSCTL_SetPinInterrupt(BUTTON_4_PININT_INDEX, BUTTON_4_PORT, BUTTON_4_PIN);
	Chip_SYSCTL_SetPinInterrupt(BUTTON_5_PININT_INDEX, BUTTON_5_PORT, BUTTON_5_PIN);

	/* Configure channel interrupt as edge sensitive and falling edge interrupt */
	Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(BUTTON_1_PININT_INDEX));
	Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(BUTTON_2_PININT_INDEX));
	Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(BUTTON_3_PININT_INDEX));
	Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(BUTTON_4_PININT_INDEX));
	Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(BUTTON_5_PININT_INDEX));
	Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(BUTTON_1_PININT_INDEX));
	Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(BUTTON_2_PININT_INDEX));
	Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(BUTTON_3_PININT_INDEX));
	Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(BUTTON_4_PININT_INDEX));
	Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(BUTTON_5_PININT_INDEX));

	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_1_PININT_INDEX));
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_2_PININT_INDEX));
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_3_PININT_INDEX));
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_4_PININT_INDEX));
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_5_PININT_INDEX));

	NVIC_ClearPendingIRQ(BUTTON_1_NVIC_NAME);
	NVIC_ClearPendingIRQ(BUTTON_2_NVIC_NAME);
	NVIC_ClearPendingIRQ(BUTTON_3_NVIC_NAME);
	NVIC_ClearPendingIRQ(BUTTON_4_NVIC_NAME);
	NVIC_ClearPendingIRQ(BUTTON_5_NVIC_NAME);

	NVIC_EnableIRQ(BUTTON_1_NVIC_NAME);
	NVIC_EnableIRQ(BUTTON_2_NVIC_NAME);
	NVIC_EnableIRQ(BUTTON_3_NVIC_NAME);
	NVIC_EnableIRQ(BUTTON_4_NVIC_NAME);
	NVIC_EnableIRQ(BUTTON_5_NVIC_NAME);

	return;
}

void App_HandleButtonPress(uint8_t ButtonNumber)
{
	if(App_GetStatus() != APP_STATUS_INIT)
	{
		DisplayCommand CommandToSend;

		CommandToSend.CommandName = OLED_CMD_BUTTON_IN;
		CommandToSend.CommandData[0] = ButtonNumber;
		xQueueSendFromISR(xDisplayCommands, (void *)&CommandToSend, NULL);
	}
	return;
}

void BUTTON_1_IRQ_HANDLER(void)
{
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_1_PININT_INDEX));
	App_HandleButtonPress(BUTTON_1_PININT_INDEX);
	return;
}

void BUTTON_2_IRQ_HANDLER(void)
{
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_2_PININT_INDEX));
	App_HandleButtonPress(BUTTON_2_PININT_INDEX);
	return;
}

void BUTTON_3_IRQ_HANDLER(void)
{
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_3_PININT_INDEX));
	App_HandleButtonPress(BUTTON_3_PININT_INDEX);
	return;
}

void BUTTON_4_IRQ_HANDLER(void)
{
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_4_PININT_INDEX));
	App_HandleButtonPress(BUTTON_4_PININT_INDEX);
	return;
}

void BUTTON_5_IRQ_HANDLER(void)
{
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(BUTTON_5_PININT_INDEX));
	App_HandleButtonPress(BUTTON_5_PININT_INDEX);
	return;
}

void App_SetStatus(uint8_t Status)
{
	ProgramStatus = Status;
	return;
}

uint8_t App_GetStatus(void)
{
	return ProgramStatus;
}


void App_Buzzer_Init(void)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 17, IOCON_FUNC0);		//Set the buzz pin to GPIO
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 17);					//Set the buzz pin to output
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 17, 0);					//Set the buzz pin low

	//Initialize the timer for the buzzer, but don't start it
	Chip_TIMER_Init(BUZZ_TIMER);
	Chip_TIMER_Reset(BUZZ_TIMER);

	BUZZ_TIMER->MCR = (1<<1)|(1<<0);		//Enable MR0 match interrupt, Reset TC on MR0 match
	BUZZ_TIMER->MR[0]= 12000;			//MR0 match value

	//Enable the IRQ for the timer
	NVIC_EnableIRQ(BUZZ_TIMER_NVIC_NAME);

	return;
}

void App_Buzzer_on (void)
{
	BUZZ_TIMER->TCR = 0x01;
	return;
}

void App_Buzzer_off (void)
{
	BUZZ_TIMER->TCR = 0x00;
	Chip_TIMER_Reset(BUZZ_TIMER);
	BUZZ_TIMER->IR = 0xFF;
	return;
}

void BUZZ_TIMER_IRQ_HANDLER(void)
{
	BUZZ_TIMER->IR = 0xFF;
	Chip_GPIO_SetPinToggle(LPC_GPIO, 0, 17);
}
