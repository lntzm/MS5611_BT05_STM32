#ifndef _MYDEFINE_H
#define _MYDEFINE_H
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "delay.h"
#include "sys.h"
#include "misc.h"
#include "stm32f10x_usart.h"
#include "stdio.h"
#include "stm32f10x_spi.h"
#include "ms5611.h"
#include "math.h"
 
void RCC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void USART_Configuration(void);




#endif
