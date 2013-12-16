/*
 * i2c.h
 *
 *  Created on: Nov 13, 2013
 *      Author: pat
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdlib.h>
#include <string.h>
#include "board.h"



#define DEFAULT_I2C          I2C0

//#define I2C_EEPROM_BUS       DEFAULT_I2C
//#define I2C_IOX_BUS          DEFAULT_I2C

#define SPEED_100KHZ         100000
#define SPEED_400KHZ         400000
#define I2C_DEFAULT_SPEED    SPEED_100KHZ
#define I2C_FASTPLUS_BIT     0

#define I2C_MODE_POLLING	1
#define I2C_MODE_INTERRUPT	0



#if (I2C_DEFAULT_SPEED > SPEED_400KHZ)
#undef  I2C_FASTPLUS_BIT
#define I2C_FASTPLUS_BIT IOCON_FASTI2C_EN
#endif

static int mode_poll;	/* Poll/Interrupt mode flag */ //TODO: What is this used for?
//static I2C_ID_T i2cDev = DEFAULT_I2C;	/* Currently active I2C device */

void i2c_app_init(I2C_ID_T id, int speed);
void i2c_probe_slaves(I2C_ID_T i2c);

#endif /* I2C_H_ */
