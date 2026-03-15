/*
 * hmi.h
 *
 *  Created on: Mar 8, 2026
 *      Author: aborav
 *
 *  Human machine interface block
 */

#ifndef INC_INPUT_H_
#define INC_INPUT_H_

#include "app_types.h"
#include "ctrl.h"

#include "Encoder/M_ENC.h"

/*---------------------------------------------DEFINES------------------------------------------------*/

//PERIF_TL494
/////////////////////////////////////////////////////
#define PERIF_TL494_ON() HAL_GPIO_WritePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin,0)
#define PERIF_TL494_OFF() HAL_GPIO_WritePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin,1)
#define PERIF_TL494_TOGGLE() HAL_GPIO_TogglePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin)

/*---------------------------------------------STRUCTURES------------------------------------------------*/
extern menc_struct_type menc1, menc2; //Encoder library structures

/*---------------------------------------------FUNCTIONS------------------------------------------------*/
void INPUT_EncStructInit(void);
void INPUT_EncHandler(rps_type *r);

#endif /* INC_INPUT_H_ */

