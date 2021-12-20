#define HCSR04_PORT     GPIOC
#define HCSR04_CLK      RCC_APB2Periph_GPIOC
#define HCSR04_TRIG     GPIO_Pin_10
#define HCSR04_ECHO     GPIO_Pin_11
#define PC_IDR (*(volatile unsigned int *)0x40011008)
#include "hcsr04.h"
#include "stm32f10x.h"
u16 msHcCount = 0;//MS count

void Hcsr04Init()
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;     //Generate structures for timer settings
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(HCSR04_CLK, ENABLE);

    //IO initialization
    GPIO_InitStructure.GPIO_Pin = HCSR04_TRIG;       //Sending level pin
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//Push-pull output
    GPIO_Init(HCSR04_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(HCSR04_PORT, HCSR04_TRIG);

    GPIO_InitStructure.GPIO_Pin = HCSR04_ECHO;     //Return the electricity pin
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//Floating input
    GPIO_Init(HCSR04_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(HCSR04_PORT, HCSR04_ECHO);

    //Timer initialization uses basic timer TIM6
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);   //Enable the RCC clock
    //Configure timer base structures
    TIM_DeInit(TIM6);
    TIM_TimeBaseStructure.TIM_Period = (1000 - 1); //Set the value of the automatic load register cycle of the next update event to 1000 to 1000 is 1 ms
    TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1); //Set the count frequency 1US count used to be used as a pre-frequency value 1m for TIMX clock frequency divisions
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//Unfrow
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //Tim up count mode
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); //Initialize TIMX time base unit according to the parameter specified in Tim_TimeBaseinitstruct         

    TIM_ClearFlag(TIM6, TIM_FLAG_Update);   //Clear update interrupts, remove the interrupt immediately
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);    //Open timer update interrupt
    hcsr04_NVIC();
    TIM_Cmd(TIM6, DISABLE);
}


//Tips: The scope of Static function is limited to defining its source file, so it is not necessary to declare in header files.
static void OpenTimerForHc()        //Open timer
{
    TIM_SetCounter(TIM6, 0);//Clear count
    msHcCount = 0;
    TIM_Cmd(TIM6, ENABLE);  //Make TIMX peripherals
}

static void CloseTimerForHc()        //Turn off timer
{
    TIM_Cmd(TIM6, DISABLE);  //Make TIMX peripherals
}


//NVIC configuration
void hcsr04_NVIC()
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;             //Select the serial port 1 interrupt
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //The sewage interrupt priority is set to 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         //Response interrupt priority set to 1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;        //Interrupt
    NVIC_Init(&NVIC_InitStructure);
}


//Timer 6 Interrupt Service
void TIM6_IRQHandler(void)   //TIM3 interrupt
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)  //Check if the TIM3 update interrupt occurs
    {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  //Clear TIMX Update Interrupt Sign 
        msHcCount++;
    }
}


//Get timer time
u32 GetEchoTimer(void)
{
    u32 t = 0;
    t = msHcCount * 1000;//Get MS
    t += TIM_GetCounter(TIM6);//Get US
    TIM6->CNT = 0;  //Clear the count value of the TIM2 count register
    Delay_Ms(50);
    return t;
}


//It takes a septum back signal between the two ranging from the ranging data of the ultrasonic ranging data.
//In order to eliminate the influence of the aftershock, the average value of 5 data is taken is weighted.
float Hcsr04GetLength(void)
{
    u32 t = 0;
    float lengthTemp = 0;
    GPIO_SetBits(GPIOC,GPIO_Pin_10);      //Send port high output
    Delay_Us(20);
    GPIO_ResetBits(GPIOC,GPIO_Pin_10); 
    while (~PC_IDR&(1<<11));      //Waiting for the receiving port high output
    OpenTimerForHc();        //Open timer
    while (PC_IDR&(1<<11));
    CloseTimerForHc();        //Turn off timer
    t = GetEchoTimer();        //Get time, resolution is 1US
    lengthTemp = ((float)t / 58.0);//cm
    return lengthTemp;
}


/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 ** Function Name: DELAY_MS_MS
 ** Function Description: Delay 1MS (can be judged by simulation to determine his accuracy)
 ** Parameter Description: Time (MS) Note Time <65535
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
void Delay_Ms(uint16_t time)  //Delay function
{
    uint16_t i, j;
    for (i = 0; i < time; i++)
        for (j = 0; j < 10260; j++);
}
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 ** Function Name: DELAY_MS_US
 ** Function Description: Delay 1US (can judge his accuracy by simulation)
 ** Parameter Description: Time (US) Note TIME <65535
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
void Delay_Us(uint16_t time)  //Delay function
{
    uint16_t i, j;
    for (i = 0; i < time; i++)
        for (j = 0; j < 9; j++);
}