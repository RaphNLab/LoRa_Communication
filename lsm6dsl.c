/*
 * lsm6dsl.c
 *
 *  Created on: 28 Apr 2020
 *      Author: silvere
 */


#include "setup.h"
#include "lsm6dsl.h"


void enable_lsm6dsl(void)
{
	uint8_t cmd[2];
	// Boot time norally 15ms
	msleep(20);

	// Enable BDU and IF_NC
	cmd[0] = CTRL3_C;
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
	cmd[1] |= (BDU_ON | IF_NC_ON);
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 2, NULL, 0);

	// Set accelerometer ODR = 416Hz, FS_XL = ±4g (Output / FS_XL_4G)
	cmd[0] = CTRL1_XL;
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
	cmd[1] |= (1 << ODR_XL2 | 1 << ODR_XL1 | 1 << FS_XL1);
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 2, NULL, 0);

	// Set gyroscope ODR = 416Hz, FS_G = 1000dps (Output / FS_G_1000DPS)
	cmd[0] = CTRL2_GY;
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
	cmd[1] |= (1 << ODR_GY2 | 1 << ODR_GY1 | 1 << FS_GY1);
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 2, NULL, 0);

	// Enable accelerometer and gyroscope data ready on INT1
	cmd[0] = INT1_CTRL;
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
	cmd[1] |= (INT1_GY_ON | INT1_ACC_ON);
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 2, NULL, 0);

	// Enable rounding function
	cmd[0] = CTRL5_C;
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
	cmd[1] |= (1 << ROUNDING1 | 1 << ROUNDING0);
	i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 2, NULL, 0);
}

float *read_acc_axis(void)
{
	uint8_t outx_l_xl;
	uint8_t outx_h_xl;
	uint8_t outy_l_xl;
	uint8_t outy_h_xl;
	uint8_t outz_l_xl;
	uint8_t outz_h_xl;

	int16_t x;
	int16_t y;
	int16_t z;
	uint8_t status_cmd[2];
	uint8_t cmd[2];

	/*
	* Alternatively to keyword static in this case
	* use malloc to dynamic allocate memory for acc_xyz
	* e.g:
	* int *acc_xyz = malloc(sizeof(array_size))
	* dont froget to free memory after using it with free(array);
	*/
	static float acc_xyz[3];

	status_cmd[0] = STATUS_REG;
	i2c_transfer7(I2C2, LSM6DSL_ADDR, status_cmd, 1, (status_cmd+1), 1);

	if (status_cmd[1] & GET_XLDA) {
		// Read X
		cmd[0] = OUTX_L_XL;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outx_l_xl = cmd[1];

		cmd[0] = OUTX_H_XL;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outx_h_xl = cmd[1];

		x = ((int16_t)outx_h_xl << 8 | (int16_t)outx_l_xl);

		// Read Y

		cmd[0] = OUTY_L_XL;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outy_l_xl = cmd[1];

		cmd[0] = OUTY_H_XL;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outy_h_xl = cmd[1];

		y = ((int16_t)outy_h_xl << 8 | (int16_t)outy_l_xl);

		// Read Z
		cmd[0] = OUTZ_L_XL;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outz_l_xl = cmd[1];

		cmd[0] = OUTZ_H_XL;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outz_h_xl = cmd[1];

		z = ((int16_t)outz_h_xl << 8 | (int16_t)outz_l_xl);

		acc_xyz[0] = (float)x * FS_XL_4G;
		acc_xyz[1] = (float)y * FS_XL_4G;
		acc_xyz[2] = (float)z * FS_XL_4G;
	}
	return acc_xyz;

}


float *read_gy_grades(void)
{
	uint8_t outx_l_gy;
	uint8_t outx_h_gy;
	uint8_t outy_l_gy;
	uint8_t outy_h_gy;
	uint8_t outz_l_gy;
	uint8_t outz_h_gy;

	int16_t x;
	int16_t y;
	int16_t z;

	uint8_t status_cmd[2];
	uint8_t cmd[2];

	/*
	* Alternatively to keyword static in this case
	* use malloc to dynamic allocate memory for gy_xyz
	* e.g:
	* int *gy_xyz = malloc(sizeof(array_size))
	* dont froget to free memory after using it with free(array);
	*/
	static float gy_xyz[3];

	status_cmd[0] = STATUS_REG;
	i2c_transfer7(I2C2, LSM6DSL_ADDR, status_cmd, 1, (status_cmd+1), 1);

	if (status_cmd[1] & GET_GYDA) {

		// Read Y
		cmd[0] = OUTX_L_GY;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outx_l_gy = cmd[1];

		cmd[0] = OUTX_H_GY;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outx_h_gy = cmd[1];

		x = ((int16_t)outx_h_gy << 8 | (int16_t)outx_l_gy);

		// Read Y
		cmd[0] = OUTY_L_GY;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outy_l_gy = cmd[1];

		cmd[0] = OUTY_H_GY;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outy_h_gy = cmd[1];

		y = ((int16_t)outy_h_gy << 8 | (int16_t)outy_l_gy);

		// Read Z
		cmd[0] = OUTZ_L_GY;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outz_l_gy = cmd[1];

		cmd[0] = OUTZ_H_GY;
		i2c_transfer7(I2C2, LSM6DSL_ADDR, cmd, 1, (cmd+1), 1);
		outz_h_gy = cmd[1];

		z = ((int16_t)outz_h_gy << 8 | (int16_t)outz_l_gy);

		gy_xyz[0] = (float)x * FS_G_1000DPS;
		gy_xyz[1] = (float)y * FS_G_1000DPS;
		gy_xyz[2] = (float)z * FS_G_1000DPS;
	}
	return gy_xyz;
}
