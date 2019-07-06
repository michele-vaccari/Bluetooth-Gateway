// Define to prevent recursive inclusion
#ifndef __APP_X_CUBE_BLE1_C
#define __APP_X_CUBE_BLE1_C
#ifdef __cplusplus
 extern "C" {
#endif

// Includes
#include "app_x-cube-ble1.h"

#include "hci.h"
#include "hci_le.h"
#include "hci_tl.h"
#include "link_layer.h"
#include "sensor.h"
#include "gatt_db.h"

#include "compiler.h"
#include "bluenrg_utils.h"
#include "stm32l4_nucleo_l452re_p.h"
#include "bluenrg_gap.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_hal_aci.h"
#include "sm.h"
#include "stm32l4xx_hal_tim.h"

// Private defines
#define USE_BUTTON 0	// 1 to send environmental and motion data when pushing the user button
 	 	 	 	 	 	// 0 to send environmental and motion data automatically (period = 1 sec)

// Private macros

// Private variables
I2C_HandleTypeDef hi2c3;

extern volatile uint8_t set_connectable;
extern volatile int     connected;
// at startup, suppose the X-NUCLEO-IDB04A1 is used
uint8_t bnrg_expansion_board = IDB04A1; 
uint8_t bdaddr[BDADDR_SIZE];
static volatile uint8_t user_button_init_state = 1;
static volatile uint8_t user_button_pressed = 0;

// Private function prototypes
static void User_Process(void);
static void User_Init(void);
static void Set_Random_Environmental_Values(float *data_t, float *data_h);
static void SetRandomBleMacAddress(uint8_t* bleMacAddress, uint8_t hardwareVersion, uint16_t firmwareVersion);
void ReadTemperatureAndHumidityFromSensor(float *temperature, float *humidity);

#if PRINT_CSV_FORMAT
extern volatile uint32_t ms_counter;

// This function is a utility to print the log time
// in the format HH:MM:SS:MSS (DK GUI time format)
void print_csv_time(void)
{
	uint32_t ms = HAL_GetTick();
	PRINT_CSV("%02d:%02d:%02d.%03d", ms/(60*60*1000)%24, ms/(60*1000)%60, (ms/1000)%60, ms%1000);
}
#endif

void MX_BlueNRG_MS_Init(I2C_HandleTypeDef *hi2c)
{
	hi2c3 = *hi2c;

	// Initialize the peripherals and the BLE Stack
	User_Init();

	// Get the User Button initial state
	user_button_init_state = BSP_PB_GetState(BUTTON_KEY);

	hci_init(user_notify, NULL);

	// Get the HW and FW versions
	uint8_t  hardwareVersion;
	uint16_t firmwareVersion;
	getBlueNRGVersion(&hardwareVersion, &firmwareVersion);

	PRINTF("Hardware version: %d\n\r", hardwareVersion);
	PRINTF("Firmware version: %d\n\r", firmwareVersion);
	if (hardwareVersion > 0x30)
	{
		// X-NUCLEO-IDB05A1 expansion board is used
		bnrg_expansion_board = IDB05A1;
	}

	// Reset BlueNRG again otherwise we won't be able to change its MAC address.
	// aci_hal_write_config_data() must be the first command after reset otherwise it will fail.
	hci_reset();
	HAL_Delay(100);

	// Change the MAC address to avoid issues with Android cache if different boards have the same MAC address
	SetRandomBleMacAddress(bdaddr, hardwareVersion, firmwareVersion);
	PRINTF("BLE MAC Address: %d\n\r", bdaddr);

	int ret;
	ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
									CONFIG_DATA_PUBADDR_LEN,
									bdaddr);
	if (ret)
	{
		PRINTF("Setting BD_ADDR failed.\n\r");
	}

	// SGATT Init
	ret = aci_gatt_init();
	if(ret)
	{
		PRINTF("GATT_Init failed.\n\r");
	}

	// GAP Init
	uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
	if (bnrg_expansion_board == IDB05A1)
	{
		ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
	}
	else
	{
		ret = aci_gap_init_IDB04A1(GAP_PERIPHERAL_ROLE_IDB04A1, &service_handle, &dev_name_char_handle, &appearance_char_handle);
	}
	if (ret != BLE_STATUS_SUCCESS)
	{
		PRINTF("GAP_Init failed.\n\r");
	}

	// Update characteristic name
	const char *name = "TSensor";
	ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0,
								   	 strlen(name), (uint8_t *)name);
	PRINTF("Characteristic name: %s\n\r", name);

	if (ret)
	{
		PRINTF("aci_gatt_update_char_value failed.\n\r");
		while(1);
	}

	ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
									   OOB_AUTH_DATA_ABSENT,
									   NULL,
									   7,
									   16,
									   USE_FIXED_PIN_FOR_PAIRING,
									   123456,
									   BONDING);
	if (ret)
	{
		PRINTF("aci_gap_set_authentication_requirement failed.\n\r");
		while(1);
	}

	PRINTF("BLE Stack Initialized\n\r");

	ret = Add_HWServW2ST_Service();
	if (ret == BLE_STATUS_SUCCESS)
	{
		PRINTF("BlueMS HW service added successfully.\n\r");
	}
	else
	{
		PRINTF("Error while adding BlueMS HW service: 0x%02x\r\n", ret);
		while(1);
	}

	ret = Add_SWServW2ST_Service();
	if (ret == BLE_STATUS_SUCCESS)
	{
		PRINTF("BlueMS SW service added successfully.\n\r");
	}
	else
	{
		 PRINTF("Error while adding BlueMS HW service: 0x%02x\r\n", ret);
		 while(1);
	}

	// Set output power level
	ret = aci_hal_set_tx_power_level(1,4);
}

// BlueNRG-MS background task
void MX_BlueNRG_MS_Process(void)
{
	User_Process();
	hci_user_evt_proc();
}

// Initialize User process.
static void User_Init(void)
{
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
	BSP_LED_Init(LED2);

	BSP_COM_Init(COM1);
}

// Process user input (i.e. pressing the USER button on Nucleo board)
// and send the updated acceleration data to the remote client.
static void User_Process(void)
{
	float data_t;
	float data_h;
	static uint32_t counter = 0;

	if (set_connectable)
	{
		Set_DeviceConnectable();
		set_connectable = FALSE;
	}
	BSP_LED_Toggle(LED2);

	if (connected)
	{
	  if (HAL_I2C_IsDeviceReady(&hi2c3, 0xB8, 2, 10) == HAL_OK)
	  {
		  ReadTemperatureAndHumidityFromSensor(&data_t, &data_h);
	  }

	  BlueMS_Environmental_Update((int16_t)(data_h *10), (int16_t)(data_t * 10));

	  counter ++;
	  if (counter == 40)
	  {
		counter = 0;
		//Reset_Motion_Values();
	  }
	  HAL_Delay(3000); // wait 3 sec before sending new data
	}
}

void ReadTemperatureAndHumidityFromSensor(float *temperature, float *humidity)
{
	uint8_t i2cData[8];
	unsigned int t;
	unsigned int h;

	//Transmit via I2C
	i2cData[0] = 0x03;
	i2cData[1] = 0x00;
	i2cData[2] = 0x04;
	HAL_I2C_Master_Transmit(&hi2c3, 0xB8, i2cData, 8, 10);

	//Read via I2C
	HAL_I2C_Master_Receive(&hi2c3, 0xB9, &i2cData, 8, 10);

	// Read temperature
	t = ((i2cData[4] & 0x7F) << 8) + i2cData[5];
	*temperature = t / 10.0;
	if (((i2cData[4] & 0x80) >> 7) == 1)
		*temperature *= -1; // the temperature can be negative

	h = (i2cData[2] << 8) + i2cData[3];
	*humidity = h / 10.0;
}

/**
 * @brief  Set random values for all environmental sensor data
 * @param  float pointer to temperature data
 * @param  float pointer to pressure data
 * @retval None
 */
static void Set_Random_Environmental_Values(float *data_t, float *data_h)
{ 
  *data_t = 27.0 + ((uint64_t)rand()*5)/RAND_MAX;     /* T sensor emulation */
  *data_h = 50.0 + ((uint64_t)rand()*10)/RAND_MAX; /* H sensor emulation */
}

/**
 * @brief  Set a random BLE MAC address
 * @param  Pointer to array where to save the BLE MAC address 
 * @param  Hardware version
 * @param  Firmware version
 * @retval None
 */
static void SetRandomBleMacAddress(uint8_t* bleMacAddress, uint8_t hardwareVersion, uint16_t firmwareVersion)
{
	// Initialize a random seed
	srand (HAL_GetTick() + hardwareVersion + firmwareVersion);

	uint8_t i;
	for (i=0; i<5; i++)
	{
		bleMacAddress[i] = rand()&0xFF;
	}
	bleMacAddress[i] = 0xD0;
}

/**
  * @brief  BSP Push Button callback
  * @param  Button Specifies the pin connected EXTI line
  * @retval None.
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  /* Set the User Button flag */
  user_button_pressed = 1;
}

#ifdef __cplusplus
}
#endif
#endif /* __APP_X_CUBE_BLE1_C */
