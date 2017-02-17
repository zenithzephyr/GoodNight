#include <asf.h>

#include "uhf.h"
#include "RH_RF22.h"
#include "gpio_spi.h"

static void UHF_IRQ_Handler(const uint32_t id, const uint32_t index)
{
	uint32_t status;
	status = pio_get_interrupt_status(PIOA);
	//printf("UHF DI Interrupt (%lu)\r\n", status);

	if ((id == ID_PIOA) && (index == (1 << UHF_NIRQ_GPIO))){
		if (pio_get(PIOA, PIO_TYPE_PIO_INPUT, (1 << UHF_NIRQ_GPIO)))
			printf("1");
		//pio_clear(PIOA, PIO_PA23);
		else {
			printf("0");
			si4432_handleInterrupt();
		//pio_set(PIOA, PIO_PA23);
		}
	}
}

void uhf_init(void)
{
	//INIT SPI
	gpio_configure_pin(UHF_NIRQ_GPIO, UHF_NIRQ_FLAGS | PIO_PULLUP);
	pio_handler_set(PIOA, ID_PIOA, (1 << UHF_NIRQ_GPIO), PIO_IT_EDGE, UHF_IRQ_Handler);
	pio_enable_interrupt(PIOA, (1 << UHF_NIRQ_GPIO));

  NVIC_EnableIRQ(PIOA_IRQn);

	gpio_set_pin_high(UHF_NSEL_GPIO);
	gpio_set_pin_low(UHF_SDO_GPIO);
	gpio_set_pin_low(UHF_SCLK_GPIO);

	delay_ms(100);
	si4432_init();
	//SPIWriteByte(0x07, 0x80); //Soft RESET
	delay_ms(100);

  printf("UHF Initialized\r\n");

	for(int i=0;i<0x7F;i++)
		printf("[%02x %02x]\r\n",i,SPIReadByte(i));

}

void uhf_test()
{
	if (si4432_available()) {
    // Should be a message for us now
    uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (si4432_recv(buf, &len)) {
      printf("RECV: ");
      for(int i=0;i<len;i++) {
        printf("%X ", buf[i]);
      }
      printf("\r\n");
    //  printf("RSSI: %d\r\n", si4432_lastRssi());
    } else {
      printf("recv failed");
    }
  }
}
