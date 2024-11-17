/**
 * @file GLCD.c
 * @brief Implementation of GLCD functions
 * @author Joseph Sier
 *
 * I opted to keep everything GLCD related inside GLCD.c.
 *
 * This displays every page, as well as the data from sensors. Preset parameteres are loosely based off the Koppen climate classification.
 *
 */


#include "Board_GLCD.h"
#include "Board_Touch.h"
#include "GLCD_Config.h"
#include "main.h"
#include <string.h>
#include "Moisture.h"
#include "mq135.h"
#include "DHT.h"
#include "LDR.h"

// Global variables

TOUCH_STATE tsc_state; /**< State of touch input, containing coordinates and if pressed**/
extern GLCD_FONT GLCD_Font_6x8; /**< External declaration of GLCD font**/
extern GLCD_FONT GLCD_Font_16x24; /**< External declaration of GLCD font**/
extern bool userTrigger;

bool dataTransition=true; /**< Flag indicating transition in data display. */
bool drawBars=true; /**< Flag indicating whether to draw bars on the display. */
bool initialPage=true; /**< FFlag indicating whether to show initial page on the display. */
bool readyTransition=true; /**< Flag indicating transition in ready display. */
bool settingsTransition=true; /**< Flag indicating transition in settings display. */
bool showData=false; /**< Flag indicating whether to show data on the display. */
bool showDataPage=false; /**< Flag indicating whether to show data page on the display. */
bool showSettingsPage=false; /**< Flag indicating whether to show settings page on the display. */

 
char preset[9]; /**< Array storing preset values. */
uint8_t Temperature; /**< Current temperature value. */
uint8_t Humidity; /**< Current humidity value. */
int8_t soilMoisturePercent; /**< Current soil moisture percentage. */
uint16_t ldr_val; /**< Current LDR (Light Dependent Resistor) value. */
uint32_t k; /**< Count value for LDR. */

LDR_DataTypedef LDR_Data;
DHT_DataTypedef DHT11_Data;
Moisture_DataTypedef Moisture_Data;
MQ135_DataTypedef MQ135_Data;

// Function prototypes

void displayMoisture(void);
void getTimeActive(void);
void displayMQ135(void);
void displayDHT(void);
void displayLDR(void);
void settingsPage(void);

// Functions

/*
Based loosely off Koppen climate classification

Desert:
hot desert climate
25-40 degrees C
Moisture: 0%
Humidity: 0%
 
 Temperate:
subtropical highland
10-25 C
Moisture: 30%
Humidity: 30%

Tropical:
tropical rainforest climate
20-35 C
60% moisture
70% humidity

*/

/**
 * @brief Display status message at specified location
 * @param x X-coordinate of the message
 * @param y Y-coordinate of the message
 * @param status Status message to display
 */
 
void displayStatus(int x, int y,
  const char * status) {
  GLCD_DrawString(x, y, status);
}
/**
 * @brief Check value against thresholds and display corresponding status message
 * @param highThreshold High threshold value
 * @param lowThreshold Low threshold value
 * @param value Value to compare against thresholds
 * @param x X-coordinate of the status message
 * @param y Y-coordinate of the status message
 * @param highMsg Message to display if value exceeds high threshold
 * @param lowMsg Message to display if value falls below low threshold
 * @param goodMsg Message to display if value is within thresholds
 */
void checkAndDisplayStatus(int highthreshold, int lowThreshold, int value, int x, int y,
  const char * highMsg,
    const char * lowMsg,
      const char * goodMsg) {
  if (value > highthreshold) {
    displayStatus(x, y, highMsg);
  } else if (value < lowThreshold) {
    displayStatus(x, y, lowMsg);
  } else {
    displayStatus(x, y, goodMsg);
  }
}
/**
 * @brief Display data page on the GLCD
 */
void dataPage() {
  if (dataTransition) {
    GLCD_ClearScreen();
    drawBars = true;
    dataTransition = false;
    GLCD_DrawRectangle(60, 50, 125, 75); // box 1 - DHT
    GLCD_DrawRectangle(60, 150, 125, 75); // box 2 - Gas
    GLCD_DrawRectangle(290, 50, 125, 75); // box 3 - Moisture
    GLCD_DrawRectangle(290, 150, 125, 75); // box 4 - LDR
  }
  if (userTrigger) { //Display all sensors on screen if not paused
    displayMQ135();
    displayMoisture();
    displayDHT();
  }
  displayLDR();
  HAL_Delay(1000);
  GLCD_DrawString(75, 90, "                 ");
  GLCD_DrawString(305, 190, "                 ");
  GLCD_DrawString(305, 90, "                ");
  if (strncmp(preset, "temperate", 9) == 0) { // Code to execute if preset is "temperate"
    GLCD_DrawString(0, 25, "Chosen preset: Temperate");
    checkAndDisplayStatus(30, 10, Temperature, 75, 90, "Too Hot!", "Too Cold!", "Good temperature");
    checkAndDisplayStatus(50, 20, Humidity, 305, 190, "Too Humid!", "Not Humid Enough!", "Good Humidity");
    checkAndDisplayStatus(30, 20, soilMoisturePercent, 305, 90, "Too Much Water!", "Not Enough Water!", "Good Moisture!");

  } else if (strncmp(preset, "tropical", 8) == 0) { // Code to execute if preset is "tropical"
    GLCD_DrawString(0, 25, "Chosen preset: Tropical");
    checkAndDisplayStatus(35, 20, Temperature, 75, 90, "Too Hot!", "Too Cold!", "Good temperature");
    checkAndDisplayStatus(80, 40, Humidity, 305, 190, "Too Humid!", "Not Humid Enough!", "Good Humidity");
    checkAndDisplayStatus(80, 50, soilMoisturePercent, 305, 90, "Too Much Water!", "Not Enough Water!", "Good Moisture!");

  } else if (strncmp(preset, "desert", 6) == 0) { // Code to execute if preset is "desert"
    GLCD_DrawString(0, 25, "Chosen preset: Desert");
    checkAndDisplayStatus(45, 20, Temperature, 75, 90, "Too Hot!", "Too Cold!", "Good temperature");
    checkAndDisplayStatus(40, 5, Humidity, 305, 190, "Too Humid!", "Not Humid Enough!", "Good Humidity");
    checkAndDisplayStatus(20, 0, soilMoisturePercent, 305, 90, "Too Much Water!", "Not Enough Water!", "Good Moisture!");

  } else {
    GLCD_DrawString(0, 25, "Chosen preset: None");

  }
}
/**
 * @brief Display ready page on the GLCD
 */
void readyPage(void) {
  if (readyTransition) {
    GLCD_SetFont( & GLCD_Font_16x24);
    GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
    GLCD_DrawString(150, 100, "Warming up");
    GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
    GLCD_DrawString(200, 100, "Ready");
    GLCD_DrawRectangle(180, 80, 120, 60);
    GLCD_SetFont( & GLCD_Font_6x8);
    readyTransition = false;
  }
}
/**
 * @brief Display all page elements on the GLCD
 */
void allPage(void) {

  if (drawBars == true) {
    drawBars = false;
    GLCD_SetFont( & GLCD_Font_16x24);
    GLCD_SetForegroundColor(GLCD_COLOR_GREEN);
    GLCD_SetBackgroundColor(GLCD_COLOR_GREEN);
    GLCD_DrawString(0, 0, "                                 "); //top bar
    GLCD_DrawString(0, 247, "                                 "); // bottom bar
    GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
    GLCD_DrawString(75, 0, "Microclimate Monitor");
    GLCD_SetFont( & GLCD_Font_6x8);
  }
  if (userTrigger) {
    GLCD_SetBackgroundColor(GLCD_COLOR_GREEN);
    GLCD_SetForegroundColor(GLCD_COLOR_GREEN);
    GLCD_DrawString(250, 250, "Paused");
  } else {

    GLCD_SetBackgroundColor(GLCD_COLOR_GREEN);
    GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
    GLCD_DrawString(250, 250, "Paused");
  }
  GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
  GLCD_DrawString(400, 255, "Settings");
  GLCD_DrawString(5, 5, "reset");
  getTimeActive();
  GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
  if (tsc_state.pressed && tsc_state.x >= 0 && tsc_state.x <= 30 && tsc_state.y >= 0 && tsc_state.y <= 30) {
    HAL_NVIC_SystemReset();
  }

}
/**
 * @brief Display warm up page on the GLCD
 */
void warmUp(void) {
  GLCD_SetFont( & GLCD_Font_16x24);
  GLCD_DrawString(150, 100, "Warming up");
  GLCD_SetFont( & GLCD_Font_6x8);
}

/**
 * @brief Display system uptime in hours, minutes, and seconds on the GLCD
 */
void getTimeActive(void) {
  uint32_t tick = HAL_GetTick();
  uint32_t elapsed_s = tick / 1000;
  uint32_t hours = elapsed_s / (60 * 60);
  uint32_t minutes = (elapsed_s / 60) % 60;
  uint32_t seconds = elapsed_s % 60;
  sprintf(buffer, "Time active: %i h %02d m %02d s", hours, minutes, seconds);
  GLCD_DrawString(20, 250, buffer);
  sprintf(buffer, "Total time active: %i ms", tick);
  GLCD_DrawString(20, 260, buffer);
}



/**
 * @brief GLCDLogic function.
 *
 * This function is responsible for the logic to control the GLCD.
 * It provides the necessary functionality to interact with the GLCD hardware.
 */
void GLCDLogic() {
  uint32_t tick = HAL_GetTick();
  uint32_t elapsed_s = tick / 1000;
  allPage();
  Touch_GetState( & tsc_state);
  //if (tsc_state.pressed) { // for finding touch location easily
  //            sprintf(buffer, "  %i %i  ", 
  //							tsc_state.x, tsc_state.y);
  //            GLCD_DrawString(0, 5 * 24, buffer);
  //        } 
  if (initialPage) {
    if (elapsed_s <= 10) {
      warmUp();
    } else if (tsc_state.pressed && tsc_state.x >= 195 && tsc_state.x <= 305 && tsc_state.y >= 90 && tsc_state.y <= 140) {
      readyTransition = false;
      showDataPage = true;
      initialPage = false;
      settingsTransition = true;
    } else if (elapsed_s >= 10) {
      readyPage();
    }
  }
  if (tsc_state.pressed && tsc_state.x >= 400 && tsc_state.x <= 500 && tsc_state.y >= 230 && tsc_state.y <= 270) {
    readyTransition = false;
    showDataPage = false;
    initialPage = false;
    showSettingsPage = true;
    settingsTransition = true;
  }
  if (showDataPage) {
    dataPage();
  }
  if (showSettingsPage) {
    settingsPage();
  }
}
/**
 * @brief Display settings page on the GLCD
 */
void settingsPage() {
  GLCD_SetFont( & GLCD_Font_16x24);
  if (settingsTransition) {
    GLCD_ClearScreen();
    drawBars = true;
    settingsTransition = false;
    GLCD_DrawString(20, 30, "Settings");
    GLCD_DrawString(0, 125, "<Data");
    GLCD_DrawString(240, 30, "Choose Preset");
  }
  GLCD_DrawString(240, 60, "<  Tropical >");
  GLCD_DrawString(240, 100, "< Temperate >");
  GLCD_DrawString(240, 140, "<   Desert  >");
  GLCD_SetFont( & GLCD_Font_6x8);
  if (tsc_state.pressed && tsc_state.x >= 250 && tsc_state.x <= 450 && tsc_state.y >= 60 && tsc_state.y <= 90) {
    strcpy(preset, "tropical");

    GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
    GLCD_DrawString(240, 200, "Current preset:           ");
  }
  if (tsc_state.pressed && tsc_state.x >= 250 && tsc_state.x <= 450 && tsc_state.y >= 100 && tsc_state.y <= 130) {
    strcpy(preset, "temperate");
    GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
    GLCD_DrawString(240, 200, "Current preset:           ");
  }
  if (tsc_state.pressed && tsc_state.x >= 250 && tsc_state.x <= 450 && tsc_state.y >= 140 && tsc_state.y <= 170) {
    strcpy(preset, "desert");
    GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
    GLCD_DrawString(240, 200, "Current preset:           ");
  }

  if (tsc_state.pressed && tsc_state.x >= 0 && tsc_state.x <= 100 && tsc_state.y >= 110 && tsc_state.y <= 160) {
    showDataPage = true;
    showSettingsPage = false;
    dataTransition = true;
  }
  sprintf(buffer, "Current preset: %s", preset); // Format temperature value
  GLCD_DrawString(240, 200, buffer); // Display temperature on GLCD
}

/**
 * @brief Display temperature and humidity data on the GLCD
 */
void displayDHT() {
  uint32_t tick = HAL_GetTick();
  uint32_t elapsed_s = tick / 1000;
  sprintf(buffer, "Temperature: %i C", Temperature);
  GLCD_DrawString(75, 70, buffer);
  sprintf(buffer, "Humidity: %i %%", Humidity);
  GLCD_DrawString(305, 170, buffer);
  if ((elapsed_s % 5) == 0) { //must be at least 3000ms apart
    DHT_GetData( & DHT11_Data);
    Temperature = DHT11_Data.Temperature;
    Humidity = DHT11_Data.Humidity;
  }
}

/**
 * @brief Display MQ135 data (CO2 concentration) on the GLCD
 */
void displayMQ135() {
  uint16_t MQ135PPM;
  uint32_t tick = HAL_GetTick();
  uint32_t elapsed_s = tick / 1000;
  if ((elapsed_s % 4) == 0) {
    MQ_GetData( & MQ135_Data);
    MQ135PPM = MQ135_Data.PPM;
    sprintf(buffer, "CO2: %i PPM", MQ135PPM);
    GLCD_DrawString(75, 170, buffer);
  }
}

/**
 * @brief Display soil moisture data on the GLCD
 */
void displayMoisture(void) {
  Moisture_GetData( & Moisture_Data);
  soilMoisturePercent = Moisture_Data.MoisturePercent;

  if (soilMoisturePercent < 0) {
    // if percentages enter the negative, display 0
    sprintf(buffer, "Moisture: %i %%", soilMoisturePercent);
    GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
    GLCD_DrawString(305, 70, buffer);
    sprintf(buffer, "Moisture: 0 %%");
  } else {
    sprintf(buffer, "Moisture: 0 %%");
    GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
    GLCD_DrawString(305, 70, buffer);
    sprintf(buffer, "Moisture: %i %%", soilMoisturePercent);
  }

  GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
  GLCD_DrawString(305, 70, buffer); // Display Moisture as a %
}
/**
 * @brief When the LDR detects light, it starts a counter which is displayed on the GLCD
 */
void displayLDR(void) {
  LDR_GetData( & LDR_Data);
  ldr_val = LDR_Data.LDR_Val;

  if (ldr_val < 1023) {
    k++;
  }
  sprintf(buffer, "Time in sun: %i ", k); // Display time in sun
  GLCD_DrawString(75, 185, buffer);
}
