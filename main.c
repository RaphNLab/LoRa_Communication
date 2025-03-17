/*
 * main.c
 *
 * Created on: 24.06.2020
 *      Author: silvere
 */

 #define EXTERN

#include "setup.h"
#include "lora_modul.h"
#include "hts221.h"
#include "lsm6dsl.h"

#define STM32L4

int main(void)
{
	float temperature;
//	float humidity;
//
//	float *acc_xyz;
//	float *gy_xyz;

	clock_setup();
	systick_ms_setup();
	i2c2_setup();
	uart_setup();
	gpio_setup();

	reset_counter = 0;
	downlink = 0;
	printf("Prog start\n");

	/* Wait the hardware to be ready*/
	msleep (2000);
	lora_otaa_config();

	//char *datastr = "48454C4C4F204D4F544F";
	char data[80];
	while (1) {
		msleep (5000);
		//send_cmd_get_status(AT_GETDATA, AT_OK);

		temperature = read_temperature();
		float_to_Ascii(temperature, data);
		printf("Temperatur = %.2lf\n",temperature);

//		float humidity = read_humidity();
//
//		float *acc_xyz = read_acc_axis();
//		float *gy_xyz = read_gy_grades();

		send_data(data, AT_OK);
		msleep (5000);

		if (downlink) {
			printf("payload main: %s\n", payload+9);
			downlink_event(payload+9);

			/* Reset to catch next downlink*/
			downlink = 0;
		}
		if (reset_counter >= 5) {
		/* Configure the node again if sending data fails more than 5 times*/
			lora_otaa_config();
			//lora_abp_config();
			reset_counter = 0;
		}
	}
	return 0;
}
