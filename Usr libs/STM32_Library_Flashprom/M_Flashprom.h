/*
 * M_Flashprom.h
 *
 *  Created on: Feb 9, 2026
 *      Author: aborav
 */

#ifndef _M_FLASHPROM_H_
#define _M_FLASHPROM_H_

#include "main.h"

/*
Library to save your data into STM32 flash memory in static way
Suitable for not frequent overwriting and BIG data

HOW TO USE:

*create FLASH_EraseInitTypeDef structure

*fill it (STM32G4xx example for the last page in bank)
	pflash.TypeErase = FLASH_TYPEERASE_PAGES;
	pflash.NbPages = 1;
	pflash.Page = 63; //last page, in STM32G4 you can choose only page number, not address
	//pflash.Banks = empty field, only one bank in MCU

*erase page
+++++BECAUSE YOU WILL GET ERRORS IN FLASH_SR REGISTER IF YOU TRY TO
+++++FLASH NOT 0xFFFF FFFF FFFF FFFF CELLS
	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&pflash, &flash_err);
	HAL_FLASH_Lock();

*save your array using WriteArray function

 */




uint8_t MFLPR_WriteArray(uint32_t addr, uint16_t *arr, uint8_t len);



#endif
