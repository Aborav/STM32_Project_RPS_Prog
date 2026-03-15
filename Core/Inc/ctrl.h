/*
 * rps.h
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 *  Regulated power supply block
 */

#ifndef INC_CTRL_H_
#define INC_CTRL_H_

/*
 START CONTROL SEQUENCE PROCEDURE
 1) CTRL_SP_ReachByTable -> find closest DAC value to a set point in saved table
 2) Turn ON TL494 clocking
 3) CTRL_SP_WaitUntilStable -> wait until 3 times voltage/current will be the same
 4) CTRL_SP_ReachBySteps -> step by step +/- DAC value to reach a set point
 6) if voltage can reach a set point -> Voltage is a master (CV) (or current)
 7) CTRL_SP_WaitUntilStable -> wait until 3 times voltage/current will be the same
 8) current can't reach a set point but it is stable -> it is a slave now (or voltage)
 9) CTRL_SP_ReachByTable -> write back slave DAC value from saved table
 10) CTRL_SP_ConstHold -> from 4) to 9)
 */

/*
 TO DO:
 1) Current channel control
 2) More steps or algorithm for CTRL_SP_ReachBySteps
 3) Permanent control function to maintain stable voltage/current
 4) Slave channel check in permanent control
*/

/*---------------------------------------------INCLUDES------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "main.h"

#include "app_types.h"
#include "input.h"
#include "serv.h"
#include "display.h"

#include "Encoder/M_ENC.h"
#include "SignalFilter/M_Filter.h"
#include "FlashProm/M_Flashprom.h"
#include "INA226/M_INA226.h"

/*---------------------------------------------DEFINES------------------------------------------------*/
//VAW settings
/////////////////////////////////////////////////////
#define VAW_MAX_VOLT 2400U
#define VAW_MAX_CURR 5000U
#define VAW_MAX_WATT 1200U

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

#define RPS_TIMEOUT_THRESHOLD 10000000U

/*---------------------------------------------TYPES------------------------------------------------*/
extern DAC_HandleTypeDef hdac1;

/*---------------------------------------------FUNCTIONS------------------------------------------------*/
//while function
void CTRL_Handler(rps_type *r);

//conversion to volt,curr,watt
void CTRL_VAW_Conversion(rps_type *r);

//saving functions
void CTRL_SAVE_FBTableVolt(rps_type *r);
void CTRL_SAVE_FBTableCurr(rps_type *r);
void CTRL_SAVE_Table(rps_type *r);
void CTRL_SAVE_TableInit(rps_type *r);
void CTRL_SAVE_PrintSavedTables(void);
void CTRL_SAVE_CalcDACSteps(rps_type *r, rps_channel_type va);

//operating function, reach set point
void CTRL_SP_ReachByTable(rps_type *r, rps_channel_type va);
void CTRL_SP_WaitUntilStable(rps_type *r, rps_channel_type va);
void CTRL_SP_ReachBySteps(rps_type *r, rps_channel_type va);

#endif /* INC_CTRL_H_ */
