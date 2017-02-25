#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#define SPI_FLASH_CMD_READ_JEDEC_ID   0x9F
#define SPI_FLASH_CMD_PROG 0x02
#define SPI_FLASH_CMD_READ 0x03
#define SPI_FLASH_CMD_SEC_ERASE 0x20

void spi_flash_read_jedec(uint8_t *buf);

void spi_flash_init(void);

void spi_flash_read();
void spi_flash_write();
void spi_test(void);

//TODO : flash read, flash write

#endif //__SPI_FLASH_H__
