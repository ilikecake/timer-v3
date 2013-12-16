/*
 * i2c.c
 *
 *  Created on: Nov 13, 2013
 *      Author: pat
 */


/*
 * @brief I2C example.
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "main.h"

//#include <stdlib.h>
//#include <string.h>
//#include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

//#define DEFAULT_I2C          I2C0
//
//#define I2C_EEPROM_BUS       DEFAULT_I2C
//#define I2C_IOX_BUS          DEFAULT_I2C
//
//#define SPEED_100KHZ         100000
//#define SPEED_400KHZ         400000
//#define I2C_DEFAULT_SPEED    SPEED_100KHZ
//#define I2C_FASTPLUS_BIT     0
//
//#if (I2C_DEFAULT_SPEED > SPEED_400KHZ)
//#undef  I2C_FASTPLUS_BIT
//#define I2C_FASTPLUS_BIT IOCON_FASTI2C_EN
//#endif
//
////#ifdef DEBUG_ENABLE
////static const char menu[] =
////	"**************** I2C Demo Menu ****************\r\n"
////	"\t0: Exit Demo\r\n"
////	"\t1: Select I2C peripheral [\033[1;32mI2C%d\033[0;37m]\r\n"
////	"\t2: Toggle mode POLLING/INTERRUPT [\033[1;32m%s\033[0;37m]\r\n"
////	"\t3: Probe for Slave devices\r\n"
////	"\t4: Read slave data\r\n"
////	"\t5: Write slave data\r\n"
////	"\t6: Write/Read slave data\r\n";
////#endif
//
//static int mode_poll;	/* Poll/Interrupt mode flag */
//static I2C_ID_T i2cDev = DEFAULT_I2C;	/* Currently active I2C device */

/* EEPROM SLAVE data */
//#define I2C_SLAVE_EEPROM_SIZE       64
//#define I2C_SLAVE_EEPROM_ADDR       0x5A
//#define I2C_SLAVE_IOX_ADDR          0x5B

///* Xfer structure for slave operations */
//static I2C_XFER_T seep_xfer;
//static I2C_XFER_T iox_xfer;
//
///* Data area for slave operations */
//static uint8_t seep_data[I2C_SLAVE_EEPROM_SIZE + 1];
//static uint8_t buffer[2][256];
//static uint8_t iox_data[2];	/* PORT0 input port, PORT1 Output port */
//static volatile uint32_t tick_cnt;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

static void Init_I2C_PinMux(void)
{
#if (defined(BOARD_NXP_XPRESSO_11U14) || defined(BOARD_NGX_BLUEBOARD_11U24))
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
#elif (defined(BOARD_NXP_XPRESSO_11C24) || defined(BOARD_MCORE48_1125))
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_4, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_5, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
#else
	#error "No Pin Muxing defined for I2C operation"
#endif
}

/* State machine handler for I2C0 and I2C1 */
static void i2c_state_handling(I2C_ID_T id)
{
	if (Chip_I2C_IsMasterActive(id)) {
		Chip_I2C_MasterStateHandler(id);
	}
	else {
		Chip_I2C_SlaveStateHandler(id);
	}
}

/* Set I2C mode to polling/interrupt */
/* polling = 1 for polling mode, 0 for interrupt mode */
static void i2c_set_mode(I2C_ID_T id, int polling)
{
	if (!polling) {
		mode_poll &= ~(1 << id);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandler);
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	else {
		mode_poll |= 1 << id;
		NVIC_DisableIRQ(I2C0_IRQn);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandlerPolling);
	}
}

/* Initialize the I2C bus */
void i2c_app_init(I2C_ID_T id, int speed)
{
	Init_I2C_PinMux();

	/* Initialize I2C */
	Chip_I2C_Init(id);
	Chip_I2C_SetClockRate(id, speed);

	/* Set default mode to interrupt */
	i2c_set_mode(id, I2C_MODE_POLLING);
}

/* Function that probes all available slaves connected to an I2C bus */
void i2c_probe_slaves(I2C_ID_T i2c)
{
	int i;
	uint8_t ch[2];

	DEBUGOUT("Probing available I2C devices...\r\n");
	DEBUGOUT("\r\n     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	DEBUGOUT("\r\n====================================================");
	for (i = 0; i <= 0x7F; i++) {
		if (!(i & 0x0F)) {
			DEBUGOUT("\r\n%02X  ", i >> 4);
		}
		if ((i <= 7) || (i > 0x78)) {
			DEBUGOUT("   ");
			continue;
		}
		if (Chip_I2C_MasterRead(i2c, i, ch, 1) > 0) {
			DEBUGOUT(" %02X", i);
		}
		else {
			DEBUGOUT(" --");
		}
	}
	DEBUGOUT("\r\n");
}

/**
 * @brief	I2C Interrupt Handler
 * @return	None
 */
void I2C_IRQHandler(void)
{
	i2c_state_handling(I2C0);
}
