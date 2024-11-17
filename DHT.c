
/**
 * @file DHT.c
 * @author Joseph Sier
 * @brief This file contains functions to read data from the DHT11
 
 * DHT logic based on https://controllerstech.com/using-dht11-sensor-with-stm32/
 * The DHT11 uses serial communication, which is very susceptible to errors, as most of the delays need to be in
 * microseconds, which HAL does not support. I used a DWT delay to get a us (microsecond) delay.
 * 
 * The DHT sensor operates by initiating communication with a start pulse to request data, then it awaits and reads the response  pulse sent back by the DHT sensor. 
 * This response contains 40 bits of data which includes information about temperature and humidity. 
 * Upon reception, the data is processed and verified for integrity before being parsed and assigned to their respective temperature and humidity values.
 * @bug There is an issue with the timer in DHT11, and the code will only run from flash load.
 * when the device is disconnected and plugged in, it will hang on the DHT_Start().
 *
 *
 */
 
 #include "stm32f7xx_hal.h"
#include "main.h"
#include "DHT.h"
#define DHT_PORT GPIOA
#define DHT_PIN GPIO_PIN_0
uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM;
uint8_t Presence = 0;



/**
 * @brief Initializes the DWT (Data Watchpoint and Trace) unit for microsecond delay. used from
https://deepbluembedded.com/stm32-delay-microsecond-millisecond-utility-dwt-delay-timer-delay/ as HAL works in ms only.
 * @return 0 if initialization successful, 1 otherwise.
 */
uint32_t DWT_Delay_Init(void) {
  /* Disable TRC */
  CoreDebug -> DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
  /* Enable TRC */
  CoreDebug -> DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

  /* Disable clock cycle counter */
  DWT -> CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
  /* Enable  clock cycle counter */
  DWT -> CTRL |= DWT_CTRL_CYCCNTENA_Msk; //0x00000001;

  /* Reset the clock cycle counter value */
  DWT -> CYCCNT = 0;

  /* 3 NO OPERATION instructions */
  __ASM volatile("NOP");
  __ASM volatile("NOP");
  __ASM volatile("NOP");

  /* Check if clock cycle counter has started */
  if (DWT -> CYCCNT) {
    return 0; /*clock cycle counter started*/
  } else {
    return 1; /*clock cycle counter not started*/
  }
}

/**
 * @brief Provides delay in microseconds using DWT.
 * @param au32_microseconds Delay duration in microseconds.
 */

__STATIC_INLINE void DWT_Delay_us(volatile uint32_t au32_microseconds) {
  uint32_t au32_initial_ticks = DWT -> CYCCNT;
  uint32_t au32_ticks = (HAL_RCC_GetHCLKFreq() / 1000000);
  au32_microseconds *= au32_ticks;
  while ((DWT -> CYCCNT - au32_initial_ticks) < au32_microseconds - au32_ticks);
}


/**
 * @brief Sets a GPIO pin as output.
 * @param GPIOx Pointer to GPIO port.
 * @param GPIO_Pin GPIO pin to set as output.
 */


void Set_Pin_Output(GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOx, & GPIO_InitStruct);
}

/**
 * @brief Sets a GPIO pin as input.
 * @param GPIOx Pointer to GPIO port.
 * @param GPIO_Pin GPIO pin to set as input.
 */

void Set_Pin_Input(GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOx, & GPIO_InitStruct);
}
/**
 * @brief Initiates communication with DHT sensor.
 *
 * This function initializes the communication sequence with the DHT sensor to request data. 
 */

void DHT_Start(void) {
  DWT_Delay_Init();
  Set_Pin_Output(DHT_PORT, DHT_PIN); // set the pin as output
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, 0); // pull the pin low
	DWT_Delay_us(18000); // wait 18ms

  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, 1); // pull the pin high
  DWT_Delay_us(20); // wait for 20us
  Set_Pin_Input(DHT_PORT, DHT_PIN); // set as input
}
/**
 * @brief Checks response from DHT sensor.
 * @return 1 if response received, -1 if response timeout.
 */
uint8_t DHT_Check_Response(void) {
  uint8_t Response = 0;
  DWT_Delay_us(40);
  if (!(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))) {
    DWT_Delay_us(80);
    if ((HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))) Response = 1;
    else Response = -1;
  }
  while ((HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))); // wait for the pin to go low

  return Response;
}
/**
 * @brief Reads data from DHT sensor.
 * @return Data read from sensor.
 */
uint8_t DHT_Read(void) {
  uint8_t i, j;
  for (j = 0; j < 8; j++) {
    while (!(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))); // wait for the pin to go high
    DWT_Delay_us(40); // wait for 40 us
    if (!(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))) // if the pin is low
    {
      i&= ~(1 << (7 - j)); // write 0
    } else i |= (1 << (7 - j)); // if the pin is high, write 1
    while ((HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))); // wait for the pin to go low
  }
  return i;
}

/**
 * @brief Get DHT data
 * 
 * This function retrieves DHT data and stores it in the provided 
 * DHT_DataTypedef structure.
 * 
 * @param DHT_Data: Pointer to the structure DHT_DataTypedef.
 */
void DHT_GetData(DHT_DataTypedef * DHT_Data) {
  DHT_Start();
  Presence = DHT_Check_Response();
  Rh_byte1 = DHT_Read();
  Rh_byte2 = DHT_Read();
  Temp_byte1 = DHT_Read();
  Temp_byte2 = DHT_Read();
  SUM = DHT_Read();

  if (SUM == (Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2)) {
    DHT_Data -> Temperature = Temp_byte1;
    DHT_Data -> Humidity = Rh_byte1;
  }
}
