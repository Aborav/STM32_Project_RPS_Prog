/*
 * rps.h
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 *  Regulated power supply block
 */

#ifndef INC_RPS_H_
#define INC_RPS_H_

#include <stdio.h>
#include "main.h"
#include "app_types.h"
#include "Encoder/M_ENC.h"
#include "SignalFilter/M_Filter.h"
#include "FlashProm/M_Flashprom.h"
#include "INA226/M_INA226.h"
#include "hmi.h"
#include "serv.h"


/*---------------------------------------------DEFINES------------------------------------------------*/
//DAC fast function
/////////////////////////////////////////////////////
#define DAC_VOLT_CH DAC_CHANNEL_1
#define DAC_CURR_CH DAC_CHANNEL_2
#define PERIF_DAC_SET(val,ch) HAL_DAC_SetValue(&hdac1, (ch), DAC_ALIGN_12B_R, (val));

//Write to flash function defines
/////////////////////////////////////////////////////
#define TABLE_VOLT_ADDR (0x0801F800) //voltage to DAC table address in flash
#define TABLE_CURR_ADDR (TABLE_VOLT_ADDR+1024) //current to DAC table address in flash

//RPS configuration
/////////////////////////////////////////////////////
#define RPS_TABLE_DELAY 100U //this is delay between DAC step when the FB table is filling up (min 70ms)
#define RPS_TABLE_DAC_STEP 100U
#define RPS_TABLE_SIZE 42U

#define RPS_TIMEOUT_THRESHOLD 100000000U

/*---------------------------------------------TYPES------------------------------------------------*/
extern DAC_HandleTypeDef hdac1;

/*---------------------------------------------FUNCTIONS------------------------------------------------*/

void RPS_VAW_Conversion(rps_type *r);
void RPS_Save_FBTableVolt(rps_type *r);
void RPS_Save_FBTableCurr(rps_type *r);
void RPS_Save_Table(rps_type *r);
void RPS_Save_TableInit(rps_type *r);
void RPS_Save_PrintSavedTables(void);
void RPS_Save_CalculateDACSteps(rps_type *r, rps_channel_type va);
void RPS_Ctrl_SPReachTable(uint16_t set_point, rps_type *r, rps_channel_type va);
bool RPS_Ctrl_SPReachSteps(rps_type *r);


#endif /* INC_RPS_H_ */
