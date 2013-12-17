/*
 * dfu.c
 *
 *  Created on: Nov 16, 2013
 *      Author: pat
 */


#include "board.h"
#include "dfu.h"
#include "main.h"
//#include "LPC11Uxx.h"
//#include "gpio.h"
//#include "clkconfig.h"
//#include "config.h"
//#include "wdt.h"

/* This data must be global so it is not read from the stack */
//typedef void (*IAP)(uint32_t [], uint32_t []);
//IAP iap_entry = (IAP)0x1fff1ff1;

/*
typedef void (*IAP_ENTRY_T)(unsigned int[], unsigned int[]);

static INLINE void iap_entry(unsigned int cmd_param[], unsigned int status_result[])
{
	((IAP_ENTRY_T) IAP_ENTRY_LOCATION)(cmd_param, status_result);
}
*/
void WDTInit( void );


uint32_t command[5], result[4];
#define init_msdstate() *((uint32_t *)(0x10000054)) = 0x0

/* This function resets some microcontroller peripherals to reset
   hardware configuration to ensure that the USB In-System Programming module
   will work properly. It is normally called from reset and assumes some reset
   configuration settings for the MCU.
   Some of the peripheral configurations may be redundant in your specific
   project.
*/
void ReinvokeISP(void)
{
	vPortEnterCritical();

	USBD_API->hw->Connect(g_hUsb, 0);

	//WDTInit();

  /* make sure USB clock is turned on before calling ISP */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_USB);
  //LPC_SYSCON->SYSAHBCLKCTRL |= 0x04000;
  /* make sure 32-bit Timer 1 is turned on before calling ISP */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CT32B1);
  //LPC_SYSCON->SYSAHBCLKCTRL |= 0x00400;
  /* make sure GPIO clock is turned on before calling ISP */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GPIO);
  //LPC_SYSCON->SYSAHBCLKCTRL |= 0x00040;
  /* make sure IO configuration clock is turned on before calling ISP */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
  //LPC_SYSCON->SYSAHBCLKCTRL |= 0x10000;

  /* make sure AHB clock divider is 1:1 */
	Chip_Clock_SetSysClockDiv(1);
  //LPC_SYSCON->SYSAHBCLKDIV = 1;

  /* Send Reinvoke ISP command to ISP entry point*/
  command[0] = 57;

  init_msdstate();					 /* Initialize Storage state machine */
  /* Set stack pointer to ROM value (reset default) This must be the last
     piece of code executed before calling ISP, because most C expressions
     and function returns will fail after the stack pointer is changed. */
  __set_MSP(*((uint32_t *)0x00000000));

  /* Enter ISP. We call "iap_entry" to enter ISP because the ISP entry is done
     through the same command interface as IAP. */
  iap_entry(command, result);
  // Not supposed to come back!
}

void WDTInit( void )
{
	uint32_t i;
	//uint32_t wdtFreq;

	/* Initialize WWDT (also enables WWDT clock) */
	Chip_WWDT_Init(LPC_WWDT);

	/* Prior to initializing the watchdog driver, the clocking for the
	   watchdog must be enabled. This example uses the watchdog oscillator
	   set at a 50KHz (1Mhz / 20) clock rate. */
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_WDTOSC_PD);
	Chip_Clock_SetWDTOSC(WDTLFO_OSC_1_05, 20);

	/* The WDT divides the input frequency into it by 4 */
	//wdtFreq = Chip_Clock_GetWDTOSCRate() / 4;

	/* LPC1102/4, LPC11XXLV, and LPC11CXX devices select the watchdog
	   clock source from the SYSCLK block, while LPC11AXX, LPC11EXX, and
	   LPC11UXX devices select the clock as part of the watchdog block. */
	/* Select watchdog oscillator for WDT clock source */
	Chip_WWDT_SelClockSource(LPC_WWDT, WWDT_CLKSRC_WATCHDOG_WDOSC);

	Chip_WWDT_SetTimeOut(LPC_WWDT, 0x7FFF);
	Chip_WWDT_SetOption(LPC_WWDT, (WWDT_WDMOD_WDEN|WWDT_WDMOD_WDRESET));
	Chip_WWDT_Feed(LPC_WWDT);

	/* Make sure feed sequence executed properly */
	for (i = 0; i < 0x80; i++);

	return;
}
