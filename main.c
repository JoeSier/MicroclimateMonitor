
/** 
 * @file main.c
 * @brief This file contains functions to read sensor data and control the display.
 * 
 * This file contains the main functionality for reading sensor data and controlling 
 * the display on the STM32F746G-Discovery microcontroller.
 * 
 * Utilizes an interrupt with the user button on the back of the board to pause sensor reading, 
 * the user can pause and resume the reading instead of it being active all the time.
 *
 * @author Joseph Sier
 */

#include "Board_GLCD.h"
#include "GLCD_Config.h"
#include "stm32f7xx_hal.h"
#include "Board_Touch.h"
#include "GLCD.h"
#include "main.h"

// A0 - PA0 - Temperature and Humidity
// A1 - PF10 - Gas sensor
// A2 - PF9 - Moisture sensor
// A3 - PF8 - photoresistor

//Function declarations
static void MX_GPIO_Init(void);
void SystemClock_Config(void);
static void MX_ADC_Init(void);
void init_config(void);


extern GLCD_FONT GLCD_Font_16x24; /**< External declaration of GLCD font**/
GPIO_InitTypeDef GPIO_InitStruct; /**< GPIO initialization structure**/
ADC_HandleTypeDef hadc1; /**< ADC handle 1 Moisture **/
ADC_HandleTypeDef hadc2; /**< ADC handle 2 MQ135 **/
ADC_HandleTypeDef hadc3; /**< ADC handle 3 LDR **/
/** Buffer for storing data **/
char buffer[128];
bool userTrigger=true; /**< Flag for user trigger **/


/** 
 * @brief Interrupt handler for EXTI15_10.
 */
void EXTI15_10_IRQHandler(void)
{
    // Clear interrupt flag 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
}


/** 
 * @brief Callback function for EXTI interrupt.
 * 
 * @param GPIO_Pin The pin number triggering the interrupt.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_11) {
			userTrigger=!userTrigger;
    }
}




/**
 * @brief Main function.
 */
int main(void){
    // Initialize system config
    init_config();
    
    for (;;) {
        GLCDLogic(); // Function in GLCD that handles all displayed data
    }
}



/** 
 * @brief GPIO Initialization. 
 * 
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();

    // Analog sensors
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    
    // DHT sensor
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // User Button
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; 
    GPIO_InitStruct.Pull = GPIO_NOPULL; 
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
    
    // Interrupt for User Button
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0); 
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 
}


#ifdef __RTX
extern uint32_t os_time;
uint32_t HAL_GetTick(void) {
	return os_time;
}
#endif

/** 
 * @brief Configure the system clock.
 */

void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();
	/* The voltage scaling allows optimizing the power
	consumption when the device is clocked below the
	maximum system frequency. */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/* Enable HSE Oscillator and activate PLL
	with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);
	/* Select PLL as system clock source and configure
	the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | 
	RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/** 
 * @brief ADC initialization.
 */
static void MX_ADC_Init(void)
{

		__HAL_RCC_ADC3_CLK_ENABLE();

		//moisture sensor setup
	
		hadc1.Instance = ADC3; //# Select the ADC (ADC1, ADC2, ADC3)
		hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
		hadc1.Init.Resolution = ADC_RESOLUTION_10B;
		hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
		hadc1.Init.NbrOfConversion = 1;
		hadc1.Init.ScanConvMode = ENABLE;
		hadc1.Init.ContinuousConvMode = ENABLE;
		hadc1.Init.DiscontinuousConvMode = DISABLE;
		HAL_ADC_Init(&hadc1);  

		// mq135 setup

		hadc2.Instance = ADC3; //# Select the ADC (ADC1, ADC2, ADC3)
		hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
		hadc2.Init.Resolution = ADC_RESOLUTION_10B;
		hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
		hadc2.Init.NbrOfConversion = 1;
		hadc2.Init.ScanConvMode = ENABLE;
		hadc2.Init.ContinuousConvMode = ENABLE;
		hadc2.Init.DiscontinuousConvMode = DISABLE;
		HAL_ADC_Init(&hadc2);  	
		
		// LDR setup

		hadc3.Instance = ADC3; //# Select the ADC (ADC1, ADC2, ADC3)
		hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
		hadc3.Init.Resolution = ADC_RESOLUTION_10B;
		hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
		hadc3.Init.NbrOfConversion = 1;
		hadc3.Init.ScanConvMode = ENABLE;
		hadc3.Init.ContinuousConvMode = ENABLE;
		hadc3.Init.DiscontinuousConvMode = DISABLE;
		HAL_ADC_Init(&hadc3);  	
		
}




/** 
 * @brief All configurations to be run on start.
 */
void init_config(void){
    HAL_Init(); // Initialize HAL
    SystemClock_Config(); // Configure system clock
    MX_GPIO_Init(); // Initialize GPIO
    MX_ADC_Init(); // Initialize ADC
GLCD_Initialize();// Initialize GLCD
	Touch_Initialize(); // Initialize Touch
    GLCD_ClearScreen();
    GLCD_SetFont(&GLCD_Font_16x24);
    GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
    GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	
}










