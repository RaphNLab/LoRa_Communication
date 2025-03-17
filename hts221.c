/*
 * hts221.c
 *
 *  Created on: 30 Apr 2020
 *      Author: silvere
 */

#include "setup.h"
#include "hts221.h"


void enable_hts221(void)
{
	/*
	 * PD = 1 --> Turn on the device
	 * BDU = 1 --> Output register not update until MSB and LSB reading
	 * ORD1 = 1 ODR0 = 0 --> Set output data rate to 7Hz
	 */
	uint8_t cmd[2];
	cmd[0] = CTRL_REG1;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);
	cmd[1] |= (HTS221_PD_ON | HTS221_BDU_ON | HTS221_ODR1_ON);
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 2, NULL, 0);
}


float read_temperature(void)
{
	uint16_t T0_degc;
	uint16_t T1_degc;
	int16_t T0_out;
	int16_t T1_out;
	int16_t T_out;
	uint16_t T0_degc_x8;
	uint16_t T1_degc_x8;

	uint8_t reg;
	uint8_t cmd[4];
	uint8_t msb;

	int32_t tmp;
	static float temperature = 0.0;

	// Read T0_degc_x8 and T1_degc_x8
	cmd[0] = TO_DEGC_X8;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);
	cmd[0] = T1_DEGC_X8;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+2), 1);

	// Read the most significant bit of T1_DEGC and T0_DEGC
	cmd[0] = T1_T0_MSB;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+3), 1);

	// Calculate the T0_degc and T1_degc values
	T0_degc_x8 = (((uint16_t)(cmd[3] & 0x02)) << 8) | ((uint16_t)cmd[1]);
	T1_degc_x8 = (((uint16_t)(cmd[3] & 0x0C)) << 6) | ((uint16_t)cmd[2]);
	T0_degc = T0_degc_x8>>3;
	T1_degc = T1_degc_x8>>3;

	// Read T0_OUT less significant bit
	cmd[0] = T0_OUT_L;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);

	// Read T0_OUT most significant bit
	cmd[0] = T0_OUT_M;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+2), 1);
	T0_out = (((int16_t)cmd[2])<<8) | (int16_t)cmd[1];


	// Read T1_OUT less significant bit
	cmd[0] = T1_OUT_L;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);

	// Read T1_OUT most significant bit
	cmd[0] = T1_OUT_M;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+2), 1);
	T1_out = (((int16_t)cmd[2])<<8) | (int16_t)cmd[1];


	// Read T_OUT less significant bit
	cmd[0] = T_OUT_L;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);

	// Read T_OUT most significant bit
	cmd[0] = T_OUT_M;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+2), 1);
	T_out = (((int16_t)cmd[2])<<8) | (int16_t)cmd[1];

	// Calculate the temperature value
	tmp = ((int32_t)(T_out - T0_out)) * ((int32_t)(T1_degc - T0_degc));
	temperature = ((float)tmp / (float)(T1_out - T0_out)) + (float)(T0_degc);

	return temperature;
}


float read_humidity(void) {
	uint16_t H0_T0_out;
	uint16_t H1_T0_out;
	uint16_t H_out;
	uint16_t H0_rh;
	uint16_t H1_rh;

	uint8_t cmd[3];
	int32_t tmp;

	static float humidity = 0.0;

	// Read H0_rh and H1_rh coefficients
	cmd[0] = HO_RH_X2;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);

	cmd[0] = H1_RH_X2;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+2), 1);

	// Divide by two the content of registers H0_RH_X2 and H1_RH_X2
	H0_rh = cmd[1]>>1;
	H1_rh = cmd[2]>>1;

	// Read H0_T0_OUT less significant bit
	cmd[0] = HO_TO_OUT_L;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);

	// Read H0_T0_OUT most significant bit
	cmd[0] = HO_TO_OUT_M;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);
	H0_T0_out  = (((uint16_t)cmd[2])<<8) | (uint16_t)cmd[1];

	// Read H1_T0_OUT less significant bit
	cmd[0] = H1_TO_OUT_L;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);

	// Read H1_T0_OUT most significant bit
	cmd[0] = H1_TO_OUT_M;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+2), 1);
	H1_T0_out  = ((uint16_t)cmd[2]<<8) | (uint16_t)cmd[1];

	// Read H_OUT less significant bit
	cmd[0] = H_OUT_L;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+1), 1);

	// Read H1_T0_OUT most significant bit
	cmd[0] = H_OUT_M;
	i2c_transfer7(I2C2, HTS221_ADDR, cmd, 1, (cmd+2), 1);
	H_out  = (((uint16_t)cmd[2])<<8) | (uint16_t)cmd[1];

	tmp = ((int32_t)(H_out - H0_T0_out)) * ((int32_t)H1_rh - H0_rh);
	humidity = ((((float)tmp  / (float)(H1_T0_out - H0_T0_out)) + (float)H0_rh));

	if(humidity > 1000.0)
		humidity = 1000.0;

	return humidity;
}
