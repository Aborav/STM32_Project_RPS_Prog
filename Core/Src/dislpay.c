/*
 * dislpay.c
 *
 *  Created on: Mar 15, 2026
 *      Author: aborav
 */
#include <display.h>
#include "main.h"

/*---------------------------------------------TYPES------------------------------------------------*/

//Progress bars structures initialization
mgl_bar_gr_type volt_bar, curr_bar, watt_bar; //progress bars init


/*---------------------------------------------FUNCTIONS------------------------------------------------*/
/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to fill up graph bars for display
 * call this function after RPS_Save_TableInit
 * to change max values
 * @param[in/out] *r -> project structure pointer
 */
void HMI_Display_GraphBarsStructInit(rps_type *r) {
	RPS_CHECK_STRUCT_PTR();

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
	RPS_CHECK_STRUCT_PTR();

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
	RPS_CHECK_STRUCT_PTR();

	static uint16_t volt_old, curr_old, watt_old;
	static uint16_t sp_u_old, sp_i_old, dac_u_old, dac_i_old;
	//int16_t diff = 0; ///<differance between new and old values

	MGL_SET_FONT(FONT_17x24_FP);
//voltage
//	diff = volt_old - r->val.volt;
//	if (diff < 0)
//		diff *= -1; //no matter is old value bigger or smaller than a new one
//	if (diff > r->fl.tl494_on ? 1 : 0 || r->fl.start_draw) { //eliminate fluctuation of the value
	if (volt_old - r->val.volt != 0 || r->fl.start_draw) {
		MGL_SET_CLR(VOLT_COLOR);
		MGL_SetCursor(VAW_VOLTAGE_X, VAW_VOLTAGE_Y, &mgl_t);
		MGL_PrintFloatTiny_R(r->val.volt, 4, 2, &mgl_t);
		MGL_DrawBar(&volt_bar);
	}

//current
//	diff = curr_old - r->val.curr;
//	if (diff < 0)
//		diff *= -1;
//	if (diff > r->fl.tl494_on ? 1 : 0 || r->fl.start_draw) {
	if (curr_old - r->val.curr != 0 || r->fl.start_draw) {
		MGL_SET_CLR(CURR_COLOR);
		MGL_SetCursor(VAW_CURRENT_X, VAW_CURRENT_Y, &mgl_t);
		MGL_PrintFloatTiny_R(r->val.curr, 4, 3, &mgl_t);
		MGL_DrawBar(&curr_bar);
	}

//wattage
//	diff = watt_old - r->val.watt;
//	if (diff < 0)
//		diff *= -1;
//	if (diff > r->fl.tl494_on ? 1 : 0 || r->fl.start_draw) {
	if (watt_old - r->val.watt != 0 || r->fl.start_draw) {
		MGL_SET_CLR(WATT_COLOR);
		MGL_SetCursor(VAW_WATTAGE_X, VAW_WATTAGE_Y, &mgl_t);
		MGL_PrintFloatTiny_R(r->val.watt, 4, 1, &mgl_t);
		MGL_DrawBar(&watt_bar);
	}

	MGL_SET_CLR(FONT_COLOR);
	MGL_SET_FONT(FONT_5x8_FP);

//voltage DAC value
	if (dac_u_old - r->val.u_dac != 0 || r->fl.start_draw) {
		MGL_SetCursor(45, LOW_INF_BAR_UPP_Y, &mgl_t);
		MGL_PrintUint16_R(r->val.u_dac, 4, &mgl_t);
	}

//current dac value
	if (dac_i_old - r->val.i_dac != 0 || r->fl.start_draw) {
		MGL_SetCursor(45, LOW_INF_BAR_LOW_Y, &mgl_t);
		MGL_PrintUint16_R(r->val.i_dac, 4, &mgl_t);
	}

//voltage set point
	if (sp_u_old - r->val.u_sp_val != 0 || r->fl.start_draw) {
		MGL_SetCursor(120, LOW_INF_BAR_UPP_Y, &mgl_t);
		MGL_PrintFloatTiny_R(r->val.u_sp_val, 4, 2, &mgl_t);
	}

//current set point
	if (sp_i_old - r->val.i_sp_val != 0 || r->fl.start_draw) {
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
