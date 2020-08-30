#include "ms5611.h"


//����7λ�����е�0λ�� "16 bit reserved for manufacturer "
//ʵ����Ч�����Ǵӵ�1λ����6λ
u16 FactoryCalculation[7];		//������ų���У׼����C1~C6.
double temp,pressure;					//temp:�¶�,pressure:ѹ��

//IO����
void MS5611_SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//����RCC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1|RCC_APB2Periph_GPIOA,ENABLE);
	
	//����SCL SDO SDI:�����������
	GPIO_InitStructure.GPIO_Pin=MS5611_PIN_SCL|   //SPI PA.5(SCK),PA.6(MISO),PA.7(MOSI),
															MS5611_PIN_SDA|
															MS5611_PIN_SDO;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Init(MS5611_PORT,&GPIO_InitStructure);
	
	//����CSB:�������
	GPIO_InitStructure.GPIO_Pin=MS5611_PIN_CSB;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(MS5611_PORT,&GPIO_InitStructure);
	//PA5/6/7����
	//GPIO_SetBits(MS5611_PORT,MS5611_PIN_SCL|MS5611_PIN_SDA|MS5611_PIN_SDO);
	
	//����SPI
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex; 	//˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;												//�� SPI 
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;									//���ݴ���� MSB λ��ʼ
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;												//��1�����������ݱ����� 
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;													//����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
	SPI_InitStructure.SPI_CRCPolynomial=1;														//CRC ֵ����Ķ���ʽ 
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;										// SPI ���ͽ��� 8 λ֡�ṹ 
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft; 													//NSS �ź���������� 
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_256;
	SPI_Init(SPI1,&SPI_InitStructure);
	
	SPI_Cmd(SPI1,ENABLE);
	
}

//ģ���ʼ��������
//1. ����IO
//2. ��λ
//3. ������У׼����
void MS5611_Init(void)
{
	MS5611_SPI_Configuration();
	MS5611_Reset();
	MS5611_ReadPROM();
}

//��λ
void MS5611_Reset(void)
{
	MS5611_PIN_CSB_BIT=0;
	delay_us(2);
	MS5611_ReadWriteByte(CMD_MS5611_RESET);
	delay_ms(3);		//��ʱ��ͼ��֪,��д��RESET�����,����Ҫ��2.8ms��RELOAD
	MS5611_PIN_CSB_BIT=1;
}

//������У׼����
void MS5611_ReadPROM(void)
{
	u8 FactoryCalculationBuf1=0,FactoryCalculationBuf2=0;
	u8 i=0;

	for(i=0;i<7;i++)			//������߸�����,��0������Ч��
	{
		MS5611_PIN_CSB_BIT=0;		//����Ƭѡ
		MS5611_ReadWriteByte(CMD_MS5611_PROM_READ+i*2);		//д��ַ��оƬ��
		FactoryCalculationBuf1=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);	//�ȶ���8λ,�����8λ��,�Զ���λ,��������8λ
		FactoryCalculationBuf2=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);	//����8λ
		FactoryCalculation[i]=(((u16)FactoryCalculationBuf1)<<8)|FactoryCalculationBuf2;
		MS5611_PIN_CSB_BIT=1;
	}
}

//���õ����¶Ⱥ�ѹ��ת��Ϊ���kPa
void MS5611_GetTP(double *TEMP,double *PRES)
{
	uint32_t D1,D2;
	double dT,OFF,SENS;
	double temperatureTemp,
						pressureTemp,
						T2;
	float Aux,OFF2,SENS2;  //�¶�У��ֵ
	D1=MS5611_ReadADCResult(CMD_CONVERT_D1_OSR_4096);   // ��δ������ѹ��ֵ 
	D2=MS5611_ReadADCResult(CMD_CONVERT_D2_OSR_4096);   // ��δ�������¶�ֵ 
	dT=D2-FactoryCalculation[5]*pow(2,8);
		
	OFF=FactoryCalculation[2]*pow(2,16)+dT*FactoryCalculation[4]/pow(2,7);     
	SENS=FactoryCalculation[1]*pow(2,15)+dT*FactoryCalculation[3]/pow(2,8); 
	temperatureTemp=(2000+(dT*FactoryCalculation[6])/pow(2,23)); 
	
	//�����¶Ȳ��������datasheet
	if(temperatureTemp < 2000)// second order temperature compensation when under 20 degrees C
	{
		T2 = (dT*dT) /pow(2,31);
		Aux = (temperatureTemp-2000)*(temperatureTemp-2000);
		OFF2 = 2.5*Aux;
		SENS2 = 1.25*Aux;
		if(temperatureTemp < -1500)
		{
			Aux = (temperatureTemp+1500)*(temperatureTemp+1500);
			OFF2 = OFF2 + 7*Aux;
			SENS2 =SENS2+ 5.5*Aux;
		}
	}
	else  //(Temperature > 2000)
	{
		T2 = 0;
		OFF2 = 0;
		SENS2 = 0;
	}
	
	OFF = OFF - OFF2;
	SENS = SENS - SENS2;	
	
	*TEMP =( temperatureTemp - T2)/100;		//��λ:��

	*PRES=(D1*SENS/2097152-OFF)/32768/1000;	//��λ:kPa
}

//��ȡ�¶Ⱥ�ѹ����������
u32 MS5611_ReadADCResult(u8 command)
{
	u32 adc=0;
	u8 Dx_TempH=0,
		 Dx_TempM=0,
		 Dx_TempL=0;

	MS5611_PIN_CSB_BIT=0;

	MS5611_ReadWriteByte(command);			 //����ת��ָ��
	delay_ms(20);
	
	MS5611_PIN_CSB_BIT=1;
	MS5611_PIN_CSB_BIT=0;
	/*
	ע��:��Ȼ24λ����ֻ��Ҫ��3��,������ʱ��ͼ��֪,��һ�ζ�ȡ��8λ��������Ч��,����Ҫ"�ն�"һ��
	�������ܰ�������ݸ�ȥ��
	*/
	MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);										//������ͷ8λ�����ֽ�
	Dx_TempH=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);           //����  
	Dx_TempM=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);           //����  
	Dx_TempL=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);     		  //���� 
	MS5611_PIN_CSB_BIT=1;
	
	adc=(((u32)Dx_TempH)<<16)+(((u32)Dx_TempM)<<8)+Dx_TempL;

	return adc;
}

//������,д����
u8 MS5611_ReadWriteByte(u8 dat)
{
//	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET);
//	SPI_I2S_SendData(SPI1,dat);
//	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET);
//	return SPI_I2S_ReceiveData(SPI1);	
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		retry++;
		if(retry>200)				// ����200�Σ����ERROR
			printf("ERROR!\n");
	}
	SPI_I2S_SendData(SPI1, dat); //ͨ������SPIx����һ������
	
	retry=0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		retry++;
		if(retry>200)
			printf("ERROR!\n");
	}	  						    
	return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����	
}
