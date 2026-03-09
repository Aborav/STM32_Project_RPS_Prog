/*
 * serv.h
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 *
 *  Service block
 */

#ifndef INC_SERV_H_
#define INC_SERV_H_

#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "app_types.h"

/*---------------------------------------------TYPES------------------------------------------------*/
//Flash erase structure
////////////////////////////////////////////////////////////
extern FLASH_EraseInitTypeDef pflash;

/*---------------------------------------------FUNCTIONS------------------------------------------------*/

bool MillisDelay(uint32_t *counter, uint16_t delay);
void SERV_Flash_EraseTable(rps_type *r);
void SERV_Print_FakeFloat(uint16_t fake_f, uint8_t num_quant, uint8_t num_aft_comma);
int _write(int file, uint8_t *ptr, int len);

#endif /* INC_SERV_H_ */
