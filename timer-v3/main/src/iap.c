
#include "main.h"

#ifdef USE_EEPROM_LIB
uint8_t EEPROM_Write(uint8_t AddressToWrite, uint8_t *DataArray, uint8_t BytesToWrite)
{
	EELIB_Command IAP_Command;
	EELIB_Result IAP_Result;

	SystemCoreClockUpdate();

	IAP_Result[0] = 0;

	//TODO: check to make sure the address is valid

	IAP_Command[0] = EELIB_IAP_COMMAND_EEPROM_WRITE;	//EEPROM write command
	IAP_Command[1] = (unsigned int)AddressToWrite;		//EEPROM address to write to
	IAP_Command[2] = (unsigned int)*DataArray;			//RAM address of the data to write
	IAP_Command[3] = (unsigned int)BytesToWrite;		//Number of bytes to write
	IAP_Command[4] = (unsigned int)48000;				//System clock frequency in kHz

	vPortEnterCritical();
	EELIB_entry(IAP_Command, IAP_Result);
	vPortExitCritical();

	return (uint8_t)IAP_Result[0];
}

uint8_t EEPROM_Read(uint8_t AddressToRead, uint8_t *DataArray, uint8_t BytesToRead)
{
	EELIB_Command IAP_Command;
	EELIB_Result IAP_Result;

	SystemCoreClockUpdate();

	IAP_Result[0] = 0;

	//TODO: check to make sure the address is valid

	IAP_Command[0] = EELIB_IAP_COMMAND_EEPROM_READ;	//EEPROM read command
	IAP_Command[1] = (unsigned int)AddressToRead;	//EEPROM address to read from
	IAP_Command[2] = (unsigned int)*DataArray;		//RAM address to copy the EEPROM data to
	IAP_Command[3] = (unsigned int)BytesToRead;		//Number of bytes to read
	IAP_Command[4] = (unsigned int)48000;			//System clock frequency in kHz

	vPortEnterCritical();
	EELIB_entry(IAP_Command, IAP_Result);
	vPortExitCritical();

	return (uint8_t)IAP_Result[0];
}
#else
uint8_t EEPROM_Write(uint8_t AddressToWrite, uint8_t *DataArray, uint8_t BytesToWrite)
{
	unsigned int IAP_Command[5];
	unsigned int IAP_Result[5];

	SystemCoreClockUpdate();

	IAP_Result[0] = 0;
	IAP_Result[1] = 0;
	IAP_Result[2] = 0;
	IAP_Result[3] = 0;
	IAP_Result[4] = 0;

	//TODO: check to make sure the address is valid

	IAP_Command[0] = (unsigned int)IAP_EEPROM_WRITE;					//EEPROM write command (61)
	IAP_Command[1] = (unsigned int)AddressToWrite;		//EEPROM address to write to
	IAP_Command[2] = (unsigned int)*DataArray;						//RAM address of the data to write
	IAP_Command[3] = (unsigned int)BytesToWrite;		//Number of bytes to write
	IAP_Command[4] = (unsigned int)48000;				//System clock frequency in kHz

	vPortEnterCritical();
	iap_entry(IAP_Command, IAP_Result);
	vPortExitCritical();

	return (uint8_t)IAP_Result[0];
}

uint8_t EEPROM_Read(uint8_t AddressToRead, uint8_t *DataArray, uint8_t BytesToRead)
{
	unsigned int IAP_Command[5];
	unsigned int IAP_Result[5];

	SystemCoreClockUpdate();

	IAP_Result[0] = 0;
	IAP_Result[1] = 0;
	IAP_Result[2] = 0;
	IAP_Result[3] = 0;
	IAP_Result[4] = 0;

	//TODO: check to make sure the address is valid

	IAP_Command[0] = (unsigned int)IAP_EEPROM_READ;				//EEPROM read command (62)
	IAP_Command[1] = (unsigned int)AddressToRead;	//EEPROM address to read from
	IAP_Command[2] = (unsigned int)*DataArray;					//RAM address to copy the EEPROM data to
	IAP_Command[3] = (unsigned int)BytesToRead;		//Number of bytes to read
	IAP_Command[4] = (unsigned int)48000;			//System clock frequency in kHz

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

