
#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <stdint.h>

#define IDB04A1 0
#define IDB05A1 1
#define SENSOR_DEMO_NAME   'B','l','u','e','N','R','G'
#define BDADDR_SIZE        6

void Set_DeviceConnectable(void);
void user_notify(void * pData);

extern uint8_t Application_Max_Attribute_Records[]; 

#endif /* _SENSOR_H_ */
