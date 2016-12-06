/*
 * fun.c
 *
 *  Created on: Nov 24, 2011
 *      Author: Marcin
 */
#include "main.h"
extern __IO int t;
extern __IO float temperature;

void Config(void)	//funkcja konfiguracyjna
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

		//deklaracja struktur inicjalizacyjnych
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	EXTI_InitTypeDef EXTI_InitStructure;

	//konfiguracja encodera
	GPIO_InitStructure.GPIO_Pin = pinA | pinB | pinC;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(portEN, &GPIO_InitStructure);

	//konfiguracja termometru
	GPIO_InitStructure.GPIO_Pin = wire1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(portW1, &GPIO_InitStructure);

	//konfiguracja pierwszej cyfry wyswitlacza
	GPIO_InitStructure.GPIO_Pin = a1 | b1 | c1 | d1 | e1 | f1 | g1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(portD1, &GPIO_InitStructure);
	GPIO_SetBits(portD1,a1 | b1 | c1 | d1 | e1 | f1 | g1);

	//konfiguracja drugiej cyfry wyswietlacza
	GPIO_InitStructure.GPIO_Pin = a2 | b2 | c2 | d2 | e2 | f2 | g2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(portD2, &GPIO_InitStructure);
	GPIO_SetBits(portD2,a2 | b2 | c2 | d2 | e2 | f2 | g2);

	//konfiguracja grza³ki w tym przypadku to jest dioda na p³ytce
	GPIO_InitStructure.GPIO_Pin = heater;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(portHE, &GPIO_InitStructure);
	GPIO_SetBits(portHE,heater);

	//konfiguracja timera 2
	TIM_InitStruct.TIM_ClockDivision = 0; 					// dzielnik 0
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up; 	// licznik w gore
	TIM_InitStruct.TIM_Period = 65535; 						// okres licznika
	TIM_InitStruct.TIM_Prescaler = 72; 						// preskaler 72
	TIM_TimeBaseInit(TIM2, &TIM_InitStruct); 				// inicjalizuje TIM2

	//konfiguracja kontrolera przerwañ
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource9); //poinformowanie uC o zrodle przerwania (PA9)

	//konfiguracja zewnêtrzengo przerwania
	EXTI_InitStructure.EXTI_Line = EXTI_Line9;				// Konfiguracja dotyczy linii 9.
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// Bedziemy zglaszac przerwania
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // Na zboczu opadajacym (patrz schemat dzojstika)
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 				// Uaktywniamy linie EXTI
	EXTI_Init(&EXTI_InitStructure);

	SysTick_Config(SysTick_LOAD_RELOAD_Msk-1);			 	//konfiguracja SysTic
	NVIC_SetPriority(SysTick_IRQn, 0);						//ustawienie priorytetu przerwanai od SysTick

	TIM_Cmd(TIM2, ENABLE);		//w³¹czenie timera 2
}

//funkcja opóŸniaj¹ca
void delay(uint16_t time)
{
	TIM_SetCounter(TIM2,0);						//zerowanie licznika
	while (TIM_GetCounter(TIM2)<=time); 		//odczyt CNT i porównanie
}

//funkcja obs³uguj¹ca pierwsz¹ lub drug¹ cyfre wyswietlacza (wywietlenie odpowiedniej cyfry)
void digit(uint16_t cyfra, uint16_t digit)
{
	if (digit==1)
	{
		switch (cyfra)
		{
		case 0:
			GPIO_ResetBits(portD1, a1 | b1| c1 | d1 | e1 | f1);
			GPIO_SetBits(portD1, g1);
			break;
		case 1:
			GPIO_ResetBits(portD1, b1 | c1);
			GPIO_SetBits(portD1, a1 | d1 | e1 | f1 | g1);
			break;
		case 2:
			GPIO_ResetBits(portD1, a1 | b1  | d1 | e1 | g1);
			GPIO_SetBits(portD1, c1 | f1);
			break;
		case 3:
			GPIO_ResetBits(portD1, a1 | b1 | c1 | d1 | g1);
			GPIO_SetBits(portD1, e1 | f1);
			break;
		case 4:
			GPIO_ResetBits(portD1, b1 | c1 | f1 | g1);
			GPIO_SetBits(portD1, a1 | d1 | e1);
			break;
		case 5:
			GPIO_ResetBits(portD1, a1 | c1 | d1 | f1 | g1);
			GPIO_SetBits(portD1, b1 | e1);
			break;
		case 6:
			GPIO_ResetBits(portD1, a1 | c1 | d1 | e1 | f1 | g1);
			GPIO_SetBits(portD1, b1);
			break;
		case 7:
			GPIO_ResetBits(portD1, a1 | b1 | c1);
			GPIO_SetBits(portD1, d1 | e1 | f1 | g1);
			break;
		case 8:
			GPIO_ResetBits(portD1, a1 | b1 | c1 | d1 | e1 | f1 | g1);
			break;
		case 9:
			GPIO_ResetBits(portD1, a1 | b1 | c1 | d1 | f1 | g1);
			GPIO_SetBits(portD1, e1);
			break;
		case 10:	//wyswietla znak minus na pirwszym polu wywietlacza
			GPIO_ResetBits(portD1, g1);
			GPIO_SetBits(portD1, a1 | b1 | c1 | d1 | e1 | f1);
			break;
		case 11:	//wy³¹cza wywietlacz
			GPIO_SetBits(portD1, a1 | b1 | c1 | d1 | e1 | f1 | g1);
			break;
		}
	}
	else
	{
		switch (cyfra)
		{
		case 0:
			GPIO_ResetBits(portD2, a2 | b2| c2 | d2 | e2 | f2);
			GPIO_SetBits(portD2, g2);
			break;
		case 1:
			GPIO_ResetBits(portD2, b2 | c2);
			GPIO_SetBits(portD2, a2 | d2 | e2 | f2 | g2);
			break;
		case 2:
			GPIO_ResetBits(portD2, a2 | b2  | d2 | e2 | g2);
			GPIO_SetBits(portD2, c2 | f2);
			break;
		case 3:
			GPIO_ResetBits(portD2, a2 | b2 | c2 | d2 | g2);
			GPIO_SetBits(portD2, e2 | f2);
			break;
		case 4:
			GPIO_ResetBits(portD2, b2 | c2 | f2 | g2);
			GPIO_SetBits(portD2, a2 | d2 | e2);
			break;
		case 5:
			GPIO_ResetBits(portD2, a2 | c2 | d2 | f2 | g2);
			GPIO_SetBits(portD2, b2 | e2);
			break;
		case 6:
			GPIO_ResetBits(portD2, a2 | c2 | d2 | e2 | f2 | g2);
			GPIO_SetBits(portD2, b2);
			break;
		case 7:
			GPIO_ResetBits(portD2, a2 | b2 | c2);
			GPIO_SetBits(portD2, d2 | e2 | f2 | g2);
			break;
		case 8:
			GPIO_ResetBits(portD2, a2 | b2 | c2 | d2 | e2 | f2 | g2);
			break;
		case 9:
			GPIO_ResetBits(portD2, a2 | b2 | c2 | d2 | f2 | g2);
			GPIO_SetBits(portD2, e2);
			break;
		case 10:	//wyswietla znak minus na drugim polu wywietlacza
			GPIO_ResetBits(portD2, g2);
			GPIO_SetBits(portD2, a2 | b2 | c2 | d2 | e2 | f2);
			break;
		case 11:	//wy³¹cza drug¹ cyfrê wyswietlacza
			GPIO_SetBits(portD2, a2 | b2 | c2 | d2 | e2 | f2 | g2);
			break;
		}
	}
}

//funkcja zarz¹dzaj¹ca wywietleniem liczby na wywietlaczu zakres -9 do 99
void digits(int liczba)
{
	uint16_t digit1, digit2;
	if (liczba>=100)
	{
		digit(10, 1);
		digit(10, 2);
	}
	if ((liczba>=10) && (liczba<100))
	{
		digit2 = liczba - ((int)(liczba/10))*10;
		digit1 = (int)(liczba/10) - ((int)(liczba/100))*100;
		digit(digit1, 1);
		digit(digit2, 2);
	}
	if ((liczba>=0) && (liczba<=9))
	{
		digit2 = liczba - ((int)(liczba/10))*10;
		digit(11, 1);
		digit(digit2, 2);
	}
	if ((liczba<0) && (liczba>-10))
	{
		digit2=-liczba;
		digit(10, 1);
		digit(digit2, 2);
	}
	if (liczba<=-10)
	{
		digit(10, 1);
		digit(10, 2);
	}
}

//funkcja obs³uguj¹ca encoder
int encoder(void)
	{
	static int j=50;
	const char gray_bin[4] = {2, 3, 1, 0};
	unsigned char in;
	static unsigned char enc_last;
	static unsigned char enc_cnt = 4;

	in = (GPIO_ReadInputData(portEN) & (pinB | pinC)) >> 10; // dopasowac do potrzeb
	in = gray_bin[in];

	if (in == 0)	//jesli obracamy pokrêt³em w prawo
		{
		if (enc_cnt == 1)
		{
			j++;
			if (j==100)
				j=99;
			digits(j);
		}
		if (enc_cnt == 7) 	//jesli krêcimy  pokrêt³em w lewo
		{
			j--;
			if (j==-10)
				j=-9;
			digits(j);
		}
		enc_cnt = 4;
	} else {
		if (in == 3 && enc_last == 0)
			enc_cnt -=1;
		else
			enc_cnt += (in - enc_last);
	}
	enc_last = in;
	return j;
}

//funkcja sprawdzaj¹ca fizyczn¹ obecnosæ termometru
uint16_t RESET_PULSE(void)
{
	uint16_t PRESENCE=0;
	GPIO_ResetBits(portW1, wire1);
	delay(480);
	GPIO_SetBits(portW1, wire1);
	delay(70);
	if (GPIO_ReadInputDataBit(portW1, wire1)==Bit_RESET)
		PRESENCE++;
	delay(410);
	if (GPIO_ReadInputDataBit(portW1, wire1)==Bit_SET)
		PRESENCE++;

	if (PRESENCE == 2)
		return 1;
	else
		return 0;
}

//funkcja wysy³a pojedynczy bit do termometru
void SendBit(uint16_t bit)
{
	if (bit==0)
	{
		GPIO_ResetBits(portW1, wire1);
		delay(65);
		GPIO_SetBits(portW1, wire1);
		delay(10);
	}
	else
	{
		GPIO_ResetBits(portW1, wire1);
		delay(10);
		GPIO_SetBits(portW1, wire1);
		delay(65);
	}
}

//funkcja odbieraj¹ca pojedynczy bit od termometru
uint16_t ReadBit(void)
{
	uint16_t bit=0;
	GPIO_ResetBits(portW1, wire1);
	delay(5);
	GPIO_SetBits(portW1, wire1);
	delay(5);
	if (GPIO_ReadInputDataBit(portW1, wire1)==Bit_SET)
		bit=1;
	else
		bit=0;
	delay(55);
	return bit;
}

//funkcja wysy³aj¹ca ca³y bajt do termometru
void SendByte(uint16_t value)
{
	uint16_t i,tmp;
	for (i = 0; i < 8; i++)
	{
		tmp = value >> i;
		tmp &= 0x01;
		SendBit(tmp);
	}
}

//funkcja odbieraj¹ca ca³y bajt od termometru
uint16_t ReadByte(void)
{
	uint16_t i,value=0;
	for (i = 0; i < 8; i++)
	{
		if(ReadBit()) value |= 0x01 << i;
	}
return value;
}

//funkcja odczytuj¹ca temperature
float ReadTemp(void)
{
	uint16_t i,presence=0, memory[3];
	float temp=0;

	presence=RESET_PULSE();

		if (presence==1)
		{
			SendByte(0xCC);
			SendByte(0x44);

			for (i=0; i < 100; i++)
				delay(7500);
		}
			presence = RESET_PULSE();
		if (presence==1)
		{
			SendByte(0xCC);
			SendByte(0xBE);

			for (i=0;i<2;i++)
			memory[i] = ReadByte();

			memory[2] = (240U & memory[1]) >> 7;
			memory[1] = (15U & memory[1]) << 8;

			if (memory[2] == 0)
				temp = (memory[0] + memory[1])/16;
			if (memory[2] == 1)
				temp = (memory[0] + memory[1]-4095)/16;
		}

		presence = RESET_PULSE();
	return temp;
}

//funkcja obs³uguj¹ca grza³ke w tym przypadku jest to zapalanie i gaszenie diody
void OnOffHeater(void)
{
	if ((((int)temperature)>=t) && (GPIO_ReadOutputDataBit(portHE,heater)==Bit_RESET))
		{
			GPIO_SetBits(portHE,heater); //wylacz grzalke
		}
		else if ((((int)temperature)<=t-2) && (GPIO_ReadOutputDataBit(portHE,heater)==Bit_SET))
		{
			GPIO_ResetBits(portHE,heater);	 //wlacz grzalke
		}
}

//funkcja obs³uguj¹ca  przerwanie od encodera i ustawianie temperatury progowej termostatu
void EXTI9_5_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line9) != RESET)
	{
		uint16_t i,j;
		for (i=0;i<50;i++)
			delay(10000);
		digits(t);

		while(1)
		{
			j=encoder();
			if (GPIO_ReadInputDataBit(portEN,pinA)==Bit_RESET)
			break;
		}

		t=j;
		for (i=0;i<50;i++)
				delay(10000);
		digits((int)temperature);
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
}

//funkcja obs³uguj¹ca przerwanie od SysTick
void SysTick_Handler(void)
{
	static __IO uint16_t count = 3;
	if (++count >= 3)
	{
		temperature = ReadTemp(); //pomiar temperatury
		OnOffHeater();
		if (NVIC_GetActive(EXTI9_5_IRQn) == RESET)	//sprawdzanie czy nie jest aktywne przerwanie od encodera
		digits((int)temperature); //wyswietlenie temperatury
		count = 0;
	}
}
