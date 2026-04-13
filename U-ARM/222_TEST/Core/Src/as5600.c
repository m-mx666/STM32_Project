#include "as5600.h"

extern I2C_HandleTypeDef hi2c2;

/**
 * @brief 检查传感器是否就绪及磁铁状态
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef AS5600_Check_Status(void) {
    uint8_t status = 0;
    // 读取状态寄存器：Check if magnet is detected (MD bit)
    return HAL_I2C_Mem_Read(&hi2c2, AS5600_ADDR, AS5600_STATUS_REG, I2C_MEMADD_SIZE_8BIT, &status, 1, 100);
}

/**
 * @brief 读取 12 位原始角度数据 (0-4095)
 */
uint16_t AS5600_Read_Raw_Angle(void) {
    uint8_t data[2];
    uint16_t raw_angle = 0;

    // 连续读取两个字节：0x0C (MSB) 和 0x0D (LSB)
    if (HAL_I2C_Mem_Read(&hi2c2, AS5600_ADDR, AS5600_RAW_ANGLE_MSB, I2C_MEMADD_SIZE_8BIT, data, 2, 100) == HAL_OK) {
        raw_angle = ((uint16_t)(data[0] & 0x0F) << 8) | data[1];
    }
    return raw_angle;
}

/**
 * @brief 转换为 0-360.0 度
 */
float AS5600_Get_Degree(void) {
    return (float)AS5600_Read_Raw_Angle() * 360.0f / 4096.0f;
}
