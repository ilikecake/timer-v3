/*
 * hardware.h
 *
 *  Created on: Dec 13, 2013
 *      Author: pat
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_


//Pick more statuses later...
//Status should include error states and also normal program states
#define APP_STATUS_INIT			0x00
#define APP_STATUS_OK			0x01
#define APP_STATUS_OSC_STOPPED	0x02





void App_SSP_Init (void);



void App_SetStatus(uint8_t Status);
uint8_t App_GetStatus(void);

//TODO: Add a function to enable and disable the interrupts
void App_Button_Init(void);
void App_HandleButtonPress(void);

void App_EnableButtons(void);
void App_DisableButtons(void);



void App_Buzzer_Init(void);
void App_Buzzer_on (void);
void App_Buzzer_off (void);

#endif /* HARDWARE_H_ */
