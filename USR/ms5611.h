#ifndef _MS5611_H
#define _MS5611_H

#include "mydefine.h"

#define MS5611_PORT GPIOA
#define MS5611_PIN_CSB GPIO_Pin_4		//NSS位,片选
#define MS5611_PIN_SCL GPIO_Pin_5		//PA.5(SCK),接芯片的SCL
#define MS5611_PIN_SDO GPIO_Pin_6		//PA.6(MISO),接芯片的SDO
#define MS5611_PIN_SDA GPIO_Pin_7		//PA.7(MOSI),接芯片的SDA(SDI)

#define MS5611_PIN_CSB_BIT PAout(4)
#define MS5611_PIN_SCL_BIT PAout(5)
#define MS5611_PIN_SDO_BIT PAout(6)
#define MS5611_PIN_SDA_BIT PAout(7)


#define CMD_MS5611_RESET 0x1E
#define CMD_MS5611_PROM_READ 0xA0
#define CMD_MS5611_ADC_READ 0x00

#define CMD_CONVERT_D1_OSR_4096 0x48  
#define CMD_CONVERT_D2_OSR_4096 0x58  


void MS5611_SPI_Configuration(void);//端口配置
void MS5611_Reset(void);//复位
void MS5611_ReadPROM(void);//读出厂校准系数
u32 MS5611_ReadADCResult(u8 command);
void MS5611_GetTP(double *TEMP,double *PRES);//获取温度和压力
u32 MS5611_ReadADCResult(u8 command);
void MS5611_Init(void);//初始化
u8 MS5611_ReadWriteByte(u8 dat);//读写数据
#endif
