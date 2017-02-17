#include <asf.h>

#define GPIO_SCLK UHF_SCLK_GPIO
#define GPIO_MOSI UHF_SDO_GPIO
#define GPIO_MISO UHF_SDI_GPIO
#define GPIO_NCS  UHF_NSEL_GPIO

#define TEN_DELAY 1 //20nsec
#define TCL_DELAY 1 //40nsec
#define TCH_DELAY 1 //40nsec
#define TDS_DELAY 1 //20nsec
#define TSH_DELAY 1 //50nsec
#define TSW_DELAY 1 //80nsec

void SPIDelay(uint32_t delay)
{
    delay_us(delay);
}

uint8_t SPIGPIOTxRx(uint8_t addr, uint8_t data)
{
    uint8_t i, rx, mask;
    uint16_t wdata, wmask;

    rx = 0;
    mask = 0x80;
    wmask = 0x8000;

//  gpio_set_pin_low(GPIO_NCS);
//  SPIDelay(TEN_DELAY);
    gpio_set_pin_low(GPIO_SCLK);

    if(addr & 0x80) {
      wdata = (uint16_t)(addr << 8) | data;
      for(i=0;i<16;i++) {
        if(wdata & wmask)
          gpio_set_pin_high(GPIO_MOSI);
        else
          gpio_set_pin_low(GPIO_MOSI);
        SPIDelay(TDS_DELAY);

        gpio_set_pin_high(GPIO_SCLK);
        SPIDelay(TCH_DELAY);

#if 0
        if(gpio_pin_is_high(GPIO_MISO))
          rx |= wmask;
        else
          rx &= ~wmask;
#endif

        wmask >>= 1;
        gpio_set_pin_low(GPIO_SCLK);
        SPIDelay(TCL_DELAY);
      }
    } else {
      for(i=0;i<8;i++) {
        if(addr & mask)
          gpio_set_pin_high(GPIO_MOSI);
        else
          gpio_set_pin_low(GPIO_MOSI);
        SPIDelay(TDS_DELAY);

        gpio_set_pin_high(GPIO_SCLK);
        SPIDelay(TCH_DELAY);

        if(gpio_pin_is_high(GPIO_MISO))
          rx |= mask;
        else
          rx &= ~mask;

        mask >>= 1;
        gpio_set_pin_low(GPIO_SCLK);
        SPIDelay(TCL_DELAY);
      }
    }

    //SPIDelay(TSH_DELAY);
    //gpio_set_pin_high(GPIO_NCS);
    //SPIDelay(TSW_DELAY);
    gpio_set_pin_low(GPIO_SCLK);
    gpio_set_pin_low(GPIO_MOSI);

    return rx;
}

uint8_t SPIReadByte(uint8_t addr)
{
  uint8_t dummy_data;
  addr &= 0x7F;

  gpio_set_pin_low(GPIO_NCS);
  SPIDelay(TEN_DELAY);

  dummy_data = SPIGPIOTxRx(addr, 0);
  dummy_data = SPIGPIOTxRx(addr, 0);

  SPIDelay(TSH_DELAY);
  gpio_set_pin_high(GPIO_NCS);
  SPIDelay(TSW_DELAY);

  printf("READ %02x : %02x\r\n", addr, dummy_data);
  return dummy_data;
}

uint8_t SPIWriteByte(uint8_t addr, uint8_t data)
{
  uint8_t dummy_data;

  printf("WRITE %02x : %02x\r\n", addr, data);

  gpio_set_pin_low(GPIO_NCS);
  SPIDelay(TEN_DELAY);
  addr |= 0x80;

  dummy_data = SPIGPIOTxRx(addr, data);

  SPIDelay(TSH_DELAY);
  gpio_set_pin_high(GPIO_NCS);
  SPIDelay(TSW_DELAY);


  return dummy_data;
}

uint8_t SPIBurstReadByte(uint8_t addr, uint8_t *buf, uint8_t size)
{
  for(int i=0;i<size;i++) {
    *buf++ = SPIReadByte(addr++);
  }
}

uint8_t SPIBurstWriteByte(uint8_t addr, uint8_t *buf, uint8_t size)
{
  for(int i=0;i<size;i++) {
    SPIWriteByte(addr++, buf[i]);
  }
}
