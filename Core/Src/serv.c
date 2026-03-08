/*
 * serv.c
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 */
#include "serv.h"

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to erase both tables, one page
 */
void SERV_Flash_EraseTable(rps_type *r) {
	uint32_t flash_err;

	pflash.TypeErase = FLASH_TYPEERASE_PAGES;
	pflash.NbPages = 1;
	pflash.Page = 63;

	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&pflash, &flash_err);
	HAL_FLASH_Lock();

	if (flash_err != 0xffffffff) {
		r->err.bit.flash_erase = 1;
		return;
	}
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to print uint16_t as float
 * @param[in] fake_f -> uint16_t
 * @param[in] num_quant -> number of digits
 * @param[in] num_aft_comma -> digits after comma
 */
void SERV_Print_FakeFloat(uint16_t fake_f, uint8_t num_quant, uint8_t num_aft_comma) {
	char str[8];	//create array for PrintStr, quantity of numbers+"\0"
	char *str_ptr = str; ///<ptr to scroll array
	uint16_t div_buf;	//buffer to hold digit for division
	uint16_t div_mask[] = { 1, 10, 100, 1000, 10000 }; ///<for not to use POW function
	uint8_t clear_flag = 1; ///<stop clear zeros flag

	for (uint8_t i = 4, j = 0; i > 0; i--, j++) {
		div_buf = (fake_f / div_mask[i]); ///<extract one by one digits from input var
		fake_f -= (div_buf) * div_mask[i]; ///<remove pow in num
		if (div_buf > 0)
			clear_flag = 0; ///<wait untill num starts
		if (clear_flag == 0)
			*str_ptr++ = div_buf + '0';
		if (num_aft_comma == i && div_buf == 0) {
			*str_ptr++ = '0';
			*str_ptr++ = '.';
		} else if (num_aft_comma == i)
			*str_ptr++ = '.';
	}
	*str_ptr++ = fake_f + '0';	///<last digit
	*str_ptr++ = '\0'; //add end of string to finish printing in PrintStr function

	printf("%s", str);
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function for safety delay
 * @param[in/out] counter -> make global variable for timer
 * @param[in] delay -> choose delay in ms
 */
bool MillisDelay(uint32_t *counter, uint16_t delay) {
	if (HAL_GetTick() - *counter <= delay) {
		return 0;
	} else {
		*counter = HAL_GetTick();
		return 1;
	}
}
