/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Regulated power supply
 ******************************************************************************
 *WHAT I WANT FROM THIS PROJECT:
 *algorithm which transform set point of Voltage and Current into real output
 *current, power limitation feature
 *self calibration of this algorithm (is it 1,5,10V)
 *INA current calibration procedure (is it 0.1mA,1A)
 *use INA only for calibration, use ADC to read values
 *modes: main, soldering iron, usb (QC)
 *graph (sliding) and logs
 *stylish menu with sliders
 *temperature control
 *fun instead of buzzer
 *alarms notification feature: high temp, current limit,
 *FREERTOS ??
 *CMSIS ??
 *
 *
 *PERIF functions -> are for further optimization
 *
 *WHAT I FOUND:
 *current can be regulated only from 300 mA - this is lm358 problem
 *voltage rising from 0 to 20 V - 80ms
 *voltage rising from 0 to 1V - 80ms due to overshoot
 *voltage rising from 0 to 3 V - 20ms
 *overshoot from 0 to 1 V - 1.8V
 *overshoot from 0 to 3 V - 3.6V
 *overshoot from 0 to 5 V - 5.2V

 //flash writing
 STM32G431:
 quantity of pages: 63
 last Flash page -> 0x0801 F800 - 0x0801 FFFF
 quantity of banks -> only 1
 only 64 bit double word can be programmed


 TO DO:
 1) Current channel control
 2) More steps or algorithm for CTRL_SP_ReachBySteps
 3) Permanent control function to maintain stable voltage/current
 4) Slave channel check in permanent control


 ******************************************************************************
 */
/* USER CODE END Header */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "MGL/MGL.h"
#include "INA226/M_INA226.h"

#include "app_types.h"
#include "ctrl.h"
#include "input.h"
#include "display.h"
#include "main.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//INA settings
/////////////////////////////////////////////////////
#define INA_CALIB_VAL 2450U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN PV */
//Flash erase structure
////////////////////////////////////////////////////////////
FLASH_EraseInitTypeDef pflash;

//Encoder library structures
////////////////////////////////////////////////////////////
//extern menc_struct_type menc1, menc2; //Encoder library structures

//Timers for ms delay
////////////////////////////////////////////////////////////
uint32_t tmr_disp_screen_refresh;
uint32_t tmr_debug;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_DAC1_Init(void);
/* USER CODE BEGIN PFP */

//Encoders IRQ
////////////////////////////////////////////////////////////
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == ENC1_KEY_Pin) {
		MENC_ClickHandlerIRQ(&menc1);
	}
	if (GPIO_Pin == ENC1_S1_Pin) {
		MENC_TurnHandlerIRQ(&menc1);
	}
	if (GPIO_Pin == ENC2_S1_Pin) {
		MENC_TurnHandlerIRQ(&menc2);
	}
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_RTC_Init();
	MX_I2C1_Init();
	MX_SPI2_Init();
	MX_DAC1_Init();
	/* USER CODE BEGIN 2 */
	//////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////INT MAIN()////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////
	HAL_Delay(10); //some delay after periphery initialization

	//Main structure initialization
	//////////////////////////////////////////////////////////////////////////////////////
	rps_type rps = { 0 };

	//RPS and VAW
	//////////////////////////////////////////////////////////////////////////////////////
	HAL_DAC_Start(&hdac1, DAC_VOLT_CH);
	HAL_DAC_Start(&hdac1, DAC_CURR_CH);
	INA_Init();
	HAL_Delay(10); //some delay after periphery initialization
	if (HAL_I2C_IsDeviceReady(&hi2c1, INA_HAL_I2C_ADDRESS, 1, 0xff) != 0) {
		rps.err.bit.ina226_off = 1;
	}
	INA_SetCalVal(INA_CALIB_VAL);

	CTRL_SAVE_TableInit(&rps);
	CTRL_SAVE_CalcDACSteps(&rps, _VOLT);
	CTRL_SAVE_CalcDACSteps(&rps, _CURR);

	//Input
	//////////////////////////////////////////////////////////////////////////////////////
	INPUT_EncStructInit();

	//Display
	//////////////////////////////////////////////////////////////////////////////////////
	DISP_GraphBarsStructInit(&rps);
	MGL_DriverInit();
	DISP_StartPage(&rps);

	//Control flags and variables initialization states
	//////////////////////////////////////////////////////////////////////////////////////
	rps.fl.ctrl_stop = 1; //tl494 control is stopped


#ifdef USE_DEBUG
	printf("While start\n\r");
	CTRL_SAVE_PrintSavedTables();
	rps.val.i_sp_val = rps.val.i_max;
#endif

//////////////////////////////////////////////////////////////////////////////////////
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (1) {
		INPUT_EncHandler(&rps);

		if (MillisDelay(&tmr_disp_screen_refresh, SCREEN_REFRESH_RATE)) {
			CTRL_VAW_Conversion(&rps);
			DISP_MeasPage(&rps);
		}

		CTRL_Handler(&rps);

#ifdef USE_DEBUG
//		if (MillisDelay(&ms_tmr_debug, DEBUG_REFRESH)) {
//			printf("U:");
//			SERV_Print_FakeFloat(rps.val.volt, 5, 2);
//			printf("   I:");
//			SERV_Print_FakeFloat(rps.val.curr, 5, 3);
//			printf("   P:");
//			SERV_Print_FakeFloat(rps.val.watt, 5, 1);
//
//			printf("   U sp:");
//			SERV_Print_FakeFloat(rps.val.u_sp_val, 5, 2);
//			printf("   I sp:");
//			SERV_Print_FakeFloat(rps.val.i_sp_val, 5, 3);
//
//			printf("   DAC U:%u", rps.val.u_dac);
//			printf("   DAC I:%u\n", rps.val.i_dac);
//
//			printf("RPS Error No: %x\n", rps.err.all_errors);
//			MGL_PrintErr();
//		}
#endif
		//////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////END///////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
	RCC_OscInitStruct.PLL.PLLN = 85;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}

	/** Enables the Clock Securrity System
	 */
	HAL_RCC_EnableCSS();

	/** Enables the Clock Securrity System
	 */
	HAL_RCC_EnableLSECSS();
}

/**
 * @brief DAC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_DAC1_Init(void) {

	/* USER CODE BEGIN DAC1_Init 0 */

	/* USER CODE END DAC1_Init 0 */

	DAC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN DAC1_Init 1 */

	/* USER CODE END DAC1_Init 1 */

	/** DAC Initialization
	 */
	hdac1.Instance = DAC1;
	if (HAL_DAC_Init(&hdac1) != HAL_OK) {
		Error_Handler();
	}

	/** DAC channel OUT1 config
	 */
	sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
	sConfig.DAC_DMADoubleDataMode = DISABLE;
	sConfig.DAC_SignedFormat = DISABLE;
	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
	sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
	sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL;
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
	if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}

	/** DAC channel OUT2 config
	 */
	if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN DAC1_Init 2 */

	/* USER CODE END DAC1_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x40621236;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void) {

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI2_Init(void) {

	/* USER CODE BEGIN SPI2_Init 0 */

	/* USER CODE END SPI2_Init 0 */

	/* USER CODE BEGIN SPI2_Init 1 */

	/* USER CODE END SPI2_Init 1 */
	/* SPI2 parameter configuration*/
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;
	hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi2.Init.NSS = SPI_NSS_SOFT;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi2.Init.CRCPolynomial = 7;
	hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	if (HAL_SPI_Init(&hspi2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI2_Init 2 */

	/* USER CODE END SPI2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, ST7735_DC_Pin | ST7735_RST_Pin | TL494_DTC_Pin | ST7735_CS_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin : ENC1_KEY_Pin */
	GPIO_InitStruct.Pin = ENC1_KEY_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(ENC1_KEY_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : ENC2_S1_Pin ENC1_S1_Pin */
	GPIO_InitStruct.Pin = ENC2_S1_Pin | ENC1_S1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : ST7735_DC_Pin ST7735_RST_Pin ST7735_CS_Pin */
	GPIO_InitStruct.Pin = ST7735_DC_Pin | ST7735_RST_Pin | ST7735_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PERIF_TL494_DTC_Pin */
	GPIO_InitStruct.Pin = TL494_DTC_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TL494_DTC_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : ENC2_S2_Pin ENC1_S2_Pin */
	GPIO_InitStruct.Pin = ENC2_S2_Pin | ENC1_S2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
