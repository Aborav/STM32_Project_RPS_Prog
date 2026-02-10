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
	uint64_t *buf64_ptr; ///<pointer to make 64 bit variable from buf16
	uint8_t resid; ///<residual after quantity of 16 bit array divided 4
	uint8_t status;

	//if 16 bit array can't be equally divided by 4, left one chunk saved in 64 bit cell
	resid = len % 4;
	if (resid > 0) {
		len -= resid;
		len += 4;
	}
	len >>= 2; //len /= 4;

	for (uint8_t i = 0; i < len; i++) {
		buf64_ptr = (uint64_t*) (arr + 4U * i); //move array 64 bit pointer
		HAL_FLASH_Unlock();
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + 8U * i, (uint64_t) *buf64_ptr);
		HAL_FLASH_Lock();
		if (status != 0) {
			break;
		}

	}
	return status;
}
