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
//INA settings
/////////////////////////////////////////////////////
//#define INA_CALIB_MODE
#define INA_CALIB_VAL 2450U

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

//fast display functions
/////////////////////////////////////////////////////
#define REFRESH_MAIN_FIELD() MGL_FillRectWH(MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_WIDTH, MAIN_FIELD_HEIGHT, BG_COLOR)
#define DRAW_LOW_INF_BAR() MGL_DrawRectWH(LOW_ST_BAR_X, LOW_ST_BAR_Y, LOW_ST_BAR_W, LOW_ST_BAR_H, LOW_INF_BAR_STROKE_COLOR)///<lower information stroke
#define DRAW_MAIN_FIELD_STROKE() MGL_DrawRectWH(MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_WIDTH, MAIN_FIELD_HEIGHT - 1, MP_STROKE_INT_COLOR)///<main field stroke

//PERIF_TL494
/////////////////////////////////////////////////////
#define PERIF_TL494_ON() HAL_GPIO_WritePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin,0)
#define PERIF_TL494_OFF() HAL_GPIO_WritePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin,1)
#define PERIF_TL494_TOGGLE() HAL_GPIO_TogglePin(TL494_DTC_GPIO_Port, TL494_DTC_Pin)

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

//DEBUG
#define USE_DEBUG
#define DEBUG_REFRESH 300U

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
menc_struct_type menc1, menc2;

//Progress bars struct init
////////////////////////////////////////////////////////////
bar_gr volt_bar, curr_bar, watt_bar; //progress bars init

//Values structure
////////////////////////////////////////////////////////////
typedef struct _values_type {
	uint16_t volt; ///<value from measuring source (INA226)
	uint16_t curr; ///<value from measuring source (INA226)
	uint16_t watt; ///<value from measuring source (INA226)
	int16_t dac_u; ///<will be written into DAC register
	int16_t dac_i; ///<will be written into DAC register
	int16_t sp_u_val; ///<set point
	int16_t sp_i_val; ///<set point
	uint16_t u_min; ///<minimum voltage after calibration
	uint16_t u_max; ///<maximum voltage after calibration
	uint16_t i_min; ///<minimum current after calibration
	uint16_t i_max; ///<maximum current after calibration
} values_type;

//Bits field for status flags
////////////////////////////////////////////////////////////
typedef struct _flags_type {
	unsigned tl494_on :1; ///<TL494 clocking is ON
} flags_type;

//Bits field for errors
////////////////////////////////////////////////////////////
typedef union _errors_type {
	uint8_t all_errors;
	struct {
		unsigned cicle_timeout :1; ///<
		unsigned ina226_off :1;
		unsigned flash_erase :1;
		unsigned flash_write :1;
		unsigned reserved :4;
	} bit;
} errors_type;

//COMMON STRUCTURE
////////////////////////////////////////////////////////////
typedef struct _rps_type {
	values_type val;
	flags_type fl;
	errors_type err;
} rps_type;

//Unions for functions
////////////////////////////////////////////////////////////
typedef enum _channel_type {
	_VOLT = 0, _CURR = 1
} channel_type;

//Timers for millist delay
////////////////////////////////////////////////////////////
uint32_t ms_tmr_screen_refresh;
uint32_t ms_tmr_debug;

//Feedback tables
////////////////////////////////////////////////////////////
const uint16_t *table_ptr_u = (uint16_t*) TABLE_VOLT_ADDR;
const uint16_t *table_ptr_i = (uint16_t*) TABLE_CURR_ADDR;

uint16_t table_dac_step[RPS_TABLE_SIZE]; ///<DAC from 0 to 4095
//uint16_t table_volt_fb[RPS_TABLE_SIZE]; ///<ralated to DAC step voltage
//uint16_t table_curr_fb[RPS_TABLE_SIZE]; ///<ralated to DAC step current

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
void SERV_Flash_EraseTable(rps_type *r);
void SERV_Print_FakeFloat(uint16_t fake_f, uint8_t num_quant, uint8_t num_aft_comma);

void HMI_Input_EncodersStructInit(void);
void HMI_Input(rps_type *r);

void HMI_Display_GraphBarsStructInit(rps_type *r);
void HMI_Display_StartPage(void);
void HMI_Display_MeasPage(rps_type *r);
void HMI_Display_DebugMeasPage(rps_type *r);

void RPS_VAW_Conversion(rps_type *r);
void RPS_Save_FBTableVolt(rps_type *r);
void RPS_Save_FBTableCurr(rps_type *r);
void RPS_Save_Table(rps_type *r);
void RPS_Save_TableInit(rps_type *r);
void RPS_Save_PrintSavedTables(void);
void RPS_Ctrl_U_SPReach(uint16_t set_point, rps_type *r);
void RPS_Ctrl_I_SPReach(uint16_t set_point, rps_type *r);

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

bool MillisDelay(uint32_t *counter, uint16_t delay) {
	if (HAL_GetTick() - *counter <= delay) {
		return 0;
	} else {
		*counter = HAL_GetTick();
		return 1;
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

	//Service
	//////////////////////////////////////////////////////////////////////////////////////
	SERV_Flash_EraseStructInit();

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

	RPS_Save_TableInit(&rps);

	//Input
	//////////////////////////////////////////////////////////////////////////////////////
	HMI_Input_EncodersStructInit();

	//Display
	//////////////////////////////////////////////////////////////////////////////////////
	HMI_Display_GraphBarsStructInit(&rps);
	MGL_DriverInit();
	MGL_SET_BUF_BG_COLOR(COLOR_BLACK); //background for text
	HMI_Display_StartPage();

#ifdef USE_DEBUG
	printf("While start\n\r");
	RPS_Save_PrintSavedTables();
#endif

	//////////////////////////////////////////////////////////////////////////////////////
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (1) {
		HMI_Input(&rps);

		if (MillisDelay(&ms_tmr_screen_refresh, SCREEN_REFRESH_RATE)) {
			RPS_VAW_Conversion(&rps);
			//HMI_Display_MeasPage();
		}
#ifdef USE_DEBUG
		if (MillisDelay(&ms_tmr_debug, DEBUG_REFRESH)) {
			printf("U:");
			SERV_Print_FakeFloat(rps.val.volt, 4, 2);
			printf("   I:");
			SERV_Print_FakeFloat(rps.val.curr, 4, 3);
			printf("   P:");
			SERV_Print_FakeFloat(rps.val.watt, 4, 1);

			printf("   U sp:");
			SERV_Print_FakeFloat(rps.val.sp_u_val, 4, 2);
			printf("   I sp:");
			SERV_Print_FakeFloat(rps.val.sp_i_val, 4, 3);

			printf("   DAC U:%u", rps.val.dac_u);
			printf("   DAC I:%u\n", rps.val.dac_i);

			if (rps.err.all_errors != 0) {
				printf("\nError No: %x\n", rps.err.all_errors);
			}
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
/////////////////////////////////////////////////////////////////////////
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
 * @brief Function to erase both tables, one page
 */
void SERV_Flash_EraseTable(rps_type *r) {
	uint32_t flash_err;

	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&pflash, &flash_err);
	HAL_FLASH_Lock();

	if (flash_err != 0xffffffff) {
		r->err.bit.flash_erase = 1;
		return;
	}
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to print uint16_t as float
 * @param[in] fake_f -> uint16_t
 * @param[in] num_quant -> number of digits
 * @param[in] num_aft_comma -> digits after comma
 */
void SERV_Print_FakeFloat(uint16_t fake_f, uint8_t num_quant, uint8_t num_aft_comma) {
	char str[8];	//create array for PrintStr, quantity of numbers+"\0"
	char *str_ptr = str; ///<ptr to scroll array
	uint16_t div_buf;	//buffer to hold digit for division
	uint16_t div_mask[] = { 1, 10, 100, 1000, 10000 }; ///<for not to use POW function
	uint8_t clear_flag = 1; ///<stop clear zeros flag

	for (uint8_t i = 4, j = 0; i > 0; i--, j++) {
		div_buf = (fake_f / div_mask[i]); ///<extract one by one digits from input var
		fake_f -= (div_buf) * div_mask[i]; ///<remove pow in num
		if (div_buf > 0)
			clear_flag = 0; ///<wait untill num starts
		if (clear_flag == 0)
			*str_ptr++ = div_buf + '0';
		if (num_aft_comma == i && div_buf == 0) {
			*str_ptr++ = '0';
			*str_ptr++ = '.';
		} else if (num_aft_comma == i)
			*str_ptr++ = '.';
	}
	*str_ptr++ = fake_f + '0';	///<last digit
	*str_ptr++ = '\0'; //add end of string to finish printing in PrintStr function

	printf("%s", str);
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
 * @brief Function to fill up graph bars for display
 * @param[in/out] *r -> project structure pointer
 */
void HMI_Display_GraphBarsStructInit(rps_type *r) {
	volt_bar.num = &r->val.volt;
	volt_bar.num_max = VAW_MAX_VOLT;
	volt_bar.x = BAR_VOLTAGE_X;
	volt_bar.y = BAR_VOLTAGE_Y;
	volt_bar.width = BAR_WIDTH;
	volt_bar.length = BAR_LENGTH;
	volt_bar.color = VOLT_COLOR;
	volt_bar.bg_color = BG_COLOR;
	volt_bar.stroke_color = BAR_STROKE_COLOR;

	curr_bar.num = &r->val.curr;
	curr_bar.num_max = VAW_MAX_CURR;
	curr_bar.x = BAR_CURRENT_X;
	curr_bar.y = BAR_CURRENT_Y;
	curr_bar.width = BAR_WIDTH;
	curr_bar.length = BAR_LENGTH;
	curr_bar.color = CURR_COLOR;
	curr_bar.bg_color = BG_COLOR;
	curr_bar.stroke_color = BAR_STROKE_COLOR;

	watt_bar.num = &r->val.watt;
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
 * @param[in/out] *r -> project structure pointer
 */
void HMI_Input(rps_type *r) {
	MENC_MainHandler(&menc1);
	MENC_MainHandler(&menc2);

	//turn on/off PERIF_TL494 clocking
	if (MENC_Click(&menc1)) {
		r->fl.tl494_on = ~r->fl.tl494_on;
		if (r->fl.tl494_on) {
			RPS_Ctrl_U_SPReach(r->val.sp_u_val, r);
			RPS_Ctrl_I_SPReach(r->val.sp_i_val, r);
			PERIF_TL494_ON();
		} else {
			PERIF_TL494_OFF();
		}
		//		PERIF_DAC_SET(r->val.dac_u, DAC_VOLT_CH);
		//		PERIF_DAC_SET(r->val.dac_i, DAC_CURR_CH);
	}

	//voltage trim
	if (MENC_TurnRight(&menc1)) {
		r->val.sp_u_val += 10;
		if (r->val.sp_u_val >= r->val.u_max)
			r->val.sp_u_val = r->val.u_max;
//		r->val.dac_u += 10;
//		if (r->val.dac_u >= 4095)
//			r->val.dac_u = 4095;
	}

	if (MENC_TurnLeft(&menc1)) {
		r->val.sp_u_val -= 10;
		if (r->val.sp_u_val < r->val.u_min)
			r->val.sp_u_val = r->val.u_min;
//		r->val.dac_u -= 10;
//		if (r->val.dac_u < 0)
//			r->val.dac_u = 0;
	}

	//current trim
	if (MENC_TurnRight(&menc2)) {
		r->val.sp_i_val += 10;
		if (r->val.sp_i_val >= r->val.i_max)
			r->val.sp_i_val = r->val.i_max;
//		r->val.dac_i += 10;
//		if (r->val.dac_i >= 4095)
//			r->val.dac_i = 4095;
	}

	if (MENC_TurnLeft(&menc2)) {
		r->val.sp_i_val -= 10;
		if (r->val.sp_i_val < r->val.i_min)
			r->val.sp_i_val = r->val.i_min;
//		r->val.dac_i -= 10;
//		if (r->val.dac_i < 0)
//			r->val.dac_i = 0;
	}

	//if any turn
	if (MENC_AnyTurn(&menc1)) {
		if (r->fl.tl494_on) {
			RPS_Ctrl_U_SPReach(r->val.sp_u_val, r);
			//PERIF_DAC_SET(r->val.dac_u, DAC_VOLT_CH);
		}
	}
	if (MENC_AnyTurn(&menc2)) {
		if (r->fl.tl494_on) {
			RPS_Ctrl_I_SPReach(r->val.sp_i_val, r);
			//PERIF_DAC_SET(r->val.dac_i, DAC_CURR_CH);
		}
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
 * @param[in/out] *r -> project structure pointer
 */
void HMI_Display_MeasPage(rps_type *r) {
	static uint16_t volt_old, curr_old, watt_old;
	static uint16_t sp_u_old, sp_i_old, dac_u_old, dac_i_old;
	int16_t diff = 0; ///<differance between new and old values

	//voltage
	diff = volt_old - r->val.volt;
	if (diff < 0)
		diff *= -1; //no matter is old value bigger or smaller than a new one
	if (diff > r->fl.tl494_on ? 1 : 0) { //eliminate fluctuation of the value
		MGL_SET_BUF_COLOR(VOLT_COLOR);
		MGL_PrintFloatTinyR(r->val.volt, 4, 2, VAW_VOLTAGE_X, VAW_VOLTAGE_Y, FONT_17x24_FP);
		MGL_DrawBar(&volt_bar);
	}

	//current
	diff = curr_old - r->val.curr;
	if (diff < 0)
		diff *= -1;
	if (diff > r->fl.tl494_on ? 1 : 0) {
		MGL_SET_BUF_COLOR(CURR_COLOR);
		MGL_PrintFloatTinyR(r->val.curr, 4, 3, VAW_CURRENT_X, VAW_CURRENT_Y, FONT_17x24_FP);
		MGL_DrawBar(&curr_bar);
	}

	//wattage
	diff = watt_old - r->val.watt;
	if (diff < 0)
		diff *= -1;
	if (diff > r->fl.tl494_on ? 1 : 0) {
		MGL_SET_BUF_COLOR(WATT_COLOR);
		MGL_PrintFloatTinyR(r->val.watt, 4, 1, VAW_WATTAGE_X, VAW_WATTAGE_Y, FONT_17x24_FP);
		MGL_DrawBar(&watt_bar);
	}

	MGL_SET_BUF_COLOR(FONT_COLOR);

	//voltage DAC value
	diff = dac_u_old - r->val.dac_u;
	if (diff < 0)
		diff *= -1;
	if (diff > 0) {
		MGL_PrintUintR(r->val.dac_u, 4, 45, LOW_INF_BAR_UPP_Y, FONT_5x8_FP);
		MGL_DrawBar(&watt_bar);
	}

	//current dac value
	diff = dac_i_old - r->val.dac_i;
	if (diff < 0)
		diff *= -1;
	if (diff > 0) {
		MGL_PrintUintR(r->val.dac_i, 4, 45, LOW_INF_BAR_LOW_Y, FONT_5x8_FP);
	}

	//voltage set point
	diff = sp_u_old - r->val.sp_u_val;
	if (diff < 0)
		diff *= -1;
	if (diff > 0) {
		MGL_PrintFloatTinyR(r->val.sp_u_val, 4, 2, 120, LOW_INF_BAR_UPP_Y, FONT_5x8_FP);
	}

	//current set point
	diff = sp_i_old - r->val.sp_i_val;
	if (diff < 0)
		diff *= -1;
	if (diff > 0) {
		MGL_PrintFloatTinyR(r->val.sp_i_val, 4, 3, 120, LOW_INF_BAR_LOW_Y, FONT_5x8_FP);
	}

	volt_old = r->val.volt;
	curr_old = r->val.curr;
	watt_old = r->val.watt;
	sp_u_old = r->val.sp_u_val;
	sp_i_old = r->val.sp_i_val;
	dac_u_old = r->val.dac_u;
	dac_i_old = r->val.dac_i;
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to read INA226 register and convert to uint16 fake float
 * @param[in/out] *r -> project structure pointer
 */
void RPS_VAW_Conversion(rps_type *r) {

	//static uint16_t med_fil_volt_buf[3],med_fil_curr_buf[3];

	r->val.volt = INA_GetBusVoltageTiny();
	//FilterMedian(&r->val.volt, med_fil_volt_buf);

	r->val.curr = INA_GetCurrentTiny();
	//FilterMedian(&r->val.curr, med_fil_curr_buf);

	r->val.watt = INA_GetPowerTiny();

}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to make DAC to voltage related table\
 * use it only after RPS_Save_TableInit()
 * @param[in/out] *r -> project structure pointer
 */
void RPS_Save_FBTableVolt(rps_type *r) {
	uint16_t val = 0; ///<buffer
	uint32_t timeout_cnt = 0; ///<against infinite while
	uint64_t *buf64_ptr; ///<HAL_FLASH_Program buffer pointer
	uint16_t buf16[4]; ///<16 bit buffer to assemble 64 bit data then
	uint8_t buf16_cnt = 0; ///<counter to make 64 bit variable from 4 16 bit
	uint16_t addr_cnt = 0; ///<flash address shifter

#ifdef USE_DEBUG
	printf("Starting voltage calibration\n");
#endif

	PERIF_DAC_SET(4095, DAC_CURR_CH); //get current on maximum to eliminate affect from it
	PERIF_TL494_ON();
	HAL_FLASH_Unlock();

	//increase DAC value +100 until max and put voltage feedback and appropriate DAC values in the arrays
	for (uint8_t i = 0; i < RPS_TABLE_SIZE; i++) {
		val = *(table_dac_step + i);
		PERIF_DAC_SET(val, DAC_VOLT_CH); //voltage increasing
		HAL_Delay(RPS_TABLE_DELAY); //wait until capacitor is charged
		r->val.dac_u = val;
		RPS_VAW_Conversion(r);
		//*(table_volt_fb + i) = r->val.volt;

#ifdef USE_DEBUG
		printf("Volt:%d   ", r->val.volt);
		printf("DAC: %d\n", val);
#endif
		//save to flash
		*(buf16 + buf16_cnt) = r->val.volt; //write data one by one in 4 elements array 16 bit resolution
		//4*16bit array is full, time to push it into flash
		if (buf16_cnt >= 3 || i == RPS_TABLE_SIZE - 1) {
			buf16_cnt = 0;
			buf64_ptr = (uint64_t*) buf16; //pointer change
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, TABLE_VOLT_ADDR + 8U * addr_cnt, (uint64_t) *buf64_ptr) != 0) {
				r->err.bit.flash_write = 1;
				return;
			}
			addr_cnt++;
		} else {
			buf16_cnt++;
		}
		__NOP();
	}

	HAL_FLASH_Lock();

	//turn off everything
	PERIF_TL494_OFF();
	PERIF_DAC_SET(0, DAC_VOLT_CH);
	PERIF_DAC_SET(0, DAC_CURR_CH);
	r->val.dac_u = 0; //clear global structure variable

	//wait until capacitor is discharged
	while (r->val.volt != 0) {
		RPS_VAW_Conversion(r);
		HMI_Display_MeasPage(r);
		if (timeout_cnt >= RPS_TIMEOUT_THRESHOLD) {
			r->err.bit.cicle_timeout = 1;
			return;
		}
		timeout_cnt++;
	}

#ifdef USE_DEBUG
	printf("Finish voltage calibration\n");
#endif
}

///////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to make DAC to current related table\
 * use it only after RPS_Save_TableInit()
 * @param[in/out] *r -> project structure pointer
 */
void RPS_Save_FBTableCurr(rps_type *r) {
	uint16_t val = 0;
	uint32_t timeout_cnt = 0; ///<against infinite while
	uint64_t *buf64_ptr; ///<HAL_FLASH_Program buffer pointer
	uint16_t buf16[4]; ///<16 bit buffer to assemble 64 bit data then
	uint8_t buf16_cnt = 0; ///<counter to make 64 bit variable from 4 16 bit
	uint16_t addr_cnt = 0; ///<flash address shifter

#ifdef USE_DEBUG
	printf("Starting current calibration\n");
#endif

	PERIF_DAC_SET(4095, DAC_VOLT_CH); //get voltage on maximum to eliminate affect from it
	PERIF_TL494_ON();

	HAL_FLASH_Unlock();

	//increase DAC value +100 until max and put current feedback and appropriate DAC values in the arrays
	for (uint8_t i = 0; i < RPS_TABLE_SIZE; i++) {
		val = *(table_dac_step + i);
		PERIF_DAC_SET(val, DAC_CURR_CH); //voltage increasing
		HAL_Delay(RPS_TABLE_DELAY); //wait until output capacitor is charged
		r->val.dac_i = val;
		RPS_VAW_Conversion(r);
		//*(table_curr_fb + i) = r->val.curr;

#ifdef USE_DEBUG
		printf("Curr:%d   ", r->val.curr);
		printf("DAC: %d\n", val);
#endif

		//save to flash
		*(buf16 + buf16_cnt) = r->val.curr; //write data one by one in 4 elements array 16 bit resolution
		//4*16bit array is full, time to push it into flash
		if (buf16_cnt >= 3 || i == RPS_TABLE_SIZE - 1) {
			buf16_cnt = 0;
			buf64_ptr = (uint64_t*) buf16; //pointer change
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, TABLE_CURR_ADDR + 8U * addr_cnt, (uint64_t) *buf64_ptr) != 0) {
				r->err.bit.flash_write = 1;
				return;
			}
			addr_cnt++;
		} else {
			buf16_cnt++;
		}
	}

	HAL_FLASH_Lock();

	//turn off everything
	PERIF_TL494_OFF();
	PERIF_DAC_SET(0, DAC_VOLT_CH);
	PERIF_DAC_SET(0, DAC_CURR_CH);
	r->val.dac_i = 0; //clear global structure variable

	while (r->val.volt != 0) {
		RPS_VAW_Conversion(r);
		HMI_Display_MeasPage(r);
		if (timeout_cnt >= RPS_TIMEOUT_THRESHOLD) {
			r->err.bit.cicle_timeout = 1;
			return;
		}
		timeout_cnt++;
	}

#ifdef USE_DEBUG
	printf("Finish current calibration\n");
#endif
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to save both tables\
 * there is no possibility to save only voltage or current
 * before each saving page erasing required
 * @param[in/out] *r -> project structure pointer
 */
void RPS_Save_Table(rps_type *r) {
	SERV_Flash_EraseTable(r); //clear last page in FLASH

	//filling tables
	RPS_Save_FBTableVolt(r);
	volt_bar.num_max = r->val.u_max; //renew maximum value after calibration

	RPS_Save_FBTableCurr(r);
	curr_bar.num_max = r->val.i_max; //renew maximum value after calibration
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to fill table_dac_step array
 * max and min values of current and voltage
 * saved previously in FLASH
 * it's required for voltage/current control functions
 */
void RPS_Save_TableInit(rps_type *r) {
	uint16_t val;
	uint16_t buf_curr = 0; //buffer
	uint16_t debug_buf;

	for (uint16_t i = 0; i < RPS_TABLE_SIZE; i++) {
		val = i * 100;
		if (val > 4095)
			val = 4095;
		*(table_dac_step + i) = val;
	}
	//min and max voltage
	r->val.dac_u = 0;
	r->val.u_min = *(table_ptr_u + 0);
	r->val.u_max = *(table_ptr_u + RPS_TABLE_SIZE - 1);

	//min and max voltage
	r->val.i_min = *(table_ptr_i + 0);

	//just maximum current in the table
	for (uint8_t i = 0; i < RPS_TABLE_SIZE; i++) {
		debug_buf = *(table_ptr_i + i);
		if (buf_curr > debug_buf) {
			r->val.i_max = buf_curr;
			break;
		}
		buf_curr = *(table_ptr_i + i);
		__NOP();

//		//compare voltage from the feedback voltage table and a set point.
//		//to find a table pointer with closest to SP voltage but smaller than it
//		if (*(table_ptr_i + i) > set_point) {
//			r->val.dac_i = *(table_dac_step + i - 1);
//			//here must be some protection from pointer miss read. But my first value in the tables are 0
//			//nothing is smaller than 0 in uint16_t
//			break;
//		}

	}
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to print saved in Flash tables
 */
void RPS_Save_PrintSavedTables(void) {
	printf("Print saved in flash tables");
	for (uint16_t i = 0; i < RPS_TABLE_SIZE; i++) {
		printf("DAC value: %u", *(table_dac_step + i));
		printf("Saved voltage: %u", *(table_ptr_u + i));
		printf("Saved current: %u \n", *(table_ptr_i + i));
	}
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to calculate DAC steps using tables in flash
 * DAC +1 -> voltage +?\
 * DAC +10 -> current +?
 */
void RPS_Save_CalculateDACSteps(rps_type *r) {
//	uint16_t aver_buf[4]; ///<buffer to make averaging  of 3 variables
//	uint16_t last_aver; ///<buffer to hold last averaged variable
//	uint16_t len = RPS_TABLE_SIZE;
//	uint8_t resid; ///<residual after quantity of 16 bit array divided 4
//
//	//if 16 bit array can't be equally divided by 4
//	resid = len % 3;
//	if (resid > 0) {
//		len -= resid;
//		len += 3;
//	}
//	len >>= 3; //len /= 4;
//
//	//find middle change during +100 DAC step
//	for (uint8_t i = 0; i < len; i++) {
//	aver_buf=&(*table_ptr_u + 4U * len);

	uint16_t aver_buf[3];
	uint16_t buf, i=0;

	for (buf = r->val.u_min; buf <= r->val.u_max; i++) {
		*(aver_buf+1)
	}
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to reach voltage by setting a set point
 * @param[in] sp_val-> set point
 * @param[in/out] *r -> structure pointer
 */
void RPS_Ctrl_U_SPReach(uint16_t set_point, rps_type *r) {
	if (set_point == 0) {
		r->val.dac_u = 0; //mix
	} else if (set_point == r->val.u_max) {
		r->val.dac_u = 4095; //max
	} else {
		for (uint8_t i = 0; i < RPS_TABLE_SIZE; i++) {
			//compare voltage from the feedback voltage table and a set point.
			//to find a table pointer with closest to SP voltage but smaller than it
			if (*(table_ptr_u + i) > set_point) {
				r->val.dac_u = *(table_dac_step + i - 1); //use this shift in the DAC values table
				//here must be some protection from pointer miss read. But my first value in the tables are 0
				//nothing is smaller than 0 in uint16_t
				break;
			}
		}
	}
	PERIF_DAC_SET(r->val.dac_u, DAC_VOLT_CH);
}

/////////////////////////////////////////////////////////////////////////
/*
 * @brief Function to reach current by setting a set point
 * @param[in] sp_val-> set point
 * @param[in/out] *r -> project structure pointer
 */
void RPS_Ctrl_I_SPReach(uint16_t set_point, rps_type *r) {
	if (set_point == 0) {
		r->val.dac_i = 0; //min
	} else if (set_point == r->val.i_max) {
		r->val.dac_i = 4095; //max
	} else {
		for (uint8_t i = 0; i < RPS_TABLE_SIZE; i++) {
			//compare voltage from the feedback voltage table and a set point.
			//to find a table pointer with closest to SP voltage but smaller than it
			if (*(table_ptr_i + i) > set_point) {
				r->val.dac_i = *(table_dac_step + i - 1);
				//here must be some protection from pointer miss read. But my first value in the tables are 0
				//nothing is smaller than 0 in uint16_t
				break;
			}
		}
	}
	PERIF_DAC_SET(r->val.dac_i, DAC_CURR_CH);
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
