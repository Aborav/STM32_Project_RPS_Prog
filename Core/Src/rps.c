/*
 * rps.c
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 */

#include "rps.h"

/*---------------------------------------------TYPES------------------------------------------------*/
//Feedback tables in flash
////////////////////////////////////////////////////////////
const uint16_t *table_ptr_u = (uint16_t*) TABLE_VOLT_ADDR;
const uint16_t *table_ptr_i = (uint16_t*) TABLE_CURR_ADDR;

uint16_t table_dac_step[RPS_TABLE_SIZE]; ///<DAC from 0 to 4095
//uint16_t table_volt_fb[RPS_TABLE_SIZE]; ///<ralated to DAC step voltage
//uint16_t table_curr_fb[RPS_TABLE_SIZE]; ///<ralated to DAC step current

/*---------------------------------------------FUNCTIONS------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to read INA226 register and convert to uint16 fake float
 * @param[in/out] *r -> project structure pointer
 */
void RPS_VAW_Conversion(rps_type *r) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}

//static uint16_t med_fil_volt_buf[3],med_fil_curr_buf[3];

	r->val.volt = INA_GetBusVoltageTiny();
//FilterMedian(&r->val.volt, med_fil_volt_buf);

	r->val.curr = INA_GetCurrentTiny();
//FilterMedian(&r->val.curr, med_fil_curr_buf);

	r->val.watt = INA_GetPowerTiny();

}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to make DAC to voltage related table\
 * use it only after RPS_Save_TableInit()
 * @param[in/out] *r -> project structure pointer
 */
void RPS_Save_FBTableVolt(rps_type *r) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}

	uint16_t val = 0; ///<buffer
	uint32_t timeout_cnt = 0; ///<against infinite while
	uint64_t *buf64_ptr; ///<HAL_FLASH_Program buffer pointer
	uint16_t buf16[4]; ///<16 bit buffer to assemble 64 bit data then
	uint8_t buf16_cnt = 0; ///<counter to make 64 bit variable from 4 16 bit
	uint16_t addr_cnt = 0; ///<flash address shifter

#ifdef USE_DEBUG
	printf("Starting voltage calibration\n");
#endif

	PERIF_DAC_SET(4095, DAC_CURR_CH); //get current on maximum to eliminate affect from it
	PERIF_TL494_ON();
	HAL_FLASH_Unlock();

//increase DAC value +100 until max and put voltage feedback and appropriate DAC values in the arrays
	for (uint8_t i = 0; i < RPS_TABLE_SIZE; i++) {
		val = *(table_dac_step + i);
		PERIF_DAC_SET(val, DAC_VOLT_CH); //voltage increasing
		HAL_Delay(RPS_TABLE_DELAY); //wait until capacitor is charged
		r->val.u_dac = val;
		RPS_VAW_Conversion(r);
		//*(table_volt_fb + i) = r->val.volt;

#ifdef USE_DEBUG
		printf("Volt:%d   ", r->val.volt);
		printf("DAC: %d\n", val);
#endif
		//save to flash
		*(buf16 + buf16_cnt) = r->val.volt; //write data one by one in 4 elements array 16 bit resolution
		//4*16bit array is full, time to push it into flash
		if (buf16_cnt >= 3 || i == RPS_TABLE_SIZE - 1) {
			buf16_cnt = 0;
			buf64_ptr = (uint64_t*) buf16; //pointer change
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, TABLE_VOLT_ADDR + 8U * addr_cnt, (uint64_t) *buf64_ptr) != 0) {
				r->err.bit.flash_write = 1;
				return;
			}
			addr_cnt++;
		} else {
			buf16_cnt++;
		}
		__NOP();
	}

	HAL_FLASH_Lock();

//turn off everything
	PERIF_TL494_OFF();
	PERIF_DAC_SET(0, DAC_VOLT_CH);
	PERIF_DAC_SET(0, DAC_CURR_CH);
	r->val.u_dac = 0; //clear global structure variable

//wait until capacitor is discharged
	while (r->val.volt != 0) {
		RPS_VAW_Conversion(r);
		HMI_Display_MeasPage(r);
		if (timeout_cnt >= RPS_TIMEOUT_THRESHOLD) {
			r->err.bit.cicle_timeout = 1;
			return;
		}
		timeout_cnt++;
	}

#ifdef USE_DEBUG
	printf("Finish voltage calibration\n");
#endif
}

///////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to make DAC to current related table\
 * use it only after RPS_Save_TableInit()
 * @param[in/out] *r -> project structure pointer
 */
void RPS_Save_FBTableCurr(rps_type *r) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}

	uint16_t val = 0;
	uint32_t timeout_cnt = 0; ///<against infinite while
	uint64_t *buf64_ptr; ///<HAL_FLASH_Program buffer pointer
	uint16_t buf16[4]; ///<16 bit buffer to assemble 64 bit data then
	uint8_t buf16_cnt = 0; ///<counter to make 64 bit variable from 4 16 bit
	uint16_t addr_cnt = 0; ///<flash address shifter

#ifdef USE_DEBUG
	printf("Starting current calibration\n");
#endif

	PERIF_DAC_SET(4095, DAC_VOLT_CH); //get voltage on maximum to eliminate affect from it
	PERIF_TL494_ON();

	HAL_FLASH_Unlock();

//increase DAC value +100 until max and put current feedback and appropriate DAC values in the arrays
	for (uint8_t i = 0; i < RPS_TABLE_SIZE; i++) {
		val = *(table_dac_step + i);
		PERIF_DAC_SET(val, DAC_CURR_CH); //voltage increasing
		HAL_Delay(RPS_TABLE_DELAY); //wait until output capacitor is charged
		r->val.i_dac = val;
		RPS_VAW_Conversion(r);
		//*(table_curr_fb + i) = r->val.curr;

#ifdef USE_DEBUG
		printf("Curr:%d   ", r->val.curr);
		printf("DAC: %d\n", val);
#endif

		//save to flash
		*(buf16 + buf16_cnt) = r->val.curr; //write data one by one in 4 elements array 16 bit resolution
		//4*16bit array is full, time to push it into flash
		if (buf16_cnt >= 3 || i == RPS_TABLE_SIZE - 1) {
			buf16_cnt = 0;
			buf64_ptr = (uint64_t*) buf16; //pointer change
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, TABLE_CURR_ADDR + 8U * addr_cnt, (uint64_t) *buf64_ptr) != 0) {
				r->err.bit.flash_write = 1;
				return;
			}
			addr_cnt++;
		} else {
			buf16_cnt++;
		}
	}

	HAL_FLASH_Lock();

//turn off everything
	PERIF_TL494_OFF();
	PERIF_DAC_SET(0, DAC_VOLT_CH);
	PERIF_DAC_SET(0, DAC_CURR_CH);
	r->val.i_dac = 0; //clear global structure variable

	while (r->val.volt != 0) {
		RPS_VAW_Conversion(r);
		HMI_Display_MeasPage(r);
		if (timeout_cnt >= RPS_TIMEOUT_THRESHOLD) {
			r->err.bit.cicle_timeout = 1;
			return;
		}
		timeout_cnt++;
	}

#ifdef USE_DEBUG
	printf("Finish current calibration\n");
#endif
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to save both tables\
 * there is no possibility to save only voltage or current
 * before each saving page erasing required
 * @param[in/out] *r -> project structure pointer
 */
void RPS_Save_Table(rps_type *r) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}
	SERV_Flash_EraseTable(r); //clear last page in FLASH

//filling tables
	RPS_Save_FBTableVolt(r);
	volt_bar.num_max = r->val.u_max; //renew maximum value after calibration

	RPS_Save_FBTableCurr(r);
	curr_bar.num_max = r->val.i_max; //renew maximum value after calibration
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to fill table_dac_step array
 * max and min values of current and voltage
 * saved previously in FLASH
 * it's required for voltage/current control functions
 */
void RPS_Save_TableInit(rps_type *r) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}
	uint16_t val;
	uint16_t buf_curr = 0; //buffer
	uint16_t debug_buf;

	for (uint16_t i = 0; i < RPS_TABLE_SIZE; i++) {
		val = i * 100;
		if (val > 4095)
			val = 4095;
		*(table_dac_step + i) = val;
	}
//min and max voltage
	r->val.u_dac = 0;
	r->val.u_min = *(table_ptr_u + 0);
	r->val.u_max = *(table_ptr_u + RPS_TABLE_SIZE - 1);

//min and max voltage
	r->val.i_min = *(table_ptr_i + 0);

//just maximum current in the table
	for (uint8_t i = 0; i < RPS_TABLE_SIZE; i++) {
		debug_buf = *(table_ptr_i + i);
		if (buf_curr > debug_buf) {
			r->val.i_max = buf_curr;
			break;
		}
		buf_curr = *(table_ptr_i + i);
		__NOP();

//		//compare voltage from the feedback voltage table and a set point.
//		//to find a table pointer with closest to SP voltage but smaller than it
//		if (*(table_ptr_i + i) > set_point) {
//			r->val.dac_i = *(table_dac_step + i - 1);
//			//here must be some protection from pointer miss read. But my first value in the tables are 0
//			//nothing is smaller than 0 in uint16_t
//			break;
//		}

	}
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to print saved in Flash tables
 */
void RPS_Save_PrintSavedTables(void) {
	printf("Print saved in flash tables");
	for (uint16_t i = 0; i < RPS_TABLE_SIZE; i++) {
		printf("DAC value: %u", *(table_dac_step + i));
		printf("Saved voltage: %u", *(table_ptr_u + i));
		printf("Saved current: %u \n", *(table_ptr_i + i));
	}
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to calculate DAC steps using tables in flash
 * DAC +10 -> voltage +?\
 * DAC +100 -> current +?
 * function returns val.u_dac_step100/10 value in the global structure
 * @param[in/out] *r -> structure pointer
 * @param[in] va -> voltage/current channel
 */
void RPS_Save_CalculateDACSteps(rps_type *r, rps_channel_type va) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}

	uint16_t *table_ptr; ///<flash table pointer
	filter_type fil_arr[3] = { 0, }; ///<median filter buffer
	filter_type fil_val; ///<median filter variable
	uint16_t buf = 0; ///<previous value holder
	uint16_t max_val; ///<<max value in the global structure

	if (va == _VOLT) {
		table_ptr = (uint16_t*) table_ptr_u;
		max_val = r->val.u_max;
	} else if (va == _CURR) {
		table_ptr = (uint16_t*) table_ptr_i;
		max_val = r->val.i_max;
	} else {
		r->err.bit.wrong_channel = 1;
		return;
	}

	//i have tried to use just summ average, but it eats about 1 kB, this is 400 B
	for (uint16_t i = 0; i < RPS_TABLE_SIZE; i++) {
		//if this is maximum value -> exit
		if (table_ptr[i] == max_val) {
			break;
		} else {
			if (table_ptr[i] - buf > 0) {
				fil_val = table_ptr[i] - buf; //check different between prev and current value
				FilterMedian(&fil_val, fil_arr); //filter it
				buf = table_ptr[i]; //previous = current value
			}
		}
	}

	if (va == _VOLT) {
		r->val.u_dac_step100 = (uint16_t) fil_val; //calculate average step of DAC
		r->val.u_dac_step10 = (uint16_t) fil_val / 10; //calculate average step of DAC
	} else if (va == _CURR) {
		r->val.i_dac_step100 = (uint16_t) fil_val; //calculate average step of DAC
		r->val.i_dac_step10 = (uint16_t) fil_val / 10; //calculate average step of DAC
	}
}
/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to reach voltage by setting a set point
 * search for the closest DAC set point
 * function returns val.dac_u/i value in the global structure
 * @param[in] sp_val-> set point
 * @param[in/out] *r -> structure pointer
 */
void RPS_Ctrl_SPReachTable(rps_type *r, rps_channel_type va) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}

	uint16_t dac_val; ///< DAC value, will be written in global structure
	uint16_t max_val; ///< max value in the global structure
	uint16_t *table_ptr; ///<flash table pointer
	uint16_t set_point; ///<set point for this function

	if (va == _VOLT) {
		table_ptr = (uint16_t*) table_ptr_u;
		max_val = r->val.u_max;
		set_point = r->val.u_sp_val;
	} else if (va == _CURR) {
		table_ptr = (uint16_t*) table_ptr_i;
		max_val = r->val.i_max;
		set_point = r->val.i_sp_val;
	} else {
		r->err.bit.wrong_channel = 1;
		return;
	}

	if (set_point == 0) {
		dac_val = 0; //mix
	} else if (set_point == max_val) {
		dac_val = 4095; //max
	} else {
		for (uint8_t i = 0; i < RPS_TABLE_SIZE; i++) {
			//compare voltage/current from the feedback voltage table and a set point.
			//to find a table pointer with closest to SP value but smaller than it
			if (*(table_ptr + i) > set_point) {
				dac_val = *(table_dac_step + i - 1); //use this shift in the DAC values table
				break;
			}
		}
	}
	__NOP();
	if (va == _VOLT) {
		r->val.u_dac = dac_val;
		PERIF_DAC_SET(dac_val, DAC_VOLT_CH);
	} else if (va == _CURR) {
		r->val.i_dac = dac_val;
		PERIF_DAC_SET(dac_val, DAC_CURR_CH);
	}
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to fine adjust after RPS_Ctrl_SPReachTable
 * Affects on both channels
 * @param[in/out] *r -> structure pointer
 */
bool RPS_Ctrl_SPReachSteps(rps_type *r) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return 1;
	}

	int16_t calc_diff; ///<difference for calculation in cycle
	uint32_t timeout_cnt = 0;
	bool rev_flag = 0; ///<reverse flag
	int16_t dac_val = r->val.u_dac;
	uint16_t dac_diff;
	uint16_t att_buf[3]; ///<buffer to hold previous values of voltage/current
	uint8_t ind = 0; ///<attempts buffer index

	/*--------VOLTAGE PART----------*/

	HAL_Delay(1000); //wait until capacitor is charged
	RPS_VAW_Conversion(r);

	while (1) {
		calc_diff = r->val.u_sp_val - r->val.volt; //what is difference now
		if (calc_diff < 0) {
			calc_diff = -calc_diff;
			rev_flag = 1;
		}

		//adjust step calculation
		if (calc_diff > r->val.u_dac_step100) {
			dac_diff = 100;
		} else if (r->val.u_dac_step100 > calc_diff && calc_diff > r->val.u_dac_step10) {
			dac_diff = 10;
		} else if (calc_diff < r->val.u_dac_step10) {
			dac_diff = 1;
		} else if (calc_diff == 1) {
			//r->val.cv_cc_ctrl = _VOLT;
			return 0;
		}

		//direction of adjust and limits check
		if (rev_flag == 0) {
			dac_val += dac_diff;
			if (dac_val > 4095)
				dac_val = 4095;
		} else if (rev_flag == 1) {
			dac_val -= dac_diff;
			if (dac_val < 0)
				dac_val = 0;
		}

		PERIF_DAC_SET(dac_val, DAC_VOLT_CH);
		r->val.u_dac = dac_val;
		HAL_Delay(RPS_TABLE_DELAY); //wait until capacitor is charged
		RPS_VAW_Conversion(r);
		HMI_Display_MeasPage(r);

		att_buf[ind > 3 ? ind = 0 : ind++] = r->val.volt; //voltage doesn't change 3 times
		if (att_buf[0] == att_buf[1]) {
			if (att_buf[1] == att_buf[2]) {
				if (att_buf[0] == att_buf[2]) {
					return 1;
				}
			}

		}

		if (timeout_cnt++ >= RPS_TIMEOUT_THRESHOLD) {
			r->err.bit.cicle_timeout = 1;
			return 1;
		}

		//__NOP();
	}

}

