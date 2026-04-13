#ifndef __AS5600_H
#define __AS5600_H

#include "main.h"

/* 硬件参数固化 */
#define AS5600_ADDR        (0x36 << 1)
#define AS5600_RAW_ANGLE_MSB 0x0C
#define AS5600_RAW_ANGLE_LSB 0x0D
#define AS5600_STATUS_REG    0x0B

/* 功能接口 */
HAL_StatusTypeDef AS5600_Check_Status(void);
uint16_t AS5600_Read_Raw_Angle(void);
float AS5600_Get_Degree(void);

#endif
