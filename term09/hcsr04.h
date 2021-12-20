#ifndef HCSR04_H_
#define HCSR04_H_

#include "stm32f10x.h"



/*-----[ Prototypes For All Functions ]-----*/

void Hcsr04Init();
static void OpenTimerForHc();
static void CloseTimerForHc();
void hcsr04_NVIC();
void TIM6_IRQHandler(void);
u32 GetEchoTimer(void);
float Hcsr04GetLength(void);
void Delay_Ms(uint16_t time);
void Delay_Us(uint16_t time);

#endif