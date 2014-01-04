
#include "main.h"

#define init_msdstate() *((uint32_t *)(0x10000054)) = 0x0

#ifdef USE_EEPROM_LIB
uint8_t EEPROM_Write(uint8_t* AddressToWrite, uint8_t* DataArray, uint8_t BytesToWrite)
{
	EELIB_Command IAP_Command;
	EELIB_Result IAP_Result;

	IAP_Result[0] = 0;

	//TODO: check to make sure the address is valid

	IAP_Command[0] = EELIB_IAP_COMMAND_EEPROM_WRITE;	//EEPROM write command
	IAP_Command[1] = (uint32_t)AddressToWrite;			//EEPROM address to write to
	IAP_Command[2] = (uint32_t)DataArray;				//RAM address of the data to write
	IAP_Command[3] = (uint32_t)BytesToWrite;			//Number of bytes to write
	IAP_Command[4] = SystemCoreClock/1000;				//System clock frequency in kHz

	EELIB_entry(IAP_Command, IAP_Result);

	return (uint8_t)IAP_Result[0];
}

uint8_t EEPROM_Read(uint8_t* AddressToRead, uint8_t* DataArray, uint8_t BytesToRead)
{
	EELIB_Command IAP_Command;
	EELIB_Result IAP_Result;

	IAP_Result[0] = 0;

	IAP_Command[0] = EELIB_IAP_COMMAND_EEPROM_READ;		//EEPROM read command
	IAP_Command[1] = (uint32_t)AddressToRead;			//EEPROM address to read from
	IAP_Command[2] = (uint32_t)DataArray;				//RAM address to copy the EEPROM data to
	IAP_Command[3] = (uint32_t)BytesToRead;				//Number of bytes to read
	IAP_Command[4] = SystemCoreClock/1000;				//System clock frequency in kHz

	EELIB_entry(IAP_Command, IAP_Result);

	return (uint8_t)IAP_Result[0];
}



#else

uint8_t EEPROM_Write(uint8_t* AddressToWrite, uint8_t* DataArray, uint8_t BytesToWrite)
{
	unsigned int IAP_Command[5];
	unsigned int IAP_Result[5];

	//SystemCoreClockUpdate();

	IAP_Result[0] = 0;
	IAP_Result[1] = 0;
	IAP_Result[2] = 0;
	IAP_Result[3] = 0;
	IAP_Result[4] = 0;

	//TODO: check to make sure the address is valid

	IAP_Command[0] = IAP_EEPROM_WRITE;					//EEPROM write command (61)
	IAP_Command[1] = (uint32_t)AddressToWrite;			//EEPROM address to write to
	IAP_Command[2] = (uint32_t)DataArray;				//RAM address of the data to write
	IAP_Command[3] = (uint32_t)BytesToWrite;			//Number of bytes to write
	IAP_Command[4] = SystemCoreClock/1000;				//System clock frequency in kHz

	vPortEnterCritical();
	iap_entry(IAP_Command, IAP_Result);
	vPortExitCritical();

	return (uint8_t)IAP_Result[0];
}

uint8_t EEPROM_Read(uint8_t* AddressToRead, uint8_t* DataArray, uint8_t BytesToRead)
{
	unsigned int IAP_Command[5];
	unsigned int IAP_Result[4];

	SystemCoreClockUpdate();

	IAP_Result[0] = 0;
	IAP_Result[1] = 0;
	IAP_Result[2] = 0;
	IAP_Result[3] = 0;
	//IAP_Result[4] = 0;

	//TODO: check to make sure the address is valid

	IAP_Command[0] = IAP_EEPROM_READ;						//EEPROM read command (62)
	IAP_Command[1] = (uint32_t)AddressToRead;				//EEPROM address to read from
	IAP_Command[2] = (uint32_t)DataArray;					//RAM address to copy the EEPROM data to
	IAP_Command[3] = (uint32_t)BytesToRead;					//Number of bytes to read
	IAP_Command[4] = SystemCoreClock/1000;					//System clock frequency in kHz

	vPortEnterCritical();
	iap_entry(IAP_Command, IAP_Result);
	vPortExitCritical();

	return (uint8_t)IAP_Result[0];
}
#endif

uint8_t ReadUID(uint32_t *UID)
{
	unsigned int IAP_Command[5];
	unsigned int IAP_Result[5];

	SystemCoreClockUpdate();

	IAP_Result[0] = 0;

	IAP_Command[0] = IAP_READ_UID;						//Read UID command (58)

	vPortEnterCritical();
	iap_entry(IAP_Command, IAP_Result);
	vPortExitCritical();

	UID[0] = IAP_Result[1];
	UID[1] = IAP_Result[2];
	UID[2] = IAP_Result[3];
	UID[3] = IAP_Result[4];


	return (uint8_t)IAP_Result[0];
}

uint8_t ReadPartID(unsigned int *PID)
{
	unsigned int IAP_Command[5];
	unsigned int IAP_Result[5];

	SystemCoreClockUpdate();

	IAP_Result[0] = 0;
	IAP_Result[1] = 0;
	IAP_Result[2] = 0;
	IAP_Result[3] = 0;
	IAP_Result[4] = 0;

	IAP_Command[0] = IAP_READ_PART_ID;			//Read part ID command (54)

	vPortEnterCritical();
	iap_entry(IAP_Command, IAP_Result);
	vPortExitCritical();

	*PID = IAP_Result[1];

	return (uint8_t)IAP_Result[0];
}

uint8_t ReadBootVersion(uint32_t *BVID)
{
	unsigned int IAP_Command[5];
	unsigned int IAP_Result[5];

	SystemCoreClockUpdate();

	IAP_Result[0] = 0;
	IAP_Result[1] = 0;
	IAP_Result[2] = 0;
	IAP_Result[3] = 0;
	IAP_Result[4] = 0;

	IAP_Command[0] = IAP_READ_BOOT_CODE_VERSION;	//Read boot code version command (55)

	vPortEnterCritical();
	iap_entry(IAP_Command, IAP_Result);
	vPortExitCritical();

	*BVID = IAP_Result[1];

	return (uint8_t)IAP_Result[0];
}

void ReinvokeISP(void)
{
	unsigned int IAP_Command[5];
	unsigned int IAP_Result[5];

	vPortEnterCritical();


	/* make sure USB clock is turned on before calling ISP */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_USB);

	/* make sure 32-bit Timer 1 is turned on before calling ISP */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CT32B1);

	/* make sure GPIO clock is turned on before calling ISP */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GPIO);

	/* make sure IO configuration clock is turned on before calling ISP */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);

	/* make sure AHB clock divider is 1:1 */
	Chip_Clock_SetSysClockDiv(1);

	/* Send Reinvoke ISP command to ISP entry point*/
	IAP_Command[0] = IAP_REINVOKE_ISP;	//Reinvoke bootloader command (57)

	init_msdstate();					 /* Initialize Storage state machine */
	/* Set stack pointer to ROM value (reset default) This must be the last
     piece of code executed before calling ISP, because most C expressions
     and function returns will fail after the stack pointer is changed. */
	__set_MSP(*((uint32_t *)0x00000000));

	/* Enter ISP. We call "iap_entry" to enter ISP because the ISP entry is done
     through the same command interface as IAP. */
	iap_entry(IAP_Command, IAP_Result);
	// Not supposed to come back!
	vPortExitCritical();
}

