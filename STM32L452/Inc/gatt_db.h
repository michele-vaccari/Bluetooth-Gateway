
#ifndef _GATT_DB_H_
#define _GATT_DB_H_

#include <stdint.h>
#include <stdlib.h>
#include "bluenrg_def.h"

#define X_OFFSET 200
#define Y_OFFSET 50
#define Z_OFFSET 1000

/** 
 * @brief Number of application services
 */
#define NUMBER_OF_APPLICATION_SERVICES (2)
   
/**
 * @brief Define How Many quaterions you want to trasmit (from 1 to 3)
 *        In this sample application use only 1
 */
#define SEND_N_QUATERNIONS 1
   
/** 
 * @brief Structure containing acceleration value of each axis.
 */
typedef struct {
  int32_t AXIS_X;
  int32_t AXIS_Y;
  int32_t AXIS_Z;
} AxesRaw_t;

enum {
  ACCELERATION_SERVICE_INDEX = 0,  
  ENVIRONMENTAL_SERVICE_INDEX = 1   
};

/** Documentation for C union Service_UUID_t */
typedef union Service_UUID_t_s {
  /** 16-bit UUID 
  */
  uint16_t Service_UUID_16;
  /** 128-bit UUID
  */
  uint8_t Service_UUID_128[16];
} Service_UUID_t;

/** Documentation for C union Char_UUID_t */
typedef union Char_UUID_t_s {
  /** 16-bit UUID 
  */
  uint16_t Char_UUID_16;
  /** 128-bit UUID
  */
  uint8_t Char_UUID_128[16];
} Char_UUID_t;

tBleStatus Add_HWServW2ST_Service(void);
tBleStatus Add_SWServW2ST_Service(void);
void Read_Request_CB(uint16_t handle);
tBleStatus BlueMS_Environmental_Update(int32_t press, int16_t temp);

#endif /* _GATT_DB_H_ */
