/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

   TODO:
   1. PWM na swiatlo
   2. Podswietlanie + wybudzanie na przycisk
   3. Ustawianie czasu na BT ?
   4. Mniejsze fonty
   5. Guzik przelacza miedzy wartosciami; High, Low, Srednia

**********************************************************************/
//JP6 PA4           <-> Con1 SCE
//JP6 PA5           <-> Con1 CLK
//JP6 PA6           <->
//JP6 PA7           <-> Con1 DIN
//JP6 PA8           <-> Con1 RES
//JP6 PA9           <-> Con1 D/C
//JP6 GND           <-> Con1 GND
//JP6 +3,3V (GPIOA) <-> Con1 +V

//TIMER
// PA12 SCL
// PA13 SDA



#include "stm32f10x_conf.h"
#include <stdio.h>
#include "..\LCD1\lcd_pcd8544_lib.h"
#include "..\LCD1\logo.h"
#include "..\delay\delay.h"
#include "..\ds1307\ds1307.h"
#include "..\DHT22\dht22.h"

#define termoWirePort GPIOB
#define termoWirePin GPIO_Pin_15

//#define TERM_RESOLUTION_12bit
#define TERM_RESOLUTION_11bit
//#define TERM_RESOLUTION_10bit
//#define TERM_RESOLUTION_9bit

#ifdef TERM_RESOLUTION_12bit
uint8_t TERM_RESOLUTION = 0x7F;
#endif
#ifdef TERM_RESOLUTION_11bit
uint8_t TERM_RESOLUTION = 0x5F;
#endif
#ifdef  TERM_RESOLUTION_10bit
uint8_t TERM_RESOLUTION = 0x3F;
#endif
#ifdef  TERM_RESOLUTION_9bit
uint8_t TERM_RESOLUTION = 0x1F;
#endif

void GPIO_Config(void);
void NVIC_Config(void);
void EXTI9_5_IRQHandler(void);
void IWDG_Config(void);

// 1-Wire
uint8_t term_Reset(void);
void term_SendBit(uint16_t bit);
uint8_t term_ReadBit(void);
void term_SendByte(uint8_t value);
uint8_t term_ReadByte(void);
void term_GetTemp(void);
void term_SetResolution(uint8_t term_resol);

int response;
uint16_t humidity,temperature;

// itoa


//Time
static uint8_t secCount = 0;
static uint8_t secFlag10 = 0;
static uint8_t secFlag1 = 0;

uint8_t lockFlag = 0;
uint8_t setTime = 0;


int main(void)
{


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //wlacz taktowanie ukladu SPI
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

    GPIO_Config();

    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {
        RCC_ClearFlag();
    }
    IWDG_Config();

    NVIC_Config();
    delay_Config();
    I2C_init1();
    DHT22_Init();

    LCDN_HwConfig();
    LCDN_Init();
    LCDN_Clear();

    GPIO_WriteBit(GPIOB,GPIO_Pin_0,Bit_SET);
    GPIO_SetBits(GPIOB,GPIO_Pin_6 | GPIO_Pin_7);
    LCDN_WriteBMP(frame);


    time t1 = getTime();


//        t1.hours = 18;
//        t1.minutes = 24;
//        t1.seconds = 0;
//        t1.date = 31;
//        t1.month = 10;
//        t1.year = 16;
        t1.rs0 = 0;
        t1.rs1 = 0;
        t1.sqwe = 1;
        t1.clock_halt = 0;

        writeTime(&t1);


    //term_Reset();
    //term_GetTemp();
    term_SetResolution(TERM_RESOLUTION);

    DHT22_Init();
    response = DHT22_GetReadings();



  while(1)
  {
    IWDG_ReloadCounter();

    if (secFlag1)
    {

        t1 = getTime();
        char hrs[10];
        sprintf(hrs, "%d:%d:%d", t1.hours,t1.minutes,t1.seconds);
        LCDN_WriteXY("        ", 1, 4);
        LCDN_WriteXY(hrs, 1, 4);
        secFlag1 = 0;
    }

    if (secFlag10)
    {

        //term_GetTemp();
        DHT22_Init();
        response = DHT22_GetReadings();
        response = DHT22_DecodeReadings();
        temperature = DHT22_GetTemperature();
        humidity = DHT22_GetHumidity();
        char temper[10];
        char humid[10];
        sprintf(temper,"%d.%d", ((temperature & 0x7fff) / 10), ((temperature & 0x7fff) % 10));
        sprintf(humid, "%d.%d", (humidity / 10),(humidity % 10));
        //sprintf(temper, "%d", response);
        LCDN_WriteXY("    ", 1, 1);
        LCDN_WriteXY(temper, 1, 1);
        LCDN_WriteChar(146);
        LCDN_WriteXY("    ", 8, 1);
        LCDN_WriteXY(humid, 8, 1);
        LCDN_WriteChar(37);

        secFlag10 = 0;
    }

  }
}

void NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);

    EXTI_InitStruct.EXTI_Line = EXTI_Line9;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);


}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line9))
    {
        secCount++;
        secFlag1 = 1;

        if (secCount >= 10)
        {
            secFlag10 = 1;
            secCount = 0;
        }

        EXTI_ClearITPendingBit(EXTI_Line9);
    }

}

void IWDG_Config(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload(625);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

void GPIO_Config(){
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Dla termometru
    GPIO_InitStruct.GPIO_Pin = termoWirePin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(termoWirePort, &GPIO_InitStruct);

    // For square wave input from DS1307
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOB, &GPIO_InitStruct);

}

// Termometr
uint8_t term_Reset(void)
{
    uint8_t PRESENCE = 0;

    GPIO_ResetBits(termoWirePort, termoWirePin);
    delay_us(480);
    GPIO_SetBits(termoWirePort, termoWirePin);
    delay_us(70);
    if (GPIO_ReadInputDataBit(termoWirePort,termoWirePin) == Bit_RESET) PRESENCE++;
    delay_us(410);
    if (GPIO_ReadInputDataBit(termoWirePort, termoWirePin) == Bit_SET) PRESENCE++;

    if (PRESENCE == 2)
    {

        //LCDN_WriteXY("Gra\0", 1,1);

        return 1;
    }
    else return 0;
}

void term_SendBit(uint16_t bit)
{
    if (bit == 0)
    {
        GPIO_ResetBits(termoWirePort, termoWirePin);
        delay_us(65);
        GPIO_SetBits(termoWirePort, termoWirePin);
        delay_us(10);
    }
    else
    {
        GPIO_ResetBits(termoWirePort, termoWirePin);
        delay_us(10);
        GPIO_SetBits(termoWirePort, termoWirePin);
        delay_us(65);
    }

}

uint8_t term_ReadBit(void)
{

    uint8_t bit = 0;
    GPIO_ResetBits(termoWirePort, termoWirePin);
    delay_us(5);
    GPIO_SetBits(termoWirePort, termoWirePin);
    delay_us(5);

    if (GPIO_ReadInputDataBit(termoWirePort,termoWirePin) == Bit_SET) bit = 1;
    else bit = 0;

    delay_us(55); //przerwa miedzy odczytem bitów

    return bit;
}

void term_SendByte(uint8_t value)
{

    uint8_t i, tmp;
    for (i = 0; i < 8; i++)
    {
        tmp = value >> i;
        tmp &= 0x01;
        term_SendBit(tmp);
    }
}

uint8_t term_ReadByte(void)
{
    uint8_t i, value = 0;
    for (i = 0; i < 8; i++)
    {
        if(term_ReadBit()) value |= 0x01 << i;
    }

    return value;
}

void term_SetResolution(uint8_t term_resol)
{
    uint8_t i, presence = 0, memory[3];

    memory[0]= 0;
    memory[1]= 0;
    memory[2]= term_resol;

    presence = term_Reset();

    if (presence == 1)
    {
        term_SendByte(0xCC); // Skip ROM
        term_SendByte(0x4E); // Write scratchpad

        for (i = 0; i < 3; i++)
        {
            term_SendByte(memory[i]);
        }
    }

    //presence = term_Reset();
}

void term_GetTemp(void)
{
    uint8_t i, presence = 0, memory[2];
    int temp, temp_int, temp_frac = 0;

    #ifdef TERM_RESOLUTION_12bit
    int pause = 7500;
    #endif
    #ifdef TERM_RESOLUTION_11bit
    int pause = 3750;
    #endif
    #ifdef TERM_RESOLUTION_10bit
    int pause = 1875;
    #endif
    #ifdef TERM_RESOLUTION_9bit
    int pause = 938;
    #endif

    presence = term_Reset();

    if (presence == 1)
    {
        term_SendByte(0xCC); // Skip ROM
        term_SendByte(0x44); // Convert T

        //Delay w zaleznoœci o ustawionej dokladnoœci pomiary
        for (i = 0; i < 100; i++) delay_us(pause);

        presence = 0;

    }

    presence = term_Reset();

    if (presence == 1)
    {
        term_SendByte(0xCC); //Skip ROM
        term_SendByte(0xBE); // Read Scratchpad

        for (i = 0; i < 2; i++) memory[i] = term_ReadByte();

        temp = memory[1] << 8;
        temp = temp | memory[0];

        temp_int = temp * 625 / 10000;
        temp_frac = ((temp * 625) % 10000) / 1000;

        char temper[10];
        sprintf(temper, "%d.%d", temp_int, temp_frac);
        LCDN_WriteXY("     \0",1,1);
        LCDN_WriteXY(temper, 1, 1);
        LCDN_WriteChar(146);


    }

    presence = term_Reset();



}


