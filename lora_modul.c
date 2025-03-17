/*
 * lora_modul.c
 *
 *  Created on: 30 Apr 2020
 *      Author: silvere
 */

#include "setup.h"
#include "lora_modul.h"
#include "systick.h"



static volatile char recv[30];
static volatile int flag = 0;


int joined = 0;

/*
 * List of possible codes
 */
/* Default network type is public (NTYP = 1, 0 for private network)
 * Band = EU868
 * DevEUI = e24f43fffe44c432
 * Duty Cycle (DC) = is on
 * Data Rate (DR) = SF7/125KHz
 * Class = Class A
 * APPEUI is a 8 bytes hex value
 * AppKey (AK) NSK ASP are 16 byte hex value
 * Device address (ADDR) is a 4 bytes hex value
 * Radio setting: Power: 14dBm
 * 				  Frequency: 867.1MHz
 * 				  Spreading factor: 12
 * 				  Bandwidth: 125KHz
 * 				  Coding rate: 4/5
 *
 */

char *code[] = {
	"AT\r\n\0",
	"ATZ\r\n\0",
	"AT+APPEUI\r\n\0",
	"AT+EUI\r\n\0",
	"AT+NSK\r\n\0",
	"AT+ASK\r\n\0",
	"AT+AK\r\n\0",
	"AT+ADDR\r\n\0",
	"AT+JSTA\r\n\0",
	"AT+RF=14,8671000000,12,0,1\r\n\0",
	"AT+RCV\r\n\0",
	AT_COMMAND(RX1DT, 1000),
	AT_COMMAND(RX2DT, 2000),
	AT_COMMAND(JRX1DT, 5000),
	AT_COMMAND(JRX2DT, 6000),
	AT_COMMAND(DC, 0),
	AT_COMMAND(BAND, 0),
	AT_COMMAND(DR, 0), //UE860: SF12-BW125
	AT_COMMAND(RX2DR, 0),//UE860: SF12-BW125
	AT_COMMAND(CLASS, 0),
	AT_COMMAND(ADDR, 12345678),
	AT_COMMAND(NSK, 1122334455663EAB546829CB361CAB7D),
	AT_COMMAND(ASK, 887766554433BCFACDE52476CA4598BA),
	AT_COMMAND(AK, 00112233445566778899AABBCCDDEEFF),
	AT_COMMAND(APPEUI, ABC123ADF135CBD8),
	AT_COMMAND(JOIN, 1),
	AT_COMMAND(JOIN, 0),
	AT_COMMAND(NTYP, 1),
	AT_COMMAND(ADR, 1)
};

char *error_list[] = {
	"OK",
	"1",
	"ERROR_UNKNOW",
	"ERROR_UNKNOW_COMMAND",
	"ERROR_LESS_ARGUMENTS",
	"ERROR_MORE_ARGUMENETS",
	"ERROR_INVALID_ARGUMENTS",
	"ERROR_INVALID_HEX_FORMAT",
	"ERROR_WAN_NON_JOINED"

};

void enable_uart4(void)
{
	usart_enable(UART4);
	nvic_enable_irq(NVIC_UART4_IRQ);
	usart_enable_rx_interrupt(UART4);
}

void disable_uart4(void)
{
	nvic_disable_irq(NVIC_UART4_IRQ);
	usart_disable_rx_interrupt(UART4);
	usart_disable(UART4);
}


void uart4_isr(void)
{
	int j = 0;
	if (usart_get_flag(UART4, USART_ISR_RXNE)) {

		/*
		 * Get response after AT-Command
		 * EX: send_cmd_get_status(AT_CLASS, AT_OK);
		 * If recv content != OK --> Error send the command again
		 *
		 */
		if (joined == 0) {
			recv[j] = usart_recv(UART4);
			j++;

			if (j == 2) {
				disable_uart4();
				j = 0;
				flag = 1;
			}
		}
	}
	if (usart_get_flag(UART4, USART_ISR_RXNE) && joined) {

		/* Get downlink
		 * EX: +RCV=2,2,3030
		 * The gateway send packet to module for APP port 2
		 * The packet's payload size is 1
		 * The payload data in Hex-format string
		 */
		payload[j] = usart_recv(UART4);
		j++;

		if (j == 13) {
			disable_uart4();
			j = 0;
			joined = 0;
			downlink = 1;
		}
	}
}


int get_response(enum lora_error err)
{
	if (flag) {
		/*Compare answer send command again if no matches*/
		return strcmp(recv, error_list[err]);
	}
	return 0;
}

void send_cmd(enum lora_cmd cmd)
{
	/*Send command*/
	if(cmd >= AT_MAX)
		return;

	char *command = code[cmd];
	enable_uart4();

	printf("cmd: %s\n",code[cmd]);
	while (*command != '\0') {
		usart_send_blocking(UART4, *command);
		command++;
	}
	msleep(500);
}

int get_join_status(void)
{
	/* Return 1 if node joined the network, 0 otherwise*/
	send_cmd(GET_JOIN_STATUS);

	printf("Join Satus: %s\n",recv);
	return (recv[0] == '1') ? 1 : 0;
}

/*
 * Send specific command
 * param: command name and error code
 *
 */
void send_cmd_get_status(enum lora_cmd cmd, enum lora_error err)
{

	send_cmd(cmd);
	switch(cmd)
	{
	case AT_JOIN_OTAA:
		/*Send the command again if join denied*/
		while (get_join_status() != 1) {
			printf("OTAA Join request denied\n");
			send_cmd(cmd);
		}
		printf("OTAA Join request accepted\n");
		joined = 1;
		break;

	case AT_JOIN_ABP:
		/*Send the command again if join denied*/
		while (get_join_status() != 1) {
			printf("ABP Join request denied\n");
			send_cmd(cmd);
		}
		printf("ABP Join request accepted\n");
		break;

	default:
		/* Send command
		 * If error code # OK, send command again
		 */
		if (get_response(err)) {
			printf("Command sent without error\n");
		} else {
			printf("Some error appears! Sending command again\n");
			send_cmd(cmd);
		}
	}
}

/*
 * Send data as string
 */
void send_data(char *datastr, enum lora_error err)
{
	int i = 0;
	char *str0 = "AT+SEND=2,";
	char *str1 = ",1\r\n\0";
	char str2[90];

	strcpy(str2, str0);
	strcat(str2, datastr);
	strcat(str2, str1);

	enable_uart4();
	joined = 0;

	printf("Data = %s", str2);
	while (str2[i] != '\0') {
		usart_send_blocking(UART4, str2[i]);
		i++;
	}

	msleep(500);
	if (get_response(err)) {
		printf("Command sent without error\n");
	} else {
		printf("Some error appears! Send command again\n");
		reset_counter++;
	}

	/* Joined = 1 means the node has already joined the network
	 * Due to the isr4 condition setting this variable to 0
	 * before sending data avoid the reset of the node
	 * Setting it to 1 when data have been send allow to catch
	 * downlinks (See isr4)
	 */
	enable_uart4();
	joined = 1;
}

/*
 * OTAA Mode: Set AppEUI, AppKey
 * Send an join request
 * get NwSKey and AppSKey from the gateway
 */
void lora_otaa_config(void)
{
	/* Check if device is ready */
	send_cmd_get_status(AT, AT_OK);

	/* Set class */
	send_cmd_get_status(AT_CLASS, AT_OK);

	/* Set/Reset duty cycle */
	send_cmd_get_status(AT_DUTY_CYCLE, AT_OK);

	/* Set data rate for TX window */
	send_cmd_get_status(AT_DATA_RATE, AT_OK);

	/* Set data rate for RX2 window */
	send_cmd_get_status(AT_RX2_DATA_RATE, AT_OK);

	/* Set AppEUI */
	send_cmd_get_status(AT_APPEUI, AT_OK);

	/* Set AppKey */
	send_cmd_get_status(AT_APP_KEY, AT_OK);

	/* Set RX1 delay to 1s*/
	send_cmd_get_status(AT_RX1_DELAY, AT_OK);

	/* Set RX2 delay to 2s*/
	send_cmd_get_status(AT_RX2_DELAY, AT_OK);

	/* Set join accept RX1 delay to 5s*/
	send_cmd_get_status(AT_JOIN_ACCEPT1_DELAY, AT_OK);

	/* Set join accept RX2 delay to 6s*/
	send_cmd_get_status(AT_JOIN_ACCEPT2_DELAY, AT_OK);

	/*Set radio values*/
	send_cmd_get_status(AT_RADIO, AT_OK);

	/* Join gateway with OTAA protocol */
	send_cmd_get_status(AT_JOIN_OTAA, AT_OK);
}

/*
 * ABP Mode:
 * Set device address, network session key
 * and application session key
 */
void lora_abp_config(void)
{
	/* Check if device is ready*/
	send_cmd_get_status(AT, AT_OK);

	/* Set class */
	send_cmd_get_status(AT_CLASS, AT_OK);

	/* Set/Reset duty cycle */
	send_cmd_get_status(AT_DUTY_CYCLE, AT_OK);

	/* Set data rate for TX window */
	send_cmd_get_status(AT_DATA_RATE, AT_OK);

	/* Set data rate for RX2 window */
	send_cmd_get_status(AT_RX2_DATA_RATE, AT_OK);

	/* Set device address */
	send_cmd_get_status(AT_ADDR, AT_OK);

	/* Set network session key */
	send_cmd_get_status(AT_NET_S_KEY, AT_OK);

	/* Set RX1 delay to 1s*/
	send_cmd_get_status(AT_RX1_DELAY, AT_OK);

	/* Set RX2 delay to 2s*/
	send_cmd_get_status(AT_RX2_DELAY, AT_OK);

	/* Set join accept RX1 delay to 5s*/
	send_cmd_get_status(AT_JOIN_ACCEPT1_DELAY, AT_OK);

	/* Set join accept RX2 delay to 6s*/
	send_cmd_get_status(AT_JOIN_ACCEPT2_DELAY, AT_OK);

	/* Set application session key */
	send_cmd_get_status(AT_APP_S_KEY, AT_OK);

	/*Set radio values*/
	send_cmd_get_status(AT_RADIO, AT_OK);

	/* Join gateway with ABP protocol */
	send_cmd_get_status(AT_JOIN_ABP, AT_OK);
}


/*
 * Convert each character of a floating point value
 * into Ascii hex string
 */
void float_to_Ascii(float value, char *buffer)
{
	char tmp[30];
	unsigned int i, x = 0;

	sprintf(tmp,"%.2f", value);
	for (i = 0; i < strlen(tmp); i++) {
	    sprintf((buffer+x), "%X", tmp[i]);
	    x += 2;
	}
}

void led_toggle(int led1, int led2)
{
	if (led1 == 1 && led2 == 1) {
		gpio_toggle(GPIOB, GPIO14);
		msleep(1000);
		gpio_toggle(GPIOB, GPIO14);
		msleep(1000);
		gpio_toggle(GPIOA, GPIO5);
		msleep(1000);
		gpio_toggle(GPIOA, GPIO5);
		msleep(1000);
	}
	else if (led1 == 1 && led2 == 0) {
		gpio_clear(GPIOA, GPIO5);
		gpio_toggle(GPIOB, GPIO14);
		msleep(1000);
		gpio_toggle(GPIOB, GPIO14);
		msleep(1000);
	}
	else if (led1 == 0 && led2 == 1) {
		gpio_clear(GPIOB, GPIO14);
		gpio_toggle(GPIOA, GPIO5);
		msleep(1000);
		gpio_toggle(GPIOA, GPIO5);
		msleep(1000);
	}
	else if (led1 == 0 && led2 == 0) {
		gpio_clear(GPIOB, GPIO14);
		gpio_clear(GPIOA, GPIO5);
	}
}


void downlink_event(volatile char *data)
{
	if (strcmp(data, "3131") == 0) { //11
		led_toggle(1,1);
	}
	else if (strcmp(data, "3130") == 0) { //10
		led_toggle(1,0);
	}
	else if (strcmp(data, "3031") == 0) { //01
		led_toggle(0,1);
	}
	else if (strcmp(data, "3030") == 0) { //00
		led_toggle(0,0);
	} else {
		printf("Strings does not match\n");
	}
	enable_uart4();
}
