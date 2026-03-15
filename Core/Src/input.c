/*
 * hmi.c
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 */

#include <input.h>
#include "main.h"

/*---------------------------------------------TYPES------------------------------------------------*/
//Encoder library structures
////////////////////////////////////////////////////////////
menc_struct_type menc1, menc2;

/*---------------------------------------------FUNCTIONS--------------------------------------------*/

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to fill up encoders structures
 */
void INPUT_EncStructInit(void) {
	menc1.s1_pin = ENC1_S1_Pin;
	menc1.s2_pin = ENC1_S2_Pin;
	menc1.key_pin = ENC1_KEY_Pin;
	menc1.s1_port = ENC1_S1_GPIO_Port;
	menc1.s2_port = ENC1_S2_GPIO_Port;
	menc1.key_port = ENC1_KEY_GPIO_Port;
	menc1.key_exti_line = EXTI2_IRQn;

	menc2.s1_pin = ENC2_S1_Pin;
	menc2.s2_pin = ENC2_S2_Pin;
	menc2.key_pin = 0;
	menc2.s1_port = GPIOB;
	menc2.s2_port = GPIOB;
	menc2.key_port = 0;
	menc2.key_exti_line = 0;
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to maintain clicks and turns response
 * @param[in/out] *r -> project structure pointer
 */
void INPUT_EncHandler(rps_type *r) {
	RPS_CHECK_STRUCT_PTR();

	MENC_MainHandler(&menc1);
	MENC_MainHandler(&menc2);

	if (MENC_Click(&menc1)) {
		r->fl.ctrl_start = ~r->fl.ctrl_start; //start/stop SP reach procedure
		r->fl.ctrl_stop = ~r->fl.ctrl_stop; //start/stop SP reach procedure
	}

//voltage trim
	if (MENC_TurnRight(&menc1)) {
		RPS_PLUS_LIMIT_CHECK(r->val.u_sp_val, 10, r->val.u_max);
	}

	if (MENC_TurnLeft(&menc1)) {
		RPS_MINUS_LIMIT_CHECK(r->val.u_sp_val, 10, r->val.u_min);
	}

//current trim
	if (MENC_TurnRight(&menc2)) {
		RPS_PLUS_LIMIT_CHECK(r->val.i_sp_val, 10, r->val.i_max);
	}

	if (MENC_TurnLeft(&menc2)) {
		RPS_MINUS_LIMIT_CHECK(r->val.i_sp_val, 10, r->val.i_min);
	}
}


//if any turn
//	if (MENC_AnyTurn(&menc1)) {
//	}
//	if (MENC_AnyTurn(&menc2)) {
//	}
//		//fast turn
//		if (MENC_TurnFastRight(&menc1)) {
//			dac_volt_value += 20;
//			if (dac_volt_value > 4095) dac_volt_value = 4095;
//			//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_volt_value);
//			sf.volt_draw = 1;
//		}
//
//		if (MENC_TurnFastRight(&menc2)) {
//			dac_curr_value += 20;
//			if (dac_curr_value > 4095) dac_curr_value = 4095;
//			//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac_curr_value);
//			sf.curr_draw = 1;
//
//		}
//		if (MENC_TurnFastLeft(&menc1)) {
//			dac_volt_value -= 20;
//			if (dac_volt_value < 0) dac_volt_value = 0;
//			//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_volt_value);
//			sf.volt_draw = 1;
//		}
//
//		if (MENC_TurnFastLeft(&menc2)) {
//			dac_curr_value -= 20;
//			if (dac_curr_value < 0) dac_curr_value = 0;
//			//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac_curr_value);
//			sf.curr_draw = 1;
//		}

