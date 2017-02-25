#include <asf.h>
#include <string.h>

#include "spi_flash.h"
//SPI
struct spi_device spi_device_conf = {
	.id = 0
};

struct flash_info_t {
    char name[16];
    uint8_t jedec_id[6]; //INFO6
    uint16_t sector_size; //erase
    uint16_t sectors;   //erase
    uint16_t page_size; //write
};

static struct flash_info_t spi_flash_ids[] = {
  {"w25q64", {0xEF, 0x40, 0x17,}, 4096, 2048, 256},
};

uint16_t flash_info_cnt = sizeof(spi_flash_ids)/sizeof(struct flash_info_t);

struct flash_info_t *g_flash;

void spi_flash_init(void)
{
	int i;
	//Init SPI module as master
	spi_master_init(SPI);

	 //Setup parameters for the slave device
	spi_master_setup_device(SPI, &spi_device_conf, SPI_MODE_0, 50000000, 0); //50MHz

	//Allow the module to transfer data
	spi_enable(SPI);

  uint8_t jedec_id[4] = {0, };

  spi_flash_read_jedec(jedec_id);

	printf("Read JEDEC ID : 0x%02X 0x%02X 0x%02X\r\n", jedec_id[0], jedec_id[1], jedec_id[2]);

  for(i=0;i<flash_info_cnt;i++) {
    if(memcmp(jedec_id, spi_flash_ids[i].jedec_id, 3) == 0)
      break;
  }

  if(i < flash_info_cnt) {
    g_flash = &spi_flash_ids[i];
    printf("SPI Flash Device[%s] Initialized\r\n", g_flash->name);
  } else {
    printf("Unknown Device\r\n");
  }
}

void spi_flash_read_jedec(uint8_t *buf)
{
	uint8_t txdata = SPI_FLASH_CMD_READ_JEDEC_ID;

	spi_select_device(SPI,&spi_device_conf);

	spi_write_packet(SPI, &txdata, 1);
	spi_read_packet(SPI, buf,3);

	spi_deselect_device(SPI,&spi_device_conf);
}

void spi_test()
{
	int i;

#if 0
	spi_select_device(SPI,&spi_device_conf);
	//Read Data
	txdata[0] = 0x03;
	spi_write_packet(SPI, txdata, 4);
	// Read data from slave
	spi_read_packet(SPI, rxdata,128);
	for(i=0;i<128;i++)
	printf("%X ",rxdata[i]);
	printf("\r\n");

	// Deselect the slave
	spi_deselect_device(SPI,&spi_device_conf);
#endif
}
