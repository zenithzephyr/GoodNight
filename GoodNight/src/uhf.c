#include <asf.h>

#include <string.h>

#include "uhf.h"
#include "RH_RF22.h"
#include "gpio_spi.h"
#include "GLCD.h" //for test

static void UHF_IRQ_Handler(const uint32_t id, const uint32_t index)
{
	uint32_t status;
	status = pio_get_interrupt_status(PIOA);
	//printf("UHF DI Interrupt (%lu)\r\n", status);

	if ((id == ID_PIOA) && (index == (1 << UHF_NIRQ_GPIO))){
		if (pio_get(PIOA, PIO_TYPE_PIO_INPUT, (1 << UHF_NIRQ_GPIO))) {
			//printf("1");
		//pio_clear(PIOA, PIO_PA23);
		} else {
			//printf("0");
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

	//for(int i=0;i<0x7F;i++)
		//printf("%x %x\r\n",i, SPIReadByte(i));
//LCD Display
		GUI_Text(20, 50, "Tire ID     :  ",White, Black);
		GUI_Text(20, 75, "Pressure    :  ",White, Black);
		GUI_Text(20, 100, "Z-axis      :  ",White, Black);
		GUI_Text(20, 125, "X-axis      :  ",White, Black);
		GUI_Text(20, 150, "Voltage     :  ",White, Black);
		GUI_Text(20, 175, "Temperature :  ",White, Black);
		GUI_Text(20, 200, "Frame ID    :  ",White, Black);
		GUI_Text(20, 225, "Sequence    :  ",White, Black);
		GUI_Text(20, 250, "Status      :  ",White, Black);
}

void uhf_test()
{
	uint32_t tire_id = 0, frame_id = 0;
	uint16_t pressure = 0;
	uint16_t accelZ = 0, accelX = 0;
	uint8_t voltage = 0, temperature = 0, status = 0;
	uint16_t sequence = 0;
	char str_buf[32];

	if (si4432_available()) {
    // Should be a message for us now
    uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (si4432_recv(buf, &len)) {
      printf("RECV: ");
			tire_id = buf[1] <<24 | buf[2] <<16 | buf[3] <<8 | buf[4];
			pressure = buf[5] << 8 | buf[6];
			accelZ = buf[7] << 8 | buf[8];
			accelX = buf[9] << 8 | buf[10];
			voltage = buf[11];
			temperature = buf[12];
			sequence = buf[14] << 8 | buf[15];
			frame_id = buf[18] <<24 | buf[19] <<16 | buf[20] <<8 | buf[21];
			status = buf[13];
			printf(" TireID[0x%x] Pressure[%d] AccelZ[%d] AccelX[%d] Voltage[%d] Temperature[%d] FrameID[0x%x]\r\n",
		tire_id, pressure, accelZ, accelX, voltage, temperature, frame_id);

		//test
		sprintf(str_buf, "%X", tire_id);
		GUI_Text(125, 50, str_buf ,White, Black);
		sprintf(str_buf, "%d   ", pressure);
		GUI_Text(125, 75, str_buf ,White, Black);
		sprintf(str_buf, "%d   ", accelZ);
		GUI_Text(125, 100, str_buf ,White, Black);
		sprintf(str_buf, "%d   ", accelX);
		GUI_Text(125, 125, str_buf ,White, Black);
		sprintf(str_buf, "%d   ", voltage);
		GUI_Text(125, 150, str_buf ,White, Black);
		sprintf(str_buf, "%d      ", temperature);
		GUI_Text(125, 175, str_buf ,White, Black);
		sprintf(str_buf, "%X", frame_id);
		GUI_Text(125, 200, str_buf ,White, Black);
		sprintf(str_buf, "%d      ", sequence);
		GUI_Text(125, 225, str_buf ,White, Black);
		sprintf(str_buf, "%d      ", status);
		GUI_Text(125, 250, str_buf ,White, Black);
#if 0
      for(int i=0;i<len;i++) {
        printf("%X ", buf[i]);
      }
      printf("\r\n");
#endif
    //  printf("RSSI: %d\r\n", si4432_lastRssi());
    } else {
      printf("recv failed");
    }
  }
}
