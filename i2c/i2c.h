#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include <stdio.h>

void I2C_init1(void);
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);
void I2C_stop(I2C_TypeDef* I2Cx);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);

#endif /* I2C_H_INCLUDED */
