/*
 * hmi.c
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 */

#include "hmi.h"
#include "main.h"

/*---------------------------------------------TYPES------------------------------------------------*/

//Progress bars struct init
mgl_bar_gr_type volt_bar, curr_bar, watt_bar; //progress bars init
//Encoder library structures
////////////////////////////////////////////////////////////
menc_struct_type menc1, menc2;

/*---------------------------------------------FUNCTIONS------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to fill up graph bars for display
 * call this function after RPS_Save_TableInit
 * to change max values
 * @param[in/out] *r -> project structure pointer
 */
void HMI_Display_GraphBarsStructInit(rps_type *r) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}

	volt_bar.num = &r->val.volt;
	volt_bar.num_max = r->val.u_max;
	volt_bar.x = BAR_VOLTAGE_X;
	volt_bar.y = BAR_VOLTAGE_Y;
	volt_bar.width = BAR_WIDTH;
	volt_bar.length = BAR_LENGTH;
	volt_bar.color = VOLT_COLOR;
	volt_bar.bg_color = BG_COLOR;
	volt_bar.stroke_color = BAR_STROKE_COLOR;

	curr_bar.num = &r->val.curr;
	curr_bar.num_max = r->val.i_max;
	curr_bar.x = BAR_CURRENT_X;
	curr_bar.y = BAR_CURRENT_Y;
	curr_bar.width = BAR_WIDTH;
	curr_bar.length = BAR_LENGTH;
	curr_bar.color = CURR_COLOR;
	curr_bar.bg_color = BG_COLOR;
	curr_bar.stroke_color = BAR_STROKE_COLOR;

	watt_bar.num = &r->val.watt;
	watt_bar.num_max = r->val.u_max * r->val.i_max / 10000;
	watt_bar.x = BAR_WATTAGE_X;
	watt_bar.y = BAR_WATTAGE_Y;
	watt_bar.width = BAR_WIDTH;
	watt_bar.length = BAR_LENGTH;
	watt_bar.color = WATT_COLOR;
	watt_bar.bg_color = BG_COLOR;
	watt_bar.stroke_color = BAR_STROKE_COLOR;
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief starting page drawing function
 */
void HMI_Display_StartPage(rps_type *r) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}

	MGL_SET_BG_CLR(COLOR_BLACK); //background for text
	MGL_SET_CLR(0xffff);
	MGL_SET_FONT(FONT_5x8_FP);

	MGL_FILL_SCREEN(COLOR_BLACK);

	MGL_SET_FONT(FONT_17x24_FP);

//not redrawing stack
	MGL_SET_CLR(VOLT_COLOR);
	MGL_SetCursor(108, VAW_VOLTAGE_Y, &mgl_t);
	MGL_PrintStr("U\0", &mgl_t);
	MGL_FILL_RECT_WH(VAW_VOLTAGE_X+FONT_17x24_WIDTH*2+4, VAW_VOLTAGE_Y+FONT_17x24_HEIGHT-2, 2, 2, VOLT_COLOR);

	MGL_SET_CLR(CURR_COLOR);
	MGL_SetCursor(108, VAW_CURRENT_Y, &mgl_t);
	MGL_PrintStr("A\0", &mgl_t);
	MGL_FILL_RECT_WH(VAW_CURRENT_X+FONT_17x24_WIDTH*1+4, VAW_CURRENT_Y+FONT_17x24_HEIGHT-2, 2, 2, CURR_COLOR);

	MGL_SET_CLR(WATT_COLOR);
	MGL_SetCursor(108, VAW_WATTAGE_Y, &mgl_t);
	MGL_PrintStr("W\0", &mgl_t);
	MGL_FILL_RECT_WH(VAW_WATTAGE_X+FONT_17x24_WIDTH*3+4, VAW_WATTAGE_Y+FONT_17x24_HEIGHT-2, 2, 2, WATT_COLOR);

	MGL_SET_CLR(FONT_COLOR);

	MGL_SET_FONT(FONT_5x8_FP);
	MGL_SetCursor(5, LOW_INF_BAR_UPP_Y, &mgl_t);
	MGL_PrintStr("DAC_U:\0", &mgl_t);
	MGL_SetCursor(5, LOW_INF_BAR_LOW_Y, &mgl_t);
	MGL_PrintStr("DAC_I:\0", &mgl_t);

	MGL_SetCursor(80, LOW_INF_BAR_UPP_Y, &mgl_t);
	MGL_PrintStr("SP_U:\0", &mgl_t);
	MGL_SetCursor(80, LOW_INF_BAR_LOW_Y, &mgl_t);
	MGL_PrintStr("SP_I:\0", &mgl_t);

	r->fl.start_draw = 1;
}




/////////////////////////////////////////////////////////////////////////
/*
 * @brief draw redrawing stack of the main page
 * @param[in/out] *r -> project structure pointer
 */
void HMI_Display_MeasPage(rps_type *r) {
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}

	static uint16_t volt_old, curr_old, watt_old;
	static uint16_t sp_u_old, sp_i_old, dac_u_old, dac_i_old;
	int16_t diff = 0; ///<differance between new and old values

	MGL_SET_FONT(FONT_17x24_FP);
//voltage
	diff = volt_old - r->val.volt;
	if (diff < 0)
		diff *= -1; //no matter is old value bigger or smaller than a new one
	if (diff > r->fl.tl494_on ? 1 : 0 || r->fl.start_draw) { //eliminate fluctuation of the value
		MGL_SET_CLR(VOLT_COLOR);
		MGL_SetCursor(VAW_VOLTAGE_X, VAW_VOLTAGE_Y, &mgl_t);
		MGL_PrintFloatTiny_R(r->val.volt, 4, 2, &mgl_t);
		MGL_DrawBar(&volt_bar);
	}

//current
	diff = curr_old - r->val.curr;
	if (diff < 0)
		diff *= -1;
	if (diff > r->fl.tl494_on ? 1 : 0 || r->fl.start_draw) {
		MGL_SET_CLR(CURR_COLOR);
		MGL_SetCursor(VAW_CURRENT_X, VAW_CURRENT_Y, &mgl_t);
		MGL_PrintFloatTiny_R(r->val.curr, 4, 3, &mgl_t);
		MGL_DrawBar(&curr_bar);
	}

//wattage
	diff = watt_old - r->val.watt;
	if (diff < 0)
		diff *= -1;
	if (diff > r->fl.tl494_on ? 1 : 0 || r->fl.start_draw) {
		MGL_SET_CLR(WATT_COLOR);
		MGL_SetCursor(VAW_WATTAGE_X, VAW_WATTAGE_Y, &mgl_t);
		MGL_PrintFloatTiny_R(r->val.watt, 4, 1, &mgl_t);
		MGL_DrawBar(&watt_bar);
	}

	MGL_SET_CLR(FONT_COLOR);
	MGL_SET_FONT(FONT_5x8_FP);

//voltage DAC value
	if(dac_u_old - r->val.u_dac != 0 || r->fl.start_draw){
		MGL_SetCursor(45, LOW_INF_BAR_UPP_Y, &mgl_t);
		MGL_PrintUint16_R(r->val.u_dac, 4, &mgl_t);
	}

//current dac value
	if(dac_i_old - r->val.i_dac != 0 || r->fl.start_draw){
		MGL_SetCursor(45, LOW_INF_BAR_LOW_Y, &mgl_t);
		MGL_PrintUint16_R(r->val.i_dac, 4, &mgl_t);
	}

//voltage set point
	if(sp_u_old - r->val.u_sp_val != 0 || r->fl.start_draw) {
		MGL_SetCursor(120, LOW_INF_BAR_UPP_Y, &mgl_t);
		MGL_PrintFloatTiny_R(r->val.u_sp_val, 4, 2, &mgl_t);
	}

//current set point
	if(sp_i_old - r->val.i_sp_val != 0 || r->fl.start_draw) {
		MGL_SetCursor(120, LOW_INF_BAR_LOW_Y, &mgl_t);
		MGL_PrintFloatTiny_R(r->val.i_sp_val, 4, 3, &mgl_t);
	}

	volt_old = r->val.volt;
	curr_old = r->val.curr;
	watt_old = r->val.watt;
	sp_u_old = r->val.u_sp_val;
	sp_i_old = r->val.i_sp_val;
	dac_u_old = r->val.u_dac;
	dac_i_old = r->val.i_dac;

	r->fl.start_draw = 0;
}

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
	if (r == 0) {
		r->err.bit.empty_ptr = 1;
		return;
	}

	MENC_MainHandler(&menc1);
	MENC_MainHandler(&menc2);

//turn on/off PERIF_TL494 clocking
	if (MENC_Click(&menc1)) {
		r->fl.tl494_on = ~r->fl.tl494_on;
		if (r->fl.tl494_on) {
			RPS_Ctrl_SPReachTable(r, _VOLT);
			RPS_Ctrl_SPReachTable(r, _CURR);
			PERIF_TL494_ON();
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

