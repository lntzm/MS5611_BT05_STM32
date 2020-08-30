#ifndef __BT05_H
#define __BT05_H

#include "sys.h"

//连接模块GPIO相关参数的一层封装
//**********************************************************************************
#define RCC_STATE_EN  RCC_APB2Periph_GPIOB
//#define RCC_EN		RCC_APB2Periph_GPIOC
#define STATE_Pin GPIO_Pin_3 
#define EN_Pin GPIO_Pin_4
//**********************************************************************************

#define BT05_EN  	    PBout(4) 	//蓝牙控制EN信号
#define BT05_STATE  	PBin(3)		//蓝牙连接状态信号


u8 BT05_Init(void);
u8 BT05_Get_Role(void);
u8 BT05_Set_Cmd(u8* atstr);
void BT05_Role_Print(void);
void BT05_Sta_Print(void);
#endif