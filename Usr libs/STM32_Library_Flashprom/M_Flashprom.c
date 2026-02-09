/*
 * M_Flashprom.c
 *
 *  Created on: Feb 9, 2026
 *      Author: aborav
 */



#include "STM32_Library_Flashprom/M_Flashprom.h"



/*
 * @brief Write array to MCU flash
 * @param add -> address in flash
 * @param arr -> array pointer
 * @param len -> array length
 */
uint8_t MFLPR_WriteArray(uint32_t addr, uint16_t *arr, uint8_t len) {
	/*0x0801 F800 - 0x0801 FFFF -> last Flash page for G4 with 128kB
	 only one bank -> 0
	 only 64 bit double word can be programmed
	 one devision is byte
	 */

	//maybe there is a way to just move pointer and save data directly into flash, without buf16
	uint64_t buf64; ///<final variable for HAL_FLASH_Program function
	uint64_t *buf64_ptr; ///<pointer to make 64 bit variable from buf16
	uint16_t buf16[4]; ///<4 array element chunks
	uint8_t buf_cnt = 0; ///<counter to make chunk from 4 uint16_t
	uint8_t addr_ptr = 0; ///<move pointer in flash stack
	uint32_t write_addr;
	uint8_t status;

	for (uint16_t i = 0; i < len; i++) {

		*(buf16 + buf_cnt) = *(arr + i); //write input array into temporary uint16_t array buffer

		//if array is full, flush into flash
		//else continue filling buf16
		if (buf_cnt > 3) {
			buf_cnt = 0;
			buf64_ptr = (uint64_t*) &buf16;
			buf64 = *buf64_ptr;
			write_addr = addr + (8U * addr_ptr);
			HAL_FLASH_Unlock();
			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, write_addr, buf64);
			HAL_FLASH_Lock();
			//printf("Write to flash:%li\n\r", (long int) buf64);
			//printf("Flash address:%li\n\r", addr + (8U * addr_ptr));
			addr_ptr++; //move flash address when bu16 is full
			__NOP();
			if (status != 0) {
				return status;
			}
		} else {
			buf_cnt++;
		}
		__NOP();
	}

	return status;

}
