#ifndef _USART_H
#define _USART_H


#include <stdio.h>

extern char USART1_RxPacket[];
extern uint8_t USART1_RxFlag;

uint8_t USART1_getflag(void);

void USART1_SendByte(uint8_t Byte);
void USART1_SendArray(uint8_t *Array, uint16_t Length);
void USART1_SendString(char *String);
void USART1_SendNumber(uint32_t Number, uint8_t Length);
void USART1_Printf(char *format, ...);


void USART1_Init(uint32_t baud);
void USART1_IRQHandler(void);


//extern int8_t openmv_x , openmv_y;
extern uint8_t openmv_x , openmv_y;
extern uint32_t lost_counter;

#endif
