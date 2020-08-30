#ifndef _MS5611_H
#define _MS5611_H

#include "mydefine.h"

#define MS5611_PORT GPIOA
#define MS5611_PIN_CSB GPIO_Pin_4		//NSSλ,Ƭѡ
#define MS5611_PIN_SCL GPIO_Pin_5		//PA.5(SCK),��оƬ��SCL
#define MS5611_PIN_SDO GPIO_Pin_6		//PA.6(MISO),��оƬ��SDO
#define MS5611_PIN_SDA GPIO_Pin_7		//PA.7(MOSI),��оƬ��SDA(SDI)

#define MS5611_PIN_CSB_BIT PAout(4)
#define MS5611_PIN_SCL_BIT PAout(5)
#define MS5611_PIN_SDO_BIT PAout(6)
#define MS5611_PIN_SDA_BIT PAout(7)


#define CMD_MS5611_RESET 0x1E
#define CMD_MS5611_PROM_READ 0xA0
#define CMD_MS5611_ADC_READ 0x00

#define CMD_CONVERT_D1_OSR_4096 0x48  
#define CMD_CONVERT_D2_OSR_4096 0x58  


void MS5611_SPI_Configuration(void);//�˿�����
void MS5611_Reset(void);//��λ
void MS5611_ReadPROM(void);//������У׼ϵ��
u32 MS5611_ReadADCResult(u8 command);
void MS5611_GetTP(double *TEMP,double *PRES);//��ȡ�¶Ⱥ�ѹ��
u32 MS5611_ReadADCResult(u8 command);
void MS5611_Init(void);//��ʼ��
u8 MS5611_ReadWriteByte(u8 dat);//��д����
#endif
