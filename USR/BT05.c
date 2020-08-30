#include "BT05.h"
#include "usart2.h"

//��ʼ��BT05ģ��
//����ֵ:0,�ɹ�;1,ʧ��.
u8 BT05_Init(void)
{
	u8 retry=10,t;	  		 
	u8 temp=1;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_STATE_EN,ENABLE);	//ʹ��PORTBʱ��	
 
	GPIO_InitStructure.GPIO_Pin = STATE_Pin;				 // �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��PB3
	 
 
	GPIO_InitStructure.GPIO_Pin = EN_Pin;				 // �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��PB4

	GPIO_SetBits(GPIOB,EN_Pin);
 	
	USART2_Init(9600);	//��ʼ������2Ϊ:9600,������.
	
	delay_ms(500);
	
	while(retry--)
	{
		BT05_EN=1;					//KEY�ø�,����ATģʽ
		delay_ms(10);
		u2_printf("AT\r\n");		//����AT����ָ��
		BT05_EN=0;					//KEY����,�˳�ATģʽ
		for(t=0;t<10;t++) 			//��ȴ�50ms,������BT05ģ��Ļ�Ӧ
		{
			if(USART2_RX_STA&0X8000)break;
			delay_ms(5);
		}		
		if(USART2_RX_STA&0X8000)	//���յ�һ������
		{
			temp=USART2_RX_STA&0X7FFF;	//�õ����ݳ���
			USART2_RX_STA=0;			 
			if(temp==4&&USART2_RX_BUF[0]=='O'&&USART2_RX_BUF[1]=='K')
			{
				temp=0;//���յ�OK��Ӧ
				break;
			}
		}
	}
	if(retry==0)temp=1;	//���ʧ��
	return temp;
}	 

//��ȡBT05ģ���role
//����ֵ:0,�ӻ�;1,����;0XFF,��ȡʧ��.							  
u8 BT05_Get_Role(void)
{	 		    
	u8 retry=0X0F;
	u8 temp,t;
	while(retry--)
	{
		BT05_EN=1;					//KEY�ø�,����ATģʽ
		delay_ms(10);
		u2_printf("AT+ROLE?\r\n");	//��ѯrole
		for(t=0;t<20;t++) 			//��ȴ�200ms,������BT05ģ��Ļ�Ӧ
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)break;
		}		
		BT05_EN=0;					//KEY����,�˳�ATģʽ
		if(USART2_RX_STA&0X8000)	//���յ�һ��������
		{
			temp=USART2_RX_STA&0X7FFF;	//�õ����ݳ���
			USART2_RX_STA=0;			 
			if(temp==13&&USART2_RX_BUF[0]=='+')//���յ���ȷ��Ӧ����
			{
				temp=USART2_RX_BUF[6]-'0';//�õ�����ģʽֵ
				break;
			}
		}		
	}
	if(retry==0)temp=0XFF;//��ѯʧ��.
	return temp;
} 			
//BT05��������
//�˺�����������BT05,�����ڽ�����OKӦ���ATָ��
//atstr:ATָ�.����:"AT+RESET"/"AT+ROLE0"���ַ���
//����ֵ:0,���óɹ�;����,����ʧ��.							  
u8 BT05_Set_Cmd(u8* atstr)
{	 		    
	u8 retry=0X0F;
	u8 temp,t;
	while(retry--)
	{
		BT05_EN=1;					//KEY�ø�,����ATģʽ
		delay_ms(10);
		u2_printf("%s\r\n",atstr);	//����AT�ַ���
		BT05_EN=0;					//KEY����,�˳�ATģʽ
		for(t=0;t<20;t++) 			//��ȴ�100ms,������BT05ģ��Ļ�Ӧ
		{
			if(USART2_RX_STA&0X8000)break;
			delay_ms(5);
		}		
		if(USART2_RX_STA&0X8000)	//���յ�һ��������
		{
			temp=USART2_RX_STA&0X7FFF;	//�õ����ݳ���
			USART2_RX_STA=0;			 
			if(temp==4&&USART2_RX_BUF[0]=='O'&&USART2_RX_BUF[1]=='K')	//���յ���ȷ��Ӧ����				
			{			
				temp=0;
				break;			 
			}
		}		
	}
	if(retry==0)temp=0XFF;		//����ʧ��.
	return temp;
} 

void BT05_Role_Print(void)
{
	if(BT05_Get_Role()==1)
		printf("����ģʽ");			//����
	else 
		printf("�ӻ�ģʽ");			 		//�ӻ�
}

//��ʾBT05ģ�������״̬
void BT05_Sta_Print(void)
{												 
	if(BT05_STATE)
		printf("������");			//���ӳɹ�
	else 
		printf("δ����");	 			//δ����				 
}	