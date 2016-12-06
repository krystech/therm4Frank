/*
 * main.h
 *
 *  Created on: Oct 15, 2011
 *      Author: Marcin
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stddef.h>
#include "stm32f10x.h"

#define portW1 GPIOA	//port do magistrali 1-wire
#define portD1 GPIOA	//port do pierwszej liczby wyswietlacza
#define portD2 GPIOB	//port do drugiej liczby wyswietlacza
#define portHE GPIOB	//port do obs³ugi grza³ki
#define portEN GPIOA	//port do encodera

#define wire1 GPIO_Pin_7	//pin do 1-wire
#define heater GPIO_Pin_1	//pin do grza³ki

//piny obs³uguj¹ce encoder
#define pinA GPIO_Pin_9
#define pinB GPIO_Pin_10
#define pinC GPIO_Pin_11

//piny obs³uguj¹ce poszczególne segmenty drugiej cyfry wyswietlacza
#define a2 GPIO_Pin_13
#define b2 GPIO_Pin_14
#define c2 GPIO_Pin_11
#define d2 GPIO_Pin_9
#define e2 GPIO_Pin_8
#define f2 GPIO_Pin_12
#define g2 GPIO_Pin_10

//piny obs³uguj¹ce poszczególne segmenty pierwszej cyfry wyswietlacza
#define a1 GPIO_Pin_5
#define b1 GPIO_Pin_6
#define c1 GPIO_Pin_2
#define d1 GPIO_Pin_1
#define e1 GPIO_Pin_0
#define f1 GPIO_Pin_3
#define g1 GPIO_Pin_4

int encoder(void);
void delay(uint16_t time);
void digit(uint16_t cyfra,uint16_t digit);
void digits(int liczba);
uint16_t RESET_PULSE(void);
void SendBit(uint16_t bit);
uint16_t ReadBit(void);
void SendByte(uint16_t value);
uint16_t ReadByte(void);
float ReadTemp(void);
void OnOffHeater(void);

#endif /* MAIN_H_ */
