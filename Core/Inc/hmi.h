/*
 * hmi.h
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 *
 *  Human machine interface block
 */

#ifndef INC_HMI_H_
#define INC_HMI_H_

#include "MGL/MGL.h"
#include "app_types.h"
#include "Encoder/M_ENC.h"
#include "rps.h"

/*---------------------------------------------DEFINES------------------------------------------------*/

//VAW settings
/////////////////////////////////////////////////////
#define VAW_MAX_VOLT 2400U
#define VAW_MAX_CURR 5000U
#define VAW_MAX_WATT 1200U

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


//PERIF_TL494
/////////////////////////////////////////////////////
#define PERIF_TL494_ON() HAL_GPIO_WritePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin,0)
#define PERIF_TL494_OFF() HAL_GPIO_WritePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin,1)
#define PERIF_TL494_TOGGLE() HAL_GPIO_TogglePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin)

/*---------------------------------------------STRUCTURES------------------------------------------------*/

extern bar_gr volt_bar, curr_bar, watt_bar; //progress bars init
extern menc_struct_type menc1, menc2; //Encoder library structures

/*---------------------------------------------FUNCTIONS------------------------------------------------*/

void HMI_Input_EncodersStructInit(void);
void HMI_Input(rps_type *r);
void HMI_Display_GraphBarsStructInit(rps_type *r);
void HMI_Display_StartPage(void);
void HMI_Display_MeasPage(rps_type *r);
void HMI_Display_DebugMeasPage(rps_type *r);

#endif /* INC_HMI_H_ */


//fast display functions
/////////////////////////////////////////////////////
//#define REFRESH_MAIN_FIELD() MGL_FillRectWH(MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_WIDTH, MAIN_FIELD_HEIGHT, BG_COLOR)
//#define DRAW_LOW_INF_BAR() MGL_DrawRectWH(LOW_ST_BAR_X, LOW_ST_BAR_Y, LOW_ST_BAR_W, LOW_ST_BAR_H, LOW_INF_BAR_STROKE_COLOR)///<lower information stroke
//#define DRAW_MAIN_FIELD_STROKE() MGL_DrawRectWH(MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_WIDTH, MAIN_FIELD_HEIGHT - 1, MP_STROKE_INT_COLOR)///<main field stroke


