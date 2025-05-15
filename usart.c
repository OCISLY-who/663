#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>
void USART1_Init(uint32_t baud) 
{
	  GPIO_InitTypeDef GPIO_InitStruct;
	  USART_InitTypeDef USART_InitStruct;
	  NVIC_InitTypeDef NVIC_InitStructure;
	
    // ���� GPIOA �� USART1 ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // ���� PA9 (TX) �� PA10 (RX)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;  // PA9 ���� TX PA10 ���� RX
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct); 
	
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // ���� USART1
   
    USART_InitStruct.USART_BaudRate = baud;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStruct);
		
		USART_Cmd(USART1, ENABLE);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

		// USART1 NVIC ����
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		  // ����1�ж�ͨ��
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // ��ռ���ȼ�
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  // �����ȼ�
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
		NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���
		 
}

//int8_t openmv_x = 0;
uint8_t openmv_x = 0;

uint8_t openmv_data[5];
uint8_t rx_index = 0;
uint32_t lost_counter = 0;
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        openmv_data[rx_index++] = USART_ReceiveData(USART1);
        if (openmv_data[0] != 0xA3) rx_index = 0;
        if (rx_index == 2 && openmv_data[1] != 0xB3) rx_index = 0;

        if (rx_index == 4)
        {
            if (openmv_data[3] == 0xC3)
            {
//                openmv_x = (int8_t)openmv_data[2];
							openmv_x = openmv_data[2];
//                openmv_y = (int8_t)openmv_data[3];
                lost_counter = 0; // ׷������
            }
            rx_index = 0;
        }
    }
}

uint8_t USART1_rxflag;
uint8_t USART1_getflag(void)
{
		if(USART1_rxflag == 1)// ������յ�����
		{
			USART1_rxflag =0;// ���ñ�־
			return 1;// ���� 1����ʾ�������
		}
		return 0;//���� 0����ʾû������
}

void USART1_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void USART1_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)
	{
		USART1_SendByte(Array[i]);
	}
}

void USART1_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		USART1_SendByte(String[i]);
	}
}

uint32_t USART1_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y --)
	{
		Result *= X;
	}
	return Result;
}

void USART1_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)
	{
		USART1_SendByte(Number / USART1_Pow(10, Length - i - 1) % 10 + '0');
	}
}

int fputc(int ch, FILE *f)
{
	USART1_SendByte(ch);
	return ch;
}

void USART1_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	USART1_SendString(String);
}



//#include "stm32f10x.h"
//#include <stdio.h>
//#include "usart.h"


//// ���յ������ݻ�����
//uint8_t rx_buffer[10];
//// �������ݳ���
//volatile uint16_t rx_length = 0;
//// ���ձ�־
//volatile uint8_t rx_flag = 0;

//// ����1��ʼ������
//void USART1_Init(void) {
//    GPIO_InitTypeDef GPIO_InitStructure;
//    USART_InitTypeDef USART_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

//    // ʹ��GPIOA��USART1ʱ��
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

//    // ����USART1��TX��RX����
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // ����USART1
//    USART_InitStructure.USART_BaudRate = 115200; // ������115200
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;
//    USART_InitStructure.USART_Parity = USART_Parity_No;
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//    USART_Init(USART1, &USART_InitStructure);

//    // ʹ��USART1
//    USART_Cmd(USART1, ENABLE);

//    // �����ж�
//    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

//    // ����NVIC
//    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//}


//// ����1�жϷ�����
//void USART1_IRQHandler(void) {
//    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
//        // ���յ�����
//        uint8_t temp = USART_ReceiveData(USART1);
//        if (rx_length < sizeof(rx_buffer)) {
//            rx_buffer[rx_length++] = temp;
//        }
//        // ����Ƿ������ɣ����Ը���Э�鶨��Ľ������жϣ�
//        if (temp == 0x5B) { // ����������0x5B����
//            rx_flag = 1;
//        }
//    }
//}

//// ������յ�������
//void processReceivedData(void) {
//    if (rx_length >= 7 && rx_flag) {
//        // ������ݸ�ʽ�Ƿ���ȷ
//        if (rx_buffer[0] == 0x2C && rx_buffer[6] == 0x5B) {
//            // ��ȡX��Y����
//            uint8_t x = rx_buffer[2];
//            uint8_t y = rx_buffer[3];
//            // ��ӡ���յ�������
//            char log[50];
//            sprintf(log, "Received X: %d, Y: %d\r\n", x, y);
//            // ͨ�����ڷ�����־����ѡ��
//            // sendString(log);
//        }
//        // ���ý��ջ�����
//        rx_length = 0;
//        rx_flag = 0;
//    }
//}


//// ���������������ַ���
//void sendString(const char *str) {
//    while (*str) {
//        USART_SendData(USART1, *str++);
//        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
//    }
//}


//int fputc(int ch, FILE *f)
//{
//    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//    USART_SendData(USART1, (uint8_t)ch);
//    return ch;
//}
