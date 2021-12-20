#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "lcd.h"
#include "queue.h"
#include "touch.h"
#include "hcsr04.h"
#define BUF_SIZE 30;
Queue queue;
typedef struct _GyroInfo {
	char x;
	char y;
	int status;
} GyroInfo;

GyroInfo beforeGyroInfo;
GyroInfo currentGyroInfo;
int gyroValue;
float distance;
uint16_t brightValue;
void RCC_Configure(){
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  |RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD |RCC_APB2Periph_GPIOE| RCC_APB2Periph_USART1, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}
void GPIO_Configure(){ // GPIO ????
   // USART_1 Tx Setting
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   // USART_1 Rx Setting
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   // USART_2 Tx Setting
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   // USART_2 Rx Setting
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
   
   //moter1 dir1
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOC,&GPIO_InitStructure);
   
   //moter1 dir2
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOC,&GPIO_InitStructure);
   
   //moter2 dir1
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOD,&GPIO_InitStructure);
   
   //moter2 dir2
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOD,&GPIO_InitStructure);
   
   
   //supersonic Trig sensor
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOC,&GPIO_InitStructure);
   
   //supersonic Echo sensor
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOC,&GPIO_InitStructure);
   
   //Speaker
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOE,&GPIO_InitStructure);
   
   //bright IN sensor
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOC,&GPIO_InitStructure);
  
   
   //LED2
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOD,&GPIO_InitStructure);
   
   //LED3
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_Init(GPIOD,&GPIO_InitStructure);
}
void ADC_Configure(void){
  ADC_InitTypeDef ADC_12;
  //bright sensor ADC1
  ADC_DeInit(ADC1);
  ADC_12.ADC_Mode=ADC_Mode_Independent;
  ADC_12.ADC_ScanConvMode=DISABLE;
  ADC_12.ADC_ContinuousConvMode=ENABLE;
  ADC_12.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
  ADC_12.ADC_DataAlign=ADC_DataAlign_Right;
  ADC_12.ADC_NbrOfChannel=1;
  ADC_Init(ADC1,&ADC_12);
  ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_239Cycles5);
  ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
  ADC_Cmd(ADC1,ENABLE);
  
  ADC_ResetCalibration(ADC1);
  while(ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while(ADC_GetCalibrationStatus(ADC1));
  ADC_SoftwareStartConvCmd(ADC1,ENABLE);
}

void NVIC_Configure(){
   NVIC_InitTypeDef nvic1;
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
   
   NVIC_EnableIRQ(ADC1_2_IRQn);
   nvic1.NVIC_IRQChannel=ADC1_2_IRQn;
   nvic1.NVIC_IRQChannelCmd = ENABLE;
   nvic1.NVIC_IRQChannelPreemptionPriority = 1;
   nvic1.NVIC_IRQChannelSubPriority = 1;
   NVIC_Init(&nvic1);
  
     
   // USART_1 Interrupt ???? NVIC setting
   nvic1.NVIC_IRQChannel = USART1_IRQn;
   nvic1.NVIC_IRQChannelCmd = ENABLE;
   nvic1.NVIC_IRQChannelPreemptionPriority = 1;
   nvic1.NVIC_IRQChannelSubPriority = 1;
   NVIC_Init(&nvic1);

   // USART_2 Interrupt ???? NVIC setting
   nvic1.NVIC_IRQChannel = USART2_IRQn;
   nvic1.NVIC_IRQChannelCmd = ENABLE;
   nvic1.NVIC_IRQChannelPreemptionPriority = 1;
   nvic1.NVIC_IRQChannelSubPriority = 1;
   NVIC_Init(&nvic1);
}

void USART_Configure(){ // USART ??? ??? Setting
   USART_InitTypeDef usart1;

   usart1.USART_BaudRate = 9600;
   usart1.USART_WordLength = USART_WordLength_8b;
   usart1.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
   usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   usart1.USART_Parity = USART_Parity_No;
   usart1.USART_StopBits = USART_StopBits_1;

   USART_Init(USART1, &usart1);

   usart1.USART_BaudRate = 9600;
   usart1.USART_WordLength = USART_WordLength_8b;
   usart1.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
   usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   usart1.USART_Parity = USART_Parity_No;
   usart1.USART_StopBits = USART_StopBits_1;

   USART_Init(USART2, &usart1);

   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
   USART_Cmd(USART1, ENABLE);
   USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
   USART_Cmd(USART2, ENABLE);
}
void ADC1_2_IRQHandler(){
  if(ADC_GetITStatus(ADC1,ADC_IT_EOC)!=RESET){
    brightValue=ADC_GetConversionValue(ADC1);
  }
  ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
}
void USART1_IRQHandler(void) { // USART_1 interrupt ??? ?? o??(putty-???? ???)
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
    char data;
    data = USART_ReceiveData(USART1);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, data);
  }
  USART_ClearITPendingBit(USART1, USART_IT_RXNE);


}
void USART2_IRQHandler(void) { // USART_2 interrupt ??? ?? o??(???????? ???-???? ???)
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
    char data;
    Task task;
    data = USART_ReceiveData(USART2);
    task.data=data;
    task.usartType=2;
    queue_push(&queue,task);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); //do nothing, just wait
      USART_SendData(USART1, data); //
    
  }
  USART_ClearITPendingBit(USART1, USART_IT_RXNE);

}
int cnt = 0;
int main() {
   RCC_Configure();
   GPIO_Configure();
   ADC_Configure();
   NVIC_Configure();
   USART_Configure();
   Hcsr04Init();
   
   while (1) {
     //cnt++;
     if( cnt == 100000 ){
       cnt = 0;
      distance=Hcsr04GetLength();
      
      int dd = (int)distance;

      if( dd <15 ){
            GPIO_SetBits(GPIOE,GPIO_Pin_7);
            if( brightValue <200 ){
              GPIO_SetBits(GPIOD,GPIO_Pin_3);
            }
            else{
              GPIO_ResetBits(GPIOD,GPIO_Pin_3);
            }
      }
      else{
        GPIO_ResetBits(GPIOE,GPIO_Pin_7);
        GPIO_ResetBits(GPIOD,GPIO_Pin_3);
      }
      /*
      while( dd > 0 ){
      while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
      USART_SendData(USART1,dd%10 + 48);
        
        dd /= 10;
      }
      while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
      USART_SendData(USART1,' ');
      */
      
     }
     
     if(isEmpty(&queue)==false){
       Task task;
       queue_pop(&queue,&task);
       if(task.usartType==2){
         //while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
          //USART_SendData(USART1, task.data);
          if (task.data == '1') { // y
            currentGyroInfo.y = gyroValue;
          } else if (task.data == '0') { // x
            currentGyroInfo.x = gyroValue;
          } else {
            gyroValue = task.data;
          }

            if(currentGyroInfo.x>=60&&currentGyroInfo.x<88){    //go foward
              GPIO_ResetBits(GPIOC,GPIO_Pin_8);
              GPIO_ResetBits(GPIOD,GPIO_Pin_15);
              GPIO_SetBits(GPIOC,GPIO_Pin_9);
              GPIO_SetBits(GPIOD,GPIO_Pin_14);
            }
            if(currentGyroInfo.x>=92&&currentGyroInfo.x<109){   //back
              
              GPIO_ResetBits(GPIOC,GPIO_Pin_9);
              GPIO_ResetBits(GPIOD,GPIO_Pin_14);
              GPIO_SetBits(GPIOC,GPIO_Pin_8);
              GPIO_SetBits(GPIOD,GPIO_Pin_15);
            }
            if(currentGyroInfo.x==90){                          //stop
              
              GPIO_ResetBits(GPIOC,GPIO_Pin_8);
              GPIO_ResetBits(GPIOD,GPIO_Pin_14);
              GPIO_ResetBits(GPIOC,GPIO_Pin_9);
              GPIO_ResetBits(GPIOD,GPIO_Pin_15);
            }
          if(currentGyroInfo.y<=109&&currentGyroInfo.y>100){     //right
            /*GPIO_InitTypeDef GPIO_InitStructure;
            GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
             GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
            GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
            GPIO_Init(GPIOC,&GPIO_InitStructure);*/
              GPIO_SetBits(GPIOC,GPIO_Pin_9);
              GPIO_ResetBits(GPIOD,GPIO_Pin_14);
              GPIO_ResetBits(GPIOC,GPIO_Pin_8);
              GPIO_ResetBits(GPIOD,GPIO_Pin_15);
          }
          if(currentGyroInfo.y<=80&&currentGyroInfo.y>=71){     //left
            /*GPIO_InitTypeDef GPIO_InitStructure;
            GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
             GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
            GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
            GPIO_Init(GPIOD,&GPIO_InitStructure);*/
              GPIO_ResetBits(GPIOC,GPIO_Pin_8);
              GPIO_SetBits(GPIOD,GPIO_Pin_14);
              GPIO_ResetBits(GPIOC,GPIO_Pin_9);
              GPIO_ResetBits(GPIOD,GPIO_Pin_15);
          }
       }
    }
    
    
      if( brightValue <200 ){
        GPIO_SetBits(GPIOD,GPIO_Pin_2);
      }
      else{
        GPIO_ResetBits(GPIOD,GPIO_Pin_2);
     }
   }
}
