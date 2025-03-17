/*
 * lora_modul.h
 *
 *  Created on: 30 Apr 2020
 *      Author: silvere
 */

#ifndef LORA_MODUL_H_
#define LORA_MODUL_H_


/*
 * Stringification
 * Helful to convert macro argument into
 * string constant
 */
#define AT_COMMAND(cmd,	param) "AT+" #cmd "=" #param "\r\n\0"
#define LED GPIO5


/*
* List of possible AT command
*/
enum lora_cmd {
	AT = 0,
	ATZ,
	GET_APPEUI,
	GET_EUI,
	GET_NET_S_KEY,
	GET_APP_S_KEY,
	GET_APP_KEY,
	GET_ADDR,
	GET_JOIN_STATUS,
	AT_RADIO,
	AT_GETDATA,
	AT_RX1_DELAY,
	AT_RX2_DELAY,
	AT_JOIN_ACCEPT1_DELAY,
	AT_JOIN_ACCEPT2_DELAY,
	AT_DUTY_CYCLE,
	AT_BAND,
	AT_DATA_RATE,
	AT_RX2_DATA_RATE,
	AT_CLASS,
	AT_ADDR,
	AT_NET_S_KEY,
	AT_APP_S_KEY,
	AT_APP_KEY,
	AT_APPEUI,
	AT_JOIN_OTAA,
	AT_JOIN_ABP,
	AT_NETWORK_TYP,
	AT_ADAPTIVE_DATA_RATE,
	AT_MAX
};

enum lora_error{
	AT_OK = 0,
	JOIN_ACCEPTED,
	AT_ERROR_UNKNOW,                /* = -1 is USI  error code*/
	AT_ERROR_UNKNOW_COMMAND,        /* = -2   */
	AT_ERROR_LESS_ARGUMENTS,        /* = -3,  */
	AT_ERROR_MORE_ARGUMENETS,       /* = -4,  */
	AT_ERROR_INVALID_ARGUMENTS,     /* = -5,  */
	AT_ERROR_INVALID_HEX_FORMAT,    /* = -16, */
	AT_WAN_NON_JOINED,              /* = -103,*/
};



void send_cmd(enum lora_cmd cmd);
void send_cmd_get_status(enum lora_cmd cmd, enum lora_error err);
void send_data(char *datastr, enum lora_error err);
int get_response(enum lora_error err);
int get_join_status(void);
void lora_otaa_config(void);
void lora_abp_config(void);
void float_to_Ascii(float value, char *buffer);
void enable_uart4(void);
void disable_uart4(void);
void downlink_event(volatile char *data);
void led_toggle(int led1, int led2);

#endif /* LORA_MODUL_H_ */
