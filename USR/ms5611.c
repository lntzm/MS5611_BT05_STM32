#include "ms5611.h"


//数组7位，其中第0位被 "16 bit reserved for manufacturer "
//实际有效参数是从第1位到第6位
u16 FactoryCalculation[7];		//用来存放出厂校准数据C1~C6.
double temp,pressure;					//temp:温度,pressure:压力

//IO配置
void MS5611_SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//配置RCC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1|RCC_APB2Periph_GPIOA,ENABLE);
	
	//配置SCL SDO SDI:复用推挽输出
	GPIO_InitStructure.GPIO_Pin=MS5611_PIN_SCL|   //SPI PA.5(SCK),PA.6(MISO),PA.7(MOSI),
															MS5611_PIN_SDA|
															MS5611_PIN_SDO;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Init(MS5611_PORT,&GPIO_InitStructure);
	
	//配置CSB:推挽输出
	GPIO_InitStructure.GPIO_Pin=MS5611_PIN_CSB;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(MS5611_PORT,&GPIO_InitStructure);
	//PA5/6/7上拉
	//GPIO_SetBits(MS5611_PORT,MS5611_PIN_SCL|MS5611_PIN_SDA|MS5611_PIN_SDO);
	
	//配置SPI
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex; 	//双线双向全双工
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;												//主 SPI 
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;									//数据传输从 MSB 位开始
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;												//第1个跳变沿数据被采样 
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;													//串行同步时钟的空闲状态为低电平
	SPI_InitStructure.SPI_CRCPolynomial=1;														//CRC 值计算的多项式 
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;										// SPI 发送接收 8 位帧结构 
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft; 													//NSS 信号由软件控制 
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_256;
	SPI_Init(SPI1,&SPI_InitStructure);
	
	SPI_Cmd(SPI1,ENABLE);
	
}

//模块初始化，包括
//1. 配置IO
//2. 复位
//3. 读出厂校准数据
void MS5611_Init(void)
{
	MS5611_SPI_Configuration();
	MS5611_Reset();
	MS5611_ReadPROM();
}

//复位
void MS5611_Reset(void)
{
	MS5611_PIN_CSB_BIT=0;
	delay_us(2);
	MS5611_ReadWriteByte(CMD_MS5611_RESET);
	delay_ms(3);		//由时序图可知,在写完RESET命令后,还需要有2.8ms的RELOAD
	MS5611_PIN_CSB_BIT=1;
}

//读出厂校准数据
void MS5611_ReadPROM(void)
{
	u8 FactoryCalculationBuf1=0,FactoryCalculationBuf2=0;
	u8 i=0;

	for(i=0;i<7;i++)			//共需读七个数据,第0个是无效的
	{
		MS5611_PIN_CSB_BIT=0;		//拉低片选
		MS5611_ReadWriteByte(CMD_MS5611_PROM_READ+i*2);		//写地址到芯片中
		FactoryCalculationBuf1=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);	//先读高8位,读完高8位后,自动移位,再来读低8位
		FactoryCalculationBuf2=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);	//读低8位
		FactoryCalculation[i]=(((u16)FactoryCalculationBuf1)<<8)|FactoryCalculationBuf2;
		MS5611_PIN_CSB_BIT=1;
	}
}

//将得到的温度和压力转化为℃和kPa
void MS5611_GetTP(double *TEMP,double *PRES)
{
	uint32_t D1,D2;
	double dT,OFF,SENS;
	double temperatureTemp,
						pressureTemp,
						T2;
	float Aux,OFF2,SENS2;  //温度校验值
	D1=MS5611_ReadADCResult(CMD_CONVERT_D1_OSR_4096);   // 读未补偿的压力值 
	D2=MS5611_ReadADCResult(CMD_CONVERT_D2_OSR_4096);   // 读未补偿的温度值 
	dT=D2-FactoryCalculation[5]*pow(2,8);
		
	OFF=FactoryCalculation[2]*pow(2,16)+dT*FactoryCalculation[4]/pow(2,7);     
	SENS=FactoryCalculation[1]*pow(2,15)+dT*FactoryCalculation[3]/pow(2,8); 
	temperatureTemp=(2000+(dT*FactoryCalculation[6])/pow(2,23)); 
	
	//二阶温度补偿，详见datasheet
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
	
	*TEMP =( temperatureTemp - T2)/100;		//单位:℃

	*PRES=(D1*SENS/2097152-OFF)/32768/1000;	//单位:kPa
}

//读取温度和压力的数字量
u32 MS5611_ReadADCResult(u8 command)
{
	u32 adc=0;
	u8 Dx_TempH=0,
		 Dx_TempM=0,
		 Dx_TempL=0;

	MS5611_PIN_CSB_BIT=0;

	MS5611_ReadWriteByte(command);			 //发送转换指令
	delay_ms(20);
	
	MS5611_PIN_CSB_BIT=1;
	MS5611_PIN_CSB_BIT=0;
	/*
	注意:虽然24位数据只需要都3次,但是由时序图可知,第一次读取的8位数据是无效的,所以要"空读"一次
	这样就能把这个数据给去掉
	*/
	MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);										//跳过开头8位无用字节
	Dx_TempH=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);           //读数  
	Dx_TempM=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);           //读数  
	Dx_TempL=MS5611_ReadWriteByte(CMD_MS5611_ADC_READ);     		  //读数 
	MS5611_PIN_CSB_BIT=1;
	
	adc=(((u32)Dx_TempH)<<16)+(((u32)Dx_TempM)<<8)+Dx_TempL;

	return adc;
}

//读数据,写命令
u8 MS5611_ReadWriteByte(u8 dat)
{
//	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET);
//	SPI_I2S_SendData(SPI1,dat);
//	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET);
//	return SPI_I2S_ReceiveData(SPI1);	
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
		retry++;
		if(retry>200)				// 重试200次，输出ERROR
			printf("ERROR!\n");
	}
	SPI_I2S_SendData(SPI1, dat); //通过外设SPIx发送一个数据
	
	retry=0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
	{
		retry++;
		if(retry>200)
			printf("ERROR!\n");
	}	  						    
	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据	
}
