/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
//DEBUG
/////////////////////////////////////////////////////
//#define USE_DEBUG
#define DEBUG_REFRESH 1000U
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ENC1_KEY_Pin GPIO_PIN_2
#define ENC1_KEY_GPIO_Port GPIOA
#define ENC1_KEY_EXTI_IRQn EXTI2_IRQn
#define ENC2_S1_Pin GPIO_PIN_0
#define ENC2_S1_GPIO_Port GPIOB
#define ENC2_S1_EXTI_IRQn EXTI0_IRQn
#define ENC1_S1_Pin GPIO_PIN_1
#define ENC1_S1_GPIO_Port GPIOB
#define ENC1_S1_EXTI_IRQn EXTI1_IRQn
#define ST7735_DC_Pin GPIO_PIN_8
#define ST7735_DC_GPIO_Port GPIOA
#define ST7735_RST_Pin GPIO_PIN_10
#define ST7735_RST_GPIO_Port GPIOA
#define TL494_DTC_Pin GPIO_PIN_11
#define TL494_DTC_GPIO_Port GPIOA
#define ST7735_CS_Pin GPIO_PIN_12
#define ST7735_CS_GPIO_Port GPIOA
#define ENC2_S2_Pin GPIO_PIN_4
#define ENC2_S2_GPIO_Port GPIOB
#define ENC1_S2_Pin GPIO_PIN_5
#define ENC1_S2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
