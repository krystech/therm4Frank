#include "i2c.h"

void I2C_init1(void)
{

    I2C_InitTypeDef I2C_InitStruct;

    //enable i2c clock
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    // enable clock on SCL/SDA pins -> GPIOB
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Setup SCL / SDA pins
    * For STM32F103xxx
    *
    * PB6 = i2c-1_SCL
    * PB7 = i2c-2_SDA
    *
    * PB10 = i2c-2_SCL
    * PB11 = i2c-2_SDA
    */



    //GPIO_SetBits(GPIOB,GPIO_Pin_6 | GPIO_Pin_7);

    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStruct);

    I2C_Cmd(I2C1, ENABLE);

}

void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction)
{
 /* Parameters:
 *      I2Cx --> the I2C peripheral e.g. I2C1
 *      address --> the 7 bit slave address
 *      direction --> the transmission direction can be:
 *                      I2C_Direction_Transmitter for Master transmitter mode
 *                      I2C_Direction_Receiver for Master receiver
 */

    // wait till bus is free
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

      // Send I2C1 START condition
    I2C_GenerateSTART(I2Cx, ENABLE);

    // Send start condition
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    // Send slave address for write
    I2C_Send7bitAddress(I2Cx, address, direction);

    /* wait for I2Cx EV6, check if
     * either Slave has acknowledged Master transmitter or
     * Master receiver mode, depending on the transmission
     * direction
     */
    if (direction == I2C_Direction_Transmitter)
        {
            while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
        }
    else if (direction == I2C_Direction_Receiver)
        {
            while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
        }

}

uint8_t I2C_read_ack(I2C_TypeDef* I2Cx)
{
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));

    uint8_t data = I2C_ReceiveData(I2Cx);
    return data;
}

uint8_t I2C_read_nack(I2C_TypeDef* I2Cx)
{
    // disable acknowledge of received data
    // nack also generates stop condition after last byte received
    // see reference manual for more info
    I2C_AcknowledgeConfig(I2Cx, DISABLE);
    I2C_GenerateSTOP(I2Cx, ENABLE);
    // wait until one byte has been received
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
    // read data from I2C data register and return data byte
    uint8_t data = I2C_ReceiveData(I2Cx);
    return data;
}

void I2C_stop(I2C_TypeDef* I2Cx)
{
        // Wait until end of transmission
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
    // Send I2C1 STOP Condition after last byte has been transmitted
    I2C_GenerateSTOP(I2Cx, ENABLE);
    // wait for I2C1 EV8_2 --> byte has been transmitted
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

void I2C_write(I2C_TypeDef* I2Cx, uint8_t data)
{
/* This function transmits one byte to the slave device
 * Parameters:
 *      I2Cx --> the I2C peripheral e.g. I2C1
 *      data --> the data byte to be transmitted
 */

    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
    I2C_SendData(I2Cx, data);
}
