#ifndef __UART_H__
#define __UART_H__

#include "stm32F4xx.h"
#include <stdio.h>


void USART2_init(void);
void delayMs(int);
int USART2_write(int c);
int USART2_read(void);

#endif