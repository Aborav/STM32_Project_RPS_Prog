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
void HMI_Input_EncodersStructInit(void) {
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
void HMI_Input(rps_type *r) {
	RPS_CHECK_STRUCT_PTR();

	MENC_MainHandler(&menc1);
	MENC_MainHandler(&menc2);

//turn on/off PERIF_TL494 clocking
	if (MENC_Click(&menc1)) {
		r->fl.tl494_on = ~r->fl.tl494_on;
		if (r->fl.tl494_on) {
			RPS_Ctrl_SPReachTable(r, _VOLT);
			RPS_Ctrl_SPReachTable(r, _CURR);
			PERIF_TL494_ON();
			__NOP();
			RPS_Ctrl_SPReachSteps(r);
		} else {
			PERIF_TL494_OFF();
			PERIF_DAC_SET(r->val.u_dac = 0, DAC_VOLT_CH);
			PERIF_DAC_SET(r->val.i_dac = 0, DAC_CURR_CH);
		}
	}

//voltage trim
	if (MENC_TurnRight(&menc1)) {
		r->val.u_sp_val += 10;
		if (r->val.u_sp_val >= r->val.u_max)
			r->val.u_sp_val = r->val.u_max;
//		r->val.dac_u += 10;
//		if (r->val.dac_u >= 4095)
//			r->val.dac_u = 4095;
	}

	if (MENC_TurnLeft(&menc1)) {
		r->val.u_sp_val -= 10;
		if (r->val.u_sp_val < r->val.u_min)
			r->val.u_sp_val = r->val.u_min;
//		r->val.dac_u -= 10;
//		if (r->val.dac_u < 0)
//			r->val.dac_u = 0;
	}

//current trim
	if (MENC_TurnRight(&menc2)) {
		r->val.i_sp_val += 10;
		if (r->val.i_sp_val >= r->val.i_max)
			r->val.i_sp_val = r->val.i_max;
//		r->val.dac_i += 10;
//		if (r->val.dac_i >= 4095)
//			r->val.dac_i = 4095;
	}

	if (MENC_TurnLeft(&menc2)) {
		r->val.i_sp_val -= 10;
		if (r->val.i_sp_val < r->val.i_min)
			r->val.i_sp_val = r->val.i_min;
//		r->val.dac_i -= 10;
//		if (r->val.dac_i < 0)
//			r->val.dac_i = 0;
	}

//if any turn
	if (MENC_AnyTurn(&menc1)) {
//		if (r->fl.tl494_on) {
//			RPS_Ctrl_SPReachTable(r->val.u_sp_val, r, _VOLT);
//			//PERIF_DAC_SET(r->val.dac_u, DAC_VOLT_CH);
//		}
	}
	if (MENC_AnyTurn(&menc2)) {
//		if (r->fl.tl494_on) {
//			RPS_Ctrl_SPReachTable(r->val.i_sp_val, r, _CURR);
//			//PERIF_DAC_SET(r->val.dac_i, DAC_CURR_CH);
//		}
	}
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
}

