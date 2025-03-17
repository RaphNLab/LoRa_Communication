/*
 * setup.c
 *
 *  Created on: 24 Jun 2020
 *      Author: silvere
 */

#include "setup.h"
void clock_setup(void)
{
	/* FIXME - this should eventually become a clock struct helper setup */
	rcc_osc_on(RCC_HSI16);

	flash_prefetch_enable(); // Look why
	flash_set_ws(4);
	flash_dcache_enable();
	flash_icache_enable();
	/* 16MHz / 4 = > 4 * 40 = 160MHz VCO => 80MHz main pll  */
	rcc_set_main_pll(RCC_PLLCFGR_PLLSRC_HSI16, 4, 40,
			0, 0, RCC_PLLCFGR_PLLR_DIV2);
	rcc_osc_on(RCC_PLL);

	rcc_periph_clock_enable(RCC_GPIOA | RCC_GPIOB | RCC_GPIOC | RCC_USART3 | RCC_UART4 | RCC_I2C2);
	rcc_set_sysclk_source(RCC_CFGR_SW_PLL); /* careful with the param here! */
	rcc_wait_for_sysclk_status(RCC_PLL);

	/* FIXME - eventually handled internally */
	rcc_ahb_frequency = 80e6;
	rcc_apb1_frequency = 80e6;
	rcc_apb2_frequency = 80e6;
}

void uart_setup(void)
{
	/* Setup GPIO pins for UART4 and USART3 transmit. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0 | GPIO1);
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4 | GPIO5);

	/* Setup UART4 TX and RX pin as alternate function. */
	gpio_set_af(GPIOA, GPIO_AF8, GPIO0 | GPIO1);
	gpio_set_af(GPIOC, GPIO_AF7, GPIO5 | GPIO4);

	//UART4 setup
	usart_set_baudrate(UART4, 9600);
	usart_set_databits(UART4, 8);
	usart_set_stopbits(UART4, USART_STOPBITS_1);
	usart_set_mode(UART4, USART_MODE_TX_RX);
	usart_set_parity(UART4, USART_PARITY_NONE);
	usart_set_flow_control(UART4, USART_FLOWCONTROL_NONE);

	//USART3 setup for printf commands
	usart_set_baudrate(USART3, 9600);
	usart_set_databits(USART3, 8);
	usart_set_stopbits(USART3, USART_STOPBITS_1);
	usart_set_mode(USART3, USART_MODE_TX_RX);
	usart_set_parity(USART3, USART_PARITY_NONE);
	usart_set_flow_control(USART3, USART_FLOWCONTROL_NONE);

	/* Finally enable UART4 and USART3. */
	usart_enable(USART3);
}

void gpio_setup(void)
{
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO14);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
}

void i2c2_setup(void)
{
	/* Setup SDA and SLC for I2C communication*/
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, SCL | SDA);

	/* Setup SDA and SCL pin as alternate function. */
	gpio_set_af(GPIOB, GPIO_AF4, SCL | SDA);

	i2c_peripheral_disable(I2C2);
	i2c_enable_analog_filter(I2C2);
	i2c_set_speed(I2C2,i2c_speed_sm_100k, 8);
	i2c_enable_stretching(I2C2);
	i2c_set_7bit_addr_mode(I2C2);
	i2c_peripheral_enable(I2C2);
}

/**
 * Use USART3 as a console.
 * This is a syscall for newlib
 * @param file
 * @param ptr
 * @param len
 * @return
 */

int _write(int file, char *ptr, int len)
{
	int i;

	if (file == STDOUT_FILENO || file == STDERR_FILENO) {
		for (i = 0; i < len; i++) {
			if (ptr[i] == '\n') {
				usart_send_blocking(USART3, '\r');
			}
			usart_send_blocking(USART3, ptr[i]);
		}
		return i;
	}
	errno = EIO;
	return -1;
}

void who_i_am(uint8_t dev, uint8_t reg)
{

	uint8_t reg_cmd[2];
	reg_cmd[0] = reg;
	i2c_transfer7(I2C2, dev, reg_cmd, 1, (reg_cmd+1), 1);

	printf("Who i am = 0x%02x\n",reg_cmd[1]);
}
