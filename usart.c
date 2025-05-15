#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>
void USART1_Init(uint32_t baud) 
{
	  GPIO_InitTypeDef GPIO_InitStruct;
	  USART_InitTypeDef USART_InitStruct;
	  NVIC_InitTypeDef NVIC_InitStructure;
	
    // 启用 GPIOA 和 USART1 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // 配置 PA9 (TX) 和 PA10 (RX)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;  // PA9 用于 TX PA10 用于 RX
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct); 
	
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置 USART1
   
    USART_InitStruct.USART_BaudRate = baud;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStruct);
		
		USART_Cmd(USART1, ENABLE);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

		// USART1 NVIC 配置
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		  // 串口1中断通道
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  // 子优先级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器
		 
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
                lost_counter = 0; // 追踪正常
            }
            rx_index = 0;
        }
    }
}

uint8_t USART1_rxflag;
uint8_t USART1_getflag(void)
{
		if(USART1_rxflag == 1)// 如果接收到数据
		{
			USART1_rxflag =0;// 重置标志
			return 1;// 返回 1，表示接收完成
		}
		return 0;//返回 0，表示没有数据
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


//// 接收到的数据缓冲区
//uint8_t rx_buffer[10];
//// 接收数据长度
//volatile uint16_t rx_length = 0;
//// 接收标志
//volatile uint8_t rx_flag = 0;

//// 串口1初始化函数
//void USART1_Init(void) {
//    GPIO_InitTypeDef GPIO_InitStructure;
//    USART_InitTypeDef USART_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

//    // 使能GPIOA和USART1时钟
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

//    // 配置USART1的TX和RX引脚
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // 配置USART1
//    USART_InitStructure.USART_BaudRate = 115200; // 波特率115200
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;
//    USART_InitStructure.USART_Parity = USART_Parity_No;
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//    USART_Init(USART1, &USART_InitStructure);

//    // 使能USART1
//    USART_Cmd(USART1, ENABLE);

//    // 配置中断
//    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

//    // 配置NVIC
//    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//}


//// 串口1中断服务函数
//void USART1_IRQHandler(void) {
//    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
//        // 接收到数据
//        uint8_t temp = USART_ReceiveData(USART1);
//        if (rx_length < sizeof(rx_buffer)) {
//            rx_buffer[rx_length++] = temp;
//        }
//        // 检查是否接收完成（可以根据协议定义的结束符判断）
//        if (temp == 0x5B) { // 假设数据以0x5B结束
//            rx_flag = 1;
//        }
//    }
//}

//// 处理接收到的数据
//void processReceivedData(void) {
//    if (rx_length >= 7 && rx_flag) {
//        // 检查数据格式是否正确
//        if (rx_buffer[0] == 0x2C && rx_buffer[6] == 0x5B) {
//            // 提取X和Y坐标
//            uint8_t x = rx_buffer[2];
//            uint8_t y = rx_buffer[3];
//            // 打印接收到的坐标
//            char log[50];
//            sprintf(log, "Received X: %d, Y: %d\r\n", x, y);
//            // 通过串口发送日志（可选）
//            // sendString(log);
//        }
//        // 重置接收缓冲区
//        rx_length = 0;
//        rx_flag = 0;
//    }
//}


//// 辅助函数：发送字符串
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
