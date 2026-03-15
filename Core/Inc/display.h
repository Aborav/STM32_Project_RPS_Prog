/*
 * display.h
 *
 *  Created on: Mar 15, 2026
 *      Author: aborav
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_


#include "MGL/MGL.h"
#include "app_types.h"

/*---------------------------------------------DEFINES------------------------------------------------*/
//Graphic settings
/////////////////////////////////////////////////////
//SCREEN RESOLUTION : 160x128

/////////////////////////////////////////////////////
#define SCREEN_REFRESH_RATE 30U ///<ms

//Main field rectengle
/////////////////////////////////////////////////////
#define MAIN_FIELD_X 3U
#define MAIN_FIELD_Y 3U
#define MAIN_FIELD_WIDTH 154U
#define MAIN_FIELD_HEIGHT 122U

//Working field dimensions
/////////////////////////////////////////////////////
#define MEAS_FIELD_X 5U
#define MEAS_FIELD_Y 5U
#define MEAS_FIELD_WIDTH 150U
#define MEAS_FIELD_HEIGHT 118U

//Measurements location on the display
/////////////////////////////////////////////////////
#define VAW_VOLTAGE_X MEAS_FIELD_X+4U
#define VAW_VOLTAGE_Y MEAS_FIELD_Y
#define VAW_CURRENT_X MEAS_FIELD_X+4U
#define VAW_CURRENT_Y MEAS_FIELD_Y+30U
#define VAW_WATTAGE_X MEAS_FIELD_X+4U
#define VAW_WATTAGE_Y MEAS_FIELD_Y+59U

//Colors
/////////////////////////////////////////////////////
#define VOLT_COLOR COLOR_LIGHT_SEA_GREEN
#define CURR_COLOR 0xFBEF
#define WATT_COLOR COLOR_LIGHT_SALMON
#define BG_COLOR COLOR_BLACK
#define FONT_COLOR COLOR_LIGHT_GRAY
#define LOW_INF_BAR_STROKE_COLOR COLOR_DARK_GRAY
#define MP_STROKE_COLOR COLOR_RED
#define MP_STROKE_INT_COLOR COLOR_MAROON
#define BAR_STROKE_COLOR COLOR_DARK_GRAY

//lower information bar rect
/////////////////////////////////////////////////////
#define LOW_ST_BAR_X 7U
#define LOW_ST_BAR_Y 100U
#define LOW_ST_BAR_W 146U
#define LOW_ST_BAR_H 21U

#define LOW_INF_BAR_UPP_Y 102U  ///<upper string X
#define LOW_INF_BAR_LOW_Y 112U  ///<lower string X

//Progress bar
/////////////////////////////////////////////////////
#define BAR_VOLTAGE_X MEAS_FIELD_X+128U
#define BAR_VOLTAGE_Y MEAS_FIELD_Y
#define BAR_CURRENT_X MEAS_FIELD_X+135U
#define BAR_CURRENT_Y MEAS_FIELD_Y
#define BAR_WATTAGE_X MEAS_FIELD_X+142U
#define BAR_WATTAGE_Y MEAS_FIELD_Y
#define BAR_WIDTH 6U
#define BAR_LENGTH 75U

/*---------------------------------------------TYPES------------------------------------------------*/
extern mgl_bar_gr_type volt_bar, curr_bar, watt_bar; //progress bars init

/*---------------------------------------------FUNCTIONS------------------------------------------------*/
void DISP_GraphBarsStructInit(rps_type *r);
void DISP_StartPage(rps_type *r);
void DISP_MeasPage(rps_type *r);

#endif /* INC_DISPLAY_H_ */

//fast display functions
/////////////////////////////////////////////////////
//#define REFRESH_MAIN_FIELD() MGL_FillRectWH(MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_WIDTH, MAIN_FIELD_HEIGHT, BG_COLOR)
//#define DRAW_LOW_INF_BAR() MGL_DrawRectWH(LOW_ST_BAR_X, LOW_ST_BAR_Y, LOW_ST_BAR_W, LOW_ST_BAR_H, LOW_INF_BAR_STROKE_COLOR)///<lower information stroke
//#define DRAW_MAIN_FIELD_STROKE() MGL_DrawRectWH(MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_WIDTH, MAIN_FIELD_HEIGHT - 1, MP_STROKE_INT_COLOR)///<main field stroke

