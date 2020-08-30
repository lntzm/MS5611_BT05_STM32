#include "mydefine.h"
int fputc(int ch,FILE *f)
{
	USART_SendData(USART1,(u8)ch);
	while(!USART_GetFlagStatus(USART1,USART_FLAG_TXE));
	return ch;
}
