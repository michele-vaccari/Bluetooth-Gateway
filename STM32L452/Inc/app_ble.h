// Define to prevent recursive inclusion
#ifndef __APP_X_CUBE_BLE1_H
#define __APP_X_CUBE_BLE1_H
#ifdef __cplusplus
 extern "C" {
#endif

// Includes
#include "stm32l4xx_hal.h"

// Exported Functions
void SendsTemperatureAndHumidity_Init(I2C_HandleTypeDef *hi2c);
void SendsTemperatureAndHumidity_Process(void);

#ifdef __cplusplus
}
#endif
#endif
