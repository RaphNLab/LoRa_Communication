/*
 * setup.h
 *
 *  Created on: 24 Jun 2020
 *      Author: silvere
 */

#ifndef SETUP_H_
#define SETUP_H_


#ifndef  EXTERN
#define  EXTERN  extern
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/i2c.h>
#include "systick.h"

#define SCL GPIO10
#define SDA GPIO11

EXTERN int reset_counter;
EXTERN int downlink;
EXTERN volatile char payload[30];

void clock_setup(void);
void uart_setup(void);
void gpio_setup(void);
void i2c2_setup(void);
int _write(int file, char *ptr, int len);
void who_i_am(uint8_t dev, uint8_t reg);

#endif /* SETUP_H_ */
