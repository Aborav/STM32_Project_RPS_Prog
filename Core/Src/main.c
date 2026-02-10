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
 *
 *
 *WHAT I FOUND:
 *current can be regulated only from 300 mA
 *voltage rising from 0 to 20 V - 80ms
 *voltage rising from 0 to 1V - 80ms due to overshoot
 *voltage rising from 0 to 3 V - 20ms
 *overshoot from 0 to 1 V - 1.8V
 *overshoot from 0 to 3 V - 3.6V
 *overshoot from 0 to 5 V - 5.2V
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "STM32_Library_Encoder/M_ENC.h"
#include "STM32_Library_INA226/M_INA226.h"
#include "STM32_Library_My_Graphic_Library/MGL.h"
#include "STM32_Library_Signal_Filter/M_Filter.h"
#include "STM32_Library_Flashprom/M_Flashprom.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#define USE_DEBUG
#define DEBUG_REFRESH 500

//INA settings
/////////////////////////////////////////////////////
//#define INA_CALIB_MODE
#define INA_CALIB_VAL 2450

//VAW settings
/////////////////////////////////////////////////////
#define VAW_MAX_VOLT 2400
#define VAW_MAX_CURR 5000
#define VAW_MAX_WATT 1200

//Graphic settings
/////////////////////////////////////////////////////
//SCREEN RESOLUTION : 160x128

/////////////////////////////////////////////////////
#define SCREEN_REFRESH_RATE 30 ///<ms

//Main field rectengle
/////////////////////////////////////////////////////
#define MAIN_FIELD_X 3
#define MAIN_FIELD_Y 3
#define MAIN_FIELD_WIDTH 154
#define MAIN_FIELD_HEIGHT 122

//Working field dimensions
/////////////////////////////////////////////////////
#define MEAS_FIELD_X 5
#define MEAS_FIELD_Y 5
#define MEAS_FIELD_WIDTH 150
#define MEAS_FIELD_HEIGHT 118

//Measurements location on the display
/////////////////////////////////////////////////////
#define VAW_VOLTAGE_X MEAS_FIELD_X+4
#define VAW_VOLTAGE_Y MEAS_FIELD_Y
#define VAW_CURRENT_X MEAS_FIELD_X+4
#define VAW_CURRENT_Y MEAS_FIELD_Y+30
#define VAW_WATTAGE_X MEAS_FIELD_X+4
#define VAW_WATTAGE_Y MEAS_FIELD_Y+59

//Progress bar
/////////////////////////////////////////////////////
#define BAR_VOLTAGE_X MEAS_FIELD_X+128
#define BAR_VOLTAGE_Y MEAS_FIELD_Y
#define BAR_CURRENT_X MEAS_FIELD_X+135
#define BAR_CURRENT_Y MEAS_FIELD_Y
#define BAR_WATTAGE_X MEAS_FIELD_X+142
#define BAR_WATTAGE_Y MEAS_FIELD_Y
#define BAR_WIDTH 6
#define BAR_LENGTH 75

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
#define LOW_ST_BAR_X 7
#define LOW_ST_BAR_Y 100
#define LOW_ST_BAR_W 146
#define LOW_ST_BAR_H 21

#define LOW_INF_BAR_UPP_Y 102  ///<upper string X
#define LOW_INF_BAR_LOW_Y 112  ///<lower string X

//fast display functions
/////////////////////////////////////////////////////
#define REFRESH_MAIN_FIELD() MGL_FillRectWH(MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_WIDTH, MAIN_FIELD_HEIGHT, BG_COLOR)
#define DRAW_LOW_INF_BAR() MGL_DrawRectWH(LOW_ST_BAR_X, LOW_ST_BAR_Y, LOW_ST_BAR_W, LOW_ST_BAR_H, LOW_INF_BAR_STROKE_COLOR)///<lower information stroke
#define DRAW_MAIN_FIELD_STROKE() MGL_DrawRectWH(MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_WIDTH, MAIN_FIELD_HEIGHT - 1, MP_STROKE_INT_COLOR)///<main field stroke

//TL494
/////////////////////////////////////////////////////
#define TL494_ON() HAL_GPIO_WritePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin,0)
#define TL494_OFF() HAL_GPIO_WritePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin,1)
#define TL494_TOGGLE() HAL_GPIO_TogglePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin)

//DAC fast function
/////////////////////////////////////////////////////
#define RPS_DAC_U_SET(x) HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, x);
#define RPS_DAC_I_SET(x) HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, x);

//RPS configuration
/////////////////////////////////////////////////////
#define RPS_TABLE_DELAY 100 //this is delay between DAC step when the FB table is filling up (min 70ms)
#define RPS_TABLE_DAC_STEP 100
#define RPS_TABLE_SIZE 42

#define RPS_TIMEOUT_THRESHOLD 100000000

//Errors
/////////////////////////////////////////////////////
#define RPS_ERR_SET_NUM(x) rps_error_var|=1<<(x);

#define RPS_ERR_TIMEOUT 0
#define RPS_ERR_INA 1
#define RPS_ERR_FLASH 2

//Write to flash function defines
/////////////////////////////////////////////////////
#define FLASHROM_VOLT_TABLE_ADDR 0x0801F800 //last page start No

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
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
struct menc_struct_type menc1, menc2;
volatile uint8_t flag_enc1_turn_all_dir;
volatile uint8_t flag_enc2_turn_all_dir;

//Values structure
////////////////////////////////////////////////////////////
struct vaw_meas {
	uint16_t volt;
	uint16_t curr;
	uint16_t watt;
	int16_t dac_u;
	int16_t dac_i;
	int16_t sp_u_val;
	int16_t sp_i_val;
} vaw;

//Feedback tables
uint16_t table_dac_step[RPS_TABLE_SIZE]; ///<DAC from 0 to 4095
uint16_t table_volt_fb[RPS_TABLE_SIZE]; ///<ralated to DAC step voltage
uint16_t table_curr_fb[RPS_TABLE_SIZE]; ///<ralated to DAC step current

uint16_t vaw_dac_u_min; ///<minimum voltage after calibration DAC to current
uint16_t vaw_dac_u_max; ///<maximum voltage after calibration DAC to current
uint16_t vaw_dac_i_min; ///<minimum current after calibration DAC to current
uint16_t vaw_dac_i_max; ///<maximum current after calibration DAC to current

//Bits field for status flags
////////////////////////////////////////////////////////////
struct flags_type {
	unsigned tl494_on :1; ///<TL494 clocking is ON
} sflags;

//Progress bars struct init
////////////////////////////////////////////////////////////
struct bar_gr volt_bar, curr_bar, watt_bar; //progress bars init

//Timers
////////////////////////////////////////////////////////////
uint32_t millis_tmr_screen_refresh;
uint32_t millis_tmr_rst_display;

//Error variable
////////////////////////////////////////////////////////////
uint8_t rps_error_var; ///<errors holding variable
/*
 0
 b
 7
 6
 5
 4
 3
 2 -> Flash erase error
 1 -> INA226 no response
 0 -> cycles timeout
 */

//Timeout counter variable
////////////////////////////////////////////////////////////
uint32_t rps_timeout_cnt; ///<counts till threshold, then abort cycle

//Debug
////////////////////////////////////////////////////////////
uint32_t millis_tmr_debug;
uint32_t flash_err;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_DAC1_Init(void);
/* USER CODE BEGIN PFP */
////////////////////////////////////////////////////////////
void SERV_Flash_EraseStructInit(void);

void HMI_Input_EncodersStructInit(void);
void HMI_Input(void);

void HMI_Display_GraphBarsStructInit(void);
void HMI_Display_StartPage(void);
void HMI_Display_MeasPage(void);

void RPS_VAW_Conversion(void);
void RPS_Save_FBTableVolt(void);
void RPS_Save_FBTableCurr(void);
void RPS_Ctrl_U_SPReach(uint16_t set_point);
void RPS_Ctrl_I_SPReach(uint16_t set_point);

//Encoders IRQ
////////////////////////////////////////////////////////////
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == ENC1_KEY_Pin) {
		MENC_ClickHandlerIRQ(&menc1);
	}
	if (GPIO_Pin == ENC1_S1_Pin) {
		MENC_TurnHandlerIRQ(&menc1);
		flag_enc1_turn_all_dir = 1;
	}
	if (GPIO_Pin == ENC2_S1_Pin) {
		MENC_TurnHandlerIRQ(&menc2);
		flag_enc2_turn_all_dir = 1;
	}
}

//Printf
////////////////////////////////////////////////////////////
int _write(int file, uint8_t *ptr, int len) {
//  (void)file;
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		ITM_SendChar(*ptr++);
	}
	return len;
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


	//Service
	//////////////////////////////////////////////////////////////////////////////////////
	SERV_Flash_EraseStructInit();

	//RPS and VAW
	//////////////////////////////////////////////////////////////////////////////////////
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
	INA_Init();
	if(HAL_I2C_IsDeviceReady(&hi2c1, INA_I2C_ADDRESS, 1, 0xff)!=0){
		RPS_ERR_SET_NUM(RPS_ERR_INA);
	}
	INA_SetCalVal(INA_CALIB_VAL);

	//HMI
	//////////////////////////////////////////////////////////////////////////////////////
	HMI_Input_EncodersStructInit();
	HMI_Display_GraphBarsStructInit();
	MGL_DriverInit();
	MGL_SET_BUF_BG_COLOR(COLOR_BLACK); //background for text
	HMI_Display_StartPage();

	//filling tables
	RPS_Save_FBTableVolt();
	volt_bar.num_max = vaw_dac_u_max; //renew maximum value after calibration

	RPS_Save_FBTableCurr();
	curr_bar.num_max = vaw_dac_i_max;

	//flash writing
	/*STM32G431:
	 quantity of pages: 63
	 last Flash page -> 0x0801 F800 - 0x0801 FFFF
	 quantity of banks -> only 1
	 only 64 bit double word can be programmed
	 */
	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&pflash, &flash_err);
	if(flash_err!=0xffffffff){
		RPS_ERR_SET_NUM(RPS_ERR_FLASH);
	}
	HAL_FLASH_Lock();
	MFLPR_WriteArray(FLASHROM_VOLT_TABLE_ADDR, table_volt_fb, RPS_TABLE_SIZE);



#ifdef USE_DEBUG
	printf("Program start\n\r");
#endif

	//////////////////////////////////////////////////////////////////////////////////////
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (1) {
		HMI_Input();

		if (HAL_GetTick() - millis_tmr_screen_refresh >= SCREEN_REFRESH_RATE) {
			millis_tmr_screen_refresh = HAL_GetTick();
			RPS_VAW_Conversion();
			HMI_Display_MeasPage();
		}


		if (HAL_GetTick() - millis_tmr_rst_display >= 20000) {
			millis_tmr_rst_display = HAL_GetTick();
			MGL_DriverInit();
			MGL_SET_BUF_BG_COLOR(COLOR_BLACK); //background for text
			HMI_Display_StartPage();
		}

#ifdef USE_DEBUG
		if (HAL_GetTick() - millis_tmr_debug >= DEBUG_REFRESH) {
			millis_tmr_debug = HAL_GetTick();

		}
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

	/*Configure GPIO pin : TL494_DTC_Pin */
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


/////////////////////////////////////////////////////////////////////////
/* USER CODE BEGIN 4 */
/*
 * @brief Function fill up HAL erase structure
 */
void SERV_Flash_EraseStructInit(void) {
	pflash.TypeErase = FLASH_TYPEERASE_PAGES;
	pflash.NbPages = 1;
	pflash.Page = 63;
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to fill up encoders structures
 */
void HMI_Input_EncodersStructInit(void){
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
 * @brief Function to fill up graph bars for display
 */
void HMI_Display_GraphBarsStructInit(void) {
	volt_bar.num = &vaw.volt;
	volt_bar.num_max = VAW_MAX_VOLT;
	volt_bar.x = BAR_VOLTAGE_X;
	volt_bar.y = BAR_VOLTAGE_Y;
	volt_bar.width = BAR_WIDTH;
	volt_bar.length = BAR_LENGTH;
	volt_bar.color = VOLT_COLOR;
	volt_bar.bg_color = BG_COLOR;
	volt_bar.stroke_color = BAR_STROKE_COLOR;

	curr_bar.num = &vaw.curr;
	curr_bar.num_max = VAW_MAX_CURR;
	curr_bar.x = BAR_CURRENT_X;
	curr_bar.y = BAR_CURRENT_Y;
	curr_bar.width = BAR_WIDTH;
	curr_bar.length = BAR_LENGTH;
	curr_bar.color = CURR_COLOR;
	curr_bar.bg_color = BG_COLOR;
	curr_bar.stroke_color = BAR_STROKE_COLOR;

	watt_bar.num = &vaw.watt;
	watt_bar.num_max = VAW_MAX_WATT;
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
 * @brief Function to maintain clicks and turns response
 */
void HMI_Input(void) {
	MENC_MainHandler(&menc1);
	MENC_MainHandler(&menc2);

	//turn on/off TL494 clocking
	if (MENC_Click(&menc1)) {
		sflags.tl494_on = ~sflags.tl494_on;
//		if (sflags.tl494_on) {
//			VAW_U_SPReach(vaw.sp_u_val);
//			VAW_I_SPReach(vaw.sp_i_val);
//			VAW_DAC_U_SET(vaw.sp_u_val);
//			VAW_DAC_I_SET(vaw.sp_i_val);
//		} else {
//			VAW_DAC_U_SET(0);
//			VAW_DAC_I_SET(0);
//			vaw.dac_i = 0;
//			vaw.dac_u = 0;
//		}
		RPS_DAC_U_SET(vaw.dac_u);
		RPS_DAC_I_SET(vaw.dac_i);
		TL494_TOGGLE();
	}

	//voltage trim
	if (MENC_TurnRight(&menc1)) {
//		vaw.sp_u_val += 10;
//		if (vaw.sp_u_val >= vaw_dac_u_max)
//			vaw.sp_u_val = vaw_dac_u_max;
		vaw.dac_u += 10;
		if (vaw.dac_u >= 4095)
			vaw.dac_u = 4095;
	}

	if (MENC_TurnLeft(&menc1)) {
//		vaw.sp_u_val -= 10;
//		if (vaw.sp_u_val < 0)
//			vaw.sp_u_val = 0;
		vaw.dac_u -= 10;
		if (vaw.dac_u < 0)
			vaw.dac_u = 0;
	}

	//current trim
	if (MENC_TurnRight(&menc2)) {
//		vaw.sp_i_val += 10;
//		if (vaw.sp_i_val >= vaw_dac_i_max)
//			vaw.sp_i_val = vaw_dac_i_max;
		vaw.dac_i += 10;
		if (vaw.dac_i >= 4095)
			vaw.dac_i = 4095;
	}

	if (MENC_TurnLeft(&menc2)) {
//		vaw.sp_i_val -= 10;
//		if (vaw.sp_i_val < 0)
//			vaw.sp_i_val = 0;
		vaw.dac_i -= 10;
		if (vaw.dac_i < 0)
			vaw.dac_i = 0;
	}

	//if any turn
	if (flag_enc1_turn_all_dir) {
		flag_enc1_turn_all_dir = 0;

		if (sflags.tl494_on) {
			//RPS_U_SPReach(vaw.sp_u_val);
			RPS_DAC_U_SET(vaw.dac_u);
		}
	}
	if (flag_enc2_turn_all_dir) {
		flag_enc2_turn_all_dir = 0;

		if (sflags.tl494_on) {
//			RPS_I_SPReach(vaw.sp_i_val);
			RPS_DAC_I_SET(vaw.dac_i);
		}
	}
	//		//fast turn
	//		if (MENC_TurnFastRight(&menc1)) {
	//			dac_volt_value += 20;
	//			if (dac_volt_value > 4095) dac_volt_value = 4095;
	//			//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_volt_value);
	//			sflags.volt_draw = 1;
	//		}
	//
	//		if (MENC_TurnFastRight(&menc2)) {
	//			dac_curr_value += 20;
	//			if (dac_curr_value > 4095) dac_curr_value = 4095;
	//			//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac_curr_value);
	//			sflags.curr_draw = 1;
	//
	//		}
	//		if (MENC_TurnFastLeft(&menc1)) {
	//			dac_volt_value -= 20;
	//			if (dac_volt_value < 0) dac_volt_value = 0;
	//			//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_volt_value);
	//			sflags.volt_draw = 1;
	//		}
	//
	//		if (MENC_TurnFastLeft(&menc2)) {
	//			dac_curr_value -= 20;
	//			if (dac_curr_value < 0) dac_curr_value = 0;
	//			//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac_curr_value);
	//			sflags.curr_draw = 1;
	//		}
}


/////////////////////////////////////////////////////////////////////////
/*
 * @brief starting page drawing function
 */
void HMI_Display_StartPage(void) {
//	REFRESH_MAIN_FIELD();
//	DRAW_MAIN_FIELD_STROKE();
	//DRAW_LOW_INF_BAR();
	MGL_FillScreen(COLOR_BLACK);

	//not redrawing stack
	MGL_SET_BUF_COLOR(VOLT_COLOR);
	MGL_PrintStr_17x24("U\0", 108, VAW_VOLTAGE_Y);
	MGL_DrawRectWH(VAW_VOLTAGE_X+FONT_17x24_WIDTH*2+4, VAW_VOLTAGE_Y+FONT_17x24_HEIGHT-2, 2, 2, VOLT_COLOR);

	MGL_SET_BUF_COLOR(CURR_COLOR);
	MGL_PrintStr_17x24("A\0", 108, VAW_CURRENT_Y);
	MGL_DrawRectWH(VAW_CURRENT_X+FONT_17x24_WIDTH*1+4, VAW_CURRENT_Y+FONT_17x24_HEIGHT-2, 2, 2, CURR_COLOR);

	MGL_SET_BUF_COLOR(WATT_COLOR);
	MGL_PrintStr_17x24("W\0", 108, VAW_WATTAGE_Y);
	MGL_DrawRectWH(VAW_WATTAGE_X+FONT_17x24_WIDTH*3+4, VAW_WATTAGE_Y+FONT_17x24_HEIGHT-2, 2, 2, WATT_COLOR);

	MGL_SET_BUF_COLOR(FONT_COLOR);
	MGL_PrintStr_5x8("DAC_U:\0", 5, LOW_INF_BAR_UPP_Y);
	MGL_PrintStr_5x8("DAC_I:\0", 5, LOW_INF_BAR_LOW_Y);

	MGL_PrintStr_5x8("SP_U:\0", 80, LOW_INF_BAR_UPP_Y);
	MGL_PrintStr_5x8("SP_I:\0", 80, LOW_INF_BAR_LOW_Y);
}


/////////////////////////////////////////////////////////////////////////
/*
 * @brief draw redrawing stack of the main page
 */
void HMI_Display_MeasPage(void) {
	static uint16_t volt_old, curr_old, watt_old;
	static uint16_t sp_u_old, sp_i_old, dac_u_old, dac_i_old;
	int16_t diff = 0; ///<differance between new and old values

	//voltage
	diff = volt_old - vaw.volt;
	if (diff < 0)
		diff *= -1; //no matter is old value bigger or smaller than a new one
	if (diff > sflags.tl494_on ? 1 : 0) { //eliminate fluctuation of the value
		MGL_SET_BUF_COLOR(VOLT_COLOR);
		MGL_PrintFloatTinyR(vaw.volt, 4, 2, VAW_VOLTAGE_X, VAW_VOLTAGE_Y, FONT_17x24_FP);
		MGL_DrawBar(&volt_bar);
	}

	//current
	diff = curr_old - vaw.curr;
	if (diff < 0)
		diff *= -1;
	if (diff > sflags.tl494_on ? 1 : 0) {
		MGL_SET_BUF_COLOR(CURR_COLOR);
		MGL_PrintFloatTinyR(vaw.curr, 4, 3, VAW_CURRENT_X, VAW_CURRENT_Y, FONT_17x24_FP);
		MGL_DrawBar(&curr_bar);
	}

	//wattage
	diff = watt_old - vaw.watt;
	if (diff < 0)
		diff *= -1;
	if (diff > sflags.tl494_on ? 1 : 0) {
		MGL_SET_BUF_COLOR(WATT_COLOR);
		MGL_PrintFloatTinyR(vaw.watt, 4, 1, VAW_WATTAGE_X, VAW_WATTAGE_Y, FONT_17x24_FP);
		MGL_DrawBar(&watt_bar);
	}

	MGL_SET_BUF_COLOR(FONT_COLOR);

	//voltage DAC value
	diff = dac_u_old - vaw.dac_u;
	if (diff < 0)
		diff *= -1;
	if (diff > 0) {
		MGL_PrintUintR(vaw.dac_u, 4, 45, LOW_INF_BAR_UPP_Y, FONT_5x8_FP);
		MGL_DrawBar(&watt_bar);
	}

	//current dac value
	diff = dac_i_old - vaw.dac_i;
	if (diff < 0)
		diff *= -1;
	if (diff > 0) {
		MGL_PrintUintR(vaw.dac_i, 4, 45, LOW_INF_BAR_LOW_Y, FONT_5x8_FP);
	}

	//voltage set point
	diff = sp_u_old - vaw.sp_u_val;
	if (diff < 0)
		diff *= -1;
	if (diff > 0) {
		MGL_PrintFloatTinyR(vaw.sp_u_val, 4, 2, 120, LOW_INF_BAR_UPP_Y, FONT_5x8_FP);
	}

	//current set point
	diff = sp_i_old - vaw.sp_i_val;
	if (diff < 0)
		diff *= -1;
	if (diff > 0) {
		MGL_PrintFloatTinyR(vaw.sp_i_val, 4, 3, 120, LOW_INF_BAR_LOW_Y, FONT_5x8_FP);
	}

	volt_old = vaw.volt;
	curr_old = vaw.curr;
	watt_old = vaw.watt;
	sp_u_old = vaw.sp_u_val;
	sp_i_old = vaw.sp_i_val;
	dac_u_old = vaw.dac_u;
	dac_i_old = vaw.dac_i;
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief measurements function
 */
void RPS_VAW_Conversion(void) {

	//static uint16_t med_fil_volt_buf[3],med_fil_curr_buf[3];

	vaw.volt = INA_GetBusVoltageTiny();
	//FilterMedian(&vaw.volt, med_fil_volt_buf);

	vaw.curr = INA_GetCurrentTiny();
	//FilterMedian(&vaw.curr, med_fil_curr_buf);

	vaw.watt = INA_GetPowerTiny();

}



/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to make DAC to voltage related table
 */
void RPS_Save_FBTableVolt(void) {
	uint16_t val = 0;

	RPS_DAC_I_SET(4095); //get current on maximum to eliminate affect from it
	TL494_ON();

	//increase DAC value +100 until max and put voltage feedback and appropriate DAC values in the arrays
	for (uint8_t i = 0; i <= RPS_TABLE_SIZE - 1; i++) {
		val = i * 100;
		if (val > 4095)
			val = 4095;
		RPS_DAC_U_SET(val); //voltage increasing
		HAL_Delay(RPS_TABLE_DELAY); //wait until capacitor is charged
		vaw.dac_u = val;
		*(table_dac_step + i) = val;
		RPS_VAW_Conversion();
		*(table_volt_fb + i) = vaw.volt;
		HMI_Display_MeasPage();
	}

	//turn off everything
	TL494_OFF();
	RPS_DAC_U_SET(0);
	RPS_DAC_I_SET(0);
	vaw.dac_u = 0;
	vaw_dac_u_min = *(table_volt_fb + 0);
	vaw_dac_u_max = *(table_volt_fb + RPS_TABLE_SIZE - 1);

	rps_timeout_cnt = 0; //reset saving counter

	//wait until capacitor is discharged
	while (vaw.volt != 0) {
		RPS_VAW_Conversion();
		HMI_Display_MeasPage();
		if (rps_timeout_cnt >= RPS_TIMEOUT_THRESHOLD) {
			RPS_ERR_SET_NUM(RPS_ERR_TIMEOUT);
			return;
		}
		rps_timeout_cnt++;
	}
}


/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to make DAC to current related table
 */
void RPS_Save_FBTableCurr(void) {
	uint16_t val = 0;
	int16_t old_curr = 0;
	RPS_DAC_U_SET(4095); //get voltage on maximum to eliminate affect from it
	TL494_ON();

	//increase DAC value +100 until max and put current feedback and appropriate DAC values in the arrays
	for (uint8_t i = 0; i <= RPS_TABLE_SIZE - 1; i++) {
		val = i * 100;
		if (val > 4095)
			val = 4095;
		RPS_DAC_I_SET(val); //voltage increasing
		HAL_Delay(RPS_TABLE_DELAY); //wait until output capacitor is charged
		vaw.dac_i = val;
		*(table_dac_step + i) = val;
		RPS_VAW_Conversion();
		*(table_curr_fb + i) = vaw.curr;
		HMI_Display_MeasPage();
	}

	//turn off everything
	TL494_OFF();
	RPS_DAC_U_SET(0);
	RPS_DAC_I_SET(0);
	vaw.dac_i = 0;
	vaw_dac_i_min = *(table_curr_fb + 0);

	//if DAC stepped one more time but current feedback value is the same
	for (uint8_t i = 0; i <= RPS_TABLE_SIZE - 1; i++) {
		if (*(table_curr_fb + i) - old_curr == 0) {
			vaw_dac_i_max = *(table_curr_fb + i - 1); //write maximum current
			break;
		}
		if (i == RPS_TABLE_SIZE - 1) {
			vaw_dac_i_max = *(table_curr_fb + i - 1); //write maximum current
		}
		old_curr = vaw.curr;
	}

	rps_timeout_cnt = 0; //reset saving counter

	while (vaw.volt != 0) {
		RPS_VAW_Conversion();
		HMI_Display_MeasPage();
		if (rps_timeout_cnt >= RPS_TIMEOUT_THRESHOLD) {
			RPS_ERR_SET_NUM(RPS_ERR_TIMEOUT);
			return;
		}
		rps_timeout_cnt++;
	}
}


/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to reach current by setting a set point
 * This function controls current increasing current DAC
 */
void RPS_Ctrl_I_SPReach(uint16_t set_point) {
	if (set_point == 0) {
		vaw.dac_i = 0; //mix
	} else if (set_point == vaw_dac_i_max) {
		vaw.dac_i = 4095; //max
	} else {
		for (uint8_t i = 0; i <= 41; i++) {
			//compare voltage from the feedback voltage table and a set point.
			//to find a table pointer with closest to SP voltage but smaller than it
			if (*(table_curr_fb + i) > set_point) {
				vaw.dac_i = *(table_dac_step + i - 1); //use this pointer but in the DAC values table
				//here must be some protection from pointer miss read. But my first value in the tables are 0
				//nothing is smaller than 0 in uint16_t
				break;
			}
		}
	}
	RPS_DAC_I_SET(vaw.dac_i);
}


/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to reach voltage by setting a set point
 * @param sp_val-> set point
 */
void RPS_Ctrl_U_SPReach(uint16_t set_point) {
	if (set_point == 0) {
		vaw.dac_u = 0; //mix
	} else if (set_point == vaw_dac_u_max) {
		vaw.dac_u = 4095; //max
	} else {
		for (uint8_t i = 0; i <= 41; i++) {
			//compare voltage from the feedback voltage table and a set point.
			//to find a table pointer with closest to SP voltage but smaller than it
			if (*(table_volt_fb + i) > set_point) {
				vaw.dac_u = *(table_dac_step + i - 1); //use this pointer but in the DAC values table
				//here must be some protection from pointer miss read. But my first value in the tables are 0
				//nothing is smaller than 0 in uint16_t
				break;
			}
		}
	}
	RPS_DAC_U_SET(vaw.dac_u);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
