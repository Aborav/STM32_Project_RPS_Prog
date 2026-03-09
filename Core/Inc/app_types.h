/*
 * app_types.h
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 */

#ifndef INC_APP_TYPES_H_
#define INC_APP_TYPES_H_

//Values structure
////////////////////////////////////////////////////////////
typedef struct _values_type {
	uint16_t volt; ///<value from measuring source (INA226)
	uint16_t curr; ///<value from measuring source (INA226)
	uint16_t watt; ///<value from measuring source (INA226)
	int16_t dac_u; ///<will be written into DAC register
	int16_t dac_i; ///<will be written into DAC register
	int16_t sp_u_val; ///<set point
	int16_t sp_i_val; ///<set point
	uint16_t u_min; ///<minimum voltage after calibration
	uint16_t u_max; ///<maximum voltage after calibration
	uint16_t i_min; ///<minimum current after calibration
	uint16_t i_max; ///<maximum current after calibration
} values_type;

//Bits field for status flags
////////////////////////////////////////////////////////////
typedef struct _flags_type {
	unsigned tl494_on :1; ///<TL494 clocking is ON
	unsigned start_draw:1; ///<draw object at start
} flags_type;

//Bits field for errors
////////////////////////////////////////////////////////////
typedef union _errors_type {
	uint8_t all_errors;
	struct {
		unsigned cicle_timeout :1; ///<
		unsigned ina226_off :1;
		unsigned flash_erase :1;
		unsigned flash_write :1;
		unsigned reserved :4;
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
typedef enum _channel_type {
	_VOLT = 0, _CURR = 1
} channel_type;





#endif /* INC_APP_TYPES_H_ */
