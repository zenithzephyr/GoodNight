#ifndef __GPIO_SPI_H__
#define __GPIO_SPI_H__

uint8_t SPIReadByte(uint8_t addr);

uint8_t SPIWriteByte(uint8_t addr, uint8_t data);

uint8_t SPIBurstReadByte(uint8_t addr, uint8_t *buf, uint8_t size);
uint8_t SPIBurstWriteByte(uint8_t addr, uint8_t *buf, uint8_t size);

#endif //__GPIO_SPI_H__
