/*
 * app_types.h
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 */

#ifndef INC_APP_TYPES_H_
#define INC_APP_TYPES_H_

#include "main.h"

/*---------------------------------------------DEFINES------------------------------------------------*/
//Fast functions
/////////////////////////////////////////////////////
//empty pointer check
#define RPS_CHECK_STRUCT_PTR() do{\
		if (r == 0) {\
		r->err.bit.empty_ptr = 1;\
		return;\
		}\
	} while(0);

//timeout check
//cnt -> counter to increment, act -> action to implement (break,return,return 1)
#define RPS_CHECK_TIMEOUT(cnt,act) do{\
		if (cnt++ >= RPS_TIMEOUT_THRESHOLD) {\
		r->err.bit.cicle_timeout = 1;\
		act;\
		}\
	}while(0);


/*---------------------------------------------TYPES------------------------------------------------*/

//Values structure
////////////////////////////////////////////////////////////
typedef struct _values_type {
	//VAW
	uint16_t volt; ///<value from measuring source (INA226)
	uint16_t curr; ///<value from measuring source (INA226)
	uint16_t watt; ///<value from measuring source (INA226)

	//Control
	int16_t u_sp_val; ///<set point
	int16_t i_sp_val; ///<set point
	uint16_t u_min; ///<minimum voltage after calibration
	uint16_t u_max; ///<maximum voltage after calibration
	uint16_t i_min; ///<minimum current after calibration
	uint16_t i_max; ///<maximum current after calibration
	uint16_t u_att_buf[3]; ///<buffer to hold 3 last voltage values
	uint16_t i_att_buf[3]; ///<buffer to hold 3 last current values
	uint8_t u_att_buf_ind; ///<index for attempts buffer for voltage
	uint8_t i_att_buf_ind; ///<index for attempts buffer for current

	//DAC
	int16_t u_dac; ///<will be written into DAC register
	int16_t i_dac; ///<will be written into DAC register
	uint8_t u_dac_step100;
	uint8_t u_dac_step10;
	uint8_t u_dac_step5;
	uint8_t i_dac_step100;
	uint8_t i_dac_step10;
	uint8_t i_dac_step5;
} values_type;

//Bits field for status flags
////////////////////////////////////////////////////////////
typedef struct _flags_type {
	unsigned tl494_on :1; ///<TL494 clocking is ON
	unsigned start_draw :1; ///<draw object at start
	unsigned volt_stable:1; ///<voltage is stable after performance
	unsigned curr_stable:1; ///<current is stable after performance
} flags_type;

//Bits field for errors
////////////////////////////////////////////////////////////
typedef union _errors_type {
	uint8_t all_errors;
	struct {
		unsigned cicle_timeout :1; ///<we are stuck in a cycle
		unsigned ina226_off :1; ///<INA226 doesn't respond at start
		unsigned flash_erase :1; ///<flash erase error
		unsigned flash_write :1; ///<flash write error
		unsigned wrong_channel:1; ///<wrong function input channel _CURR,_VOLT
		unsigned empty_ptr:1; ///<empty pointer as input
		unsigned reserved :3;
	} bit;
} errors_type;

//COMMON STRUCTURE
////////////////////////////////////////////////////////////
typedef struct _rps_type {
	values_type val;
	flags_type fl;
	errors_type err;
} rps_type;

//Unions for functions
////////////////////////////////////////////////////////////
typedef enum rps_channel_type {
	_VOLT = 0, _CURR = 1
} rps_channel_type;

#endif /* INC_APP_TYPES_H_ */
