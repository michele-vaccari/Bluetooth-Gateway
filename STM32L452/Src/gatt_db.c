#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bluenrg_def.h"
#include "gatt_db.h"
#include "bluenrg_conf.h"
#include "bluenrg_gatt_aci.h"

/** @brief Macro that stores Value into a buffer in Little Endian Format (2 bytes)*/
#define HOST_TO_LE_16(buf, val)    ( ((buf)[0] =  (uint8_t) (val)    ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8) ) )

/** @brief Macro that stores Value into a buffer in Little Endian Format (4 bytes) */
#define HOST_TO_LE_32(buf, val)    ( ((buf)[0] =  (uint8_t) (val)     ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8)  ) , \
                                   ((buf)[2] =  (uint8_t) (val>>16) ) , \
                                   ((buf)[3] =  (uint8_t) (val>>24) ) ) 

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

/* Hardware Characteristics Service */
#define COPY_HW_SENS_W2ST_SERVICE_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0x9a,0xb4,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_ENVIRONMENTAL_W2ST_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
/* Software Characteristics Service */
#define COPY_SW_SENS_W2ST_SERVICE_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x02,0x11,0xe1,0x9a,0xb4,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_QUATERNIONS_W2ST_CHAR_UUID(uuid_struct)   COPY_UUID_128(uuid_struct,0x00,0x00,0x01,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)

uint16_t HWServW2STHandle, EnvironmentalCharHandle, AccGyroMagCharHandle;
uint16_t SWServW2STHandle, QuaternionsCharHandle;

/* UUIDS */
Service_UUID_t service_uuid;
Char_UUID_t char_uuid;

extern uint16_t connection_handle;
 
/**
 * @brief  Add the 'HW' service
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus Add_HWServW2ST_Service(void)
{
  tBleStatus ret;
  uint8_t uuid[16];

  // Add_HWServW2ST_Service
  COPY_HW_SENS_W2ST_SERVICE_UUID(uuid);  
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_serv(UUID_TYPE_128, service_uuid.Service_UUID_128, PRIMARY_SERVICE, 
                          1+3*5, &HWServW2STHandle); 
  if (ret != BLE_STATUS_SUCCESS) 
    return BLE_STATUS_ERROR;    

  // Fill the Environmental BLE Characteristc
  COPY_ENVIRONMENTAL_W2ST_CHAR_UUID(uuid);
  uuid[14] |= 0x04; // One Temperature value
  uuid[14] |= 0x08; // Humidity value
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);  
  ret =  aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, char_uuid.Char_UUID_128, 
                           2+2+4, 
                           CHAR_PROP_NOTIFY|CHAR_PROP_READ, 
                           ATTR_PERMISSION_NONE, 
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &EnvironmentalCharHandle);
  if (ret != BLE_STATUS_SUCCESS) 
    return BLE_STATUS_ERROR;

  return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Add the SW Feature service using a vendor specific profile
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus Add_SWServW2ST_Service(void)
{
  tBleStatus ret;
  int32_t NumberOfRecords=1;
  uint8_t uuid[16];

  COPY_SW_SENS_W2ST_SERVICE_UUID(uuid);
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_serv(UUID_TYPE_128, service_uuid.Service_UUID_128, PRIMARY_SERVICE, 
                          1+3*NumberOfRecords, &SWServW2STHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  COPY_QUATERNIONS_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(SWServW2STHandle, UUID_TYPE_128, char_uuid.Char_UUID_128, 
                           2+6*SEND_N_QUATERNIONS,
                           CHAR_PROP_NOTIFY,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &QuaternionsCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  return BLE_STATUS_SUCCESS;

fail:  
  return BLE_STATUS_ERROR;
}

/*******************************************************************************
* Function Name  : Read_Request_CB.
* Description    : Update the sensor value.
* Input          : Handle of the characteristic to update.
* Return         : None.
*******************************************************************************/
void Read_Request_CB(uint16_t handle)
{
  tBleStatus ret;

  if (handle == EnvironmentalCharHandle + 1)
  {
    float temperature, humidity;
	ReadTemperatureAndHumidityFromSensor(&temperature, &humidity);
	UpdateBluetoothData((int16_t)(temperature *10), (int16_t)(humidity * 10));
  }

  if(connection_handle !=0)
  { 
    ret = aci_gatt_allow_read(connection_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
      PRINTF("aci_gatt_allow_read() failed: 0x%02x\r\n", ret); 
    }
  }
}

tBleStatus UpdateBluetoothData(int16_t temperature, int16_t humidity)
{
  tBleStatus ret;
  uint8_t buff[8];
  HOST_TO_LE_16(buff, HAL_GetTick()>>3);

  HOST_TO_LE_16(buff+2,humidity);
  HOST_TO_LE_16(buff+4,temperature);

  ret = aci_gatt_update_char_value(HWServW2STHandle, EnvironmentalCharHandle, 
                                   0, 8, buff);

  if (ret != BLE_STATUS_SUCCESS){
    PRINTF("Error while updating TEMP characteristic: 0x%04X\n",ret) ;
    return BLE_STATUS_ERROR ;
  }

  return BLE_STATUS_SUCCESS;	
}
