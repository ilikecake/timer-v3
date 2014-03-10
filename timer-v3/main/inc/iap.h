/*
 * eeprom.h
 *
 *  Created on: Dec 26, 2013
 *      Author: pat
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#define USE_EEPROM_LIB



#define IAP_PREPARE_SECTOR_FOR_WRITE	50
#define IAP_COPY_RAM_TO_FLASH			51
#define IAP_ERASE_SECTOR				52
#define IAP_BLANK_CHECK_SECTOR			53
#define IAP_READ_PART_ID				54
#define IAP_READ_BOOT_CODE_VERSION		55
#define IAP_COMPARE						56
#define IAP_REINVOKE_ISP				57
#define IAP_READ_UID					58
#define IAP_ERASE_PAGE					59
#define IAP_EEPROM_WRITE				61
#define IAP_EEPROM_READ					62

//IAP status codes
#define IAP_CMD_SUCCESS					0
#define IAP_INVALID_COMMAND				1
#define IAP_SRC_ADDR_ERROR				2
#define IAP_DST_ADDR_ERROR				3
#define IAP_SRC_ADDR_NOT_MAPPED			4
#define IAP_DST_ADDR_NOT_MAPPED			5
#define IAP_COUNT_ERROR					6
#define IAP_INVALID_SECTOR				7
#define IAP_SECTOR_NOT_BLANK			8
#define IAP_SECTOR_NOT_READY			9
#define IAP_COMPARE_ERROR				10
#define IAP_BUSY						11

uint8_t EEPROM_Write(uint16_t AddressToWrite, void* DataArray, uint16_t BytesToWrite);
uint8_t EEPROM_Read(uint16_t AddressToRead, void* DataArray, uint16_t BytesToRead);

//void readEEPROM( uint8_t* eeAddress, uint8_t* buffAddress, uint32_t byteCount );
//void writeEEPROM( uint8_t* eeAddress, uint8_t* buffAddress, uint32_t byteCount );

uint8_t ReadUID(uint32_t *UID);
uint8_t ReadPartID(unsigned int *PID);
uint8_t ReadBootVersion(uint32_t *BVID);
void ReinvokeISP(void);

void RequestISP(void);

#endif /* EEPROM_H_ */
