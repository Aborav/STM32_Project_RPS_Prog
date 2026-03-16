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

/*---------------------------------------------MACROS------------------------------------------------*/

//cnt -> counter to increment, act -> action to implement (break,return,return 1)
#define SERV_CHECK_TIMEOUT(cnt,act) do{\
		if (cnt++ >= RPS_TIMEOUT_THRESHOLD) {\
		r->err.bit.cicle_timeout = 1;\
		act;\
		}\
	}while(0);

#define SERV_RESET_ATT_BUF(buf) do{\
		buf[0] = 0xF;\
		buf[1] = 0xFF;\
		buf[2] = 0xFFF;\
	} while(0);

//Increase or decrease DAC value and check limits
#define SERV_PLUS_LIMIT_CHECK(val,inc,max) do{\
		val+=(inc);\
		if (val>(max))\
			val = (max);\
	} while(0)

#define SERV_MINUS_LIMIT_CHECK(val,dec,max) do{\
		val -= (dec);\
		if (val < (max))\
			val = (max);\
	} while(0)

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
