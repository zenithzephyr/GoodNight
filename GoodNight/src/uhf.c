#include <asf.h>

#include <string.h>

#include "uhf.h"
#include "RH_RF22.h"
#include "gpio_spi.h"
#include "GLCD.h" //for test

int8_t x = -10; //offset

#define MAX_SAVE_COUNT	128
struct saved_data_t {
		uint32_t truck_id;
		uint32_t tire_id;
		uint16_t pressure;
		uint8_t voltage;
		uint8_t temperature;
};

struct saved_data_t saved_list[MAX_SAVE_COUNT];

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
		GUI_Text(20, x+50, "Tire ID     :  ",White, Black);
		GUI_Text(20, x+75, "Pressure    :  ",White, Black);
		GUI_Text(20, x+100, "Z-axis      :  ",White, Black);
		GUI_Text(20, x+125, "X-axis      :  ",White, Black);
		GUI_Text(20, x+150, "Voltage     :  ",White, Black);
		GUI_Text(20, x+175, "Temperature :  ",White, Black);
		GUI_Text(20, x+200, "Frame ID    :  ",White, Black);
		GUI_Text(20, x+225, "Sequence    :  ",White, Black);
		GUI_Text(20, x+250, "Status      :  ",White, Black);
		GUI_Text(20, x+275, "RSSI        :  ",White, Black);
}

void uhf_test()
{
	uint32_t tire_id = 0, frame_id = 0;
	uint16_t pressure = 0;
	uint16_t accelZ = 0, accelX = 0;
	uint16_t voltage = 0, temperature = 0, status = 0;
	uint16_t sequence = 0;
	int8_t rssi = 0;

	static uint32_t last_tire_id = 0, last_frame_id = 0;
	static uint16_t last_pressure = 0;
	static uint16_t last_accelZ = 0, last_accelX = 0;
	static uint8_t last_voltage = 0, last_temperature = 0, last_status = 0;
	static uint16_t last_sequence = 0;
	static int8_t last_rssi = 0;

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
			rssi = -120+si4432_rssiRead()/2;

			pressure = ((pressure + 35.3636) * 2.75) * 0.145; //PSI
			temperature = ((temperature - 55) * 1.8) + 32; //F
			voltage = voltage + 122; //10mv

			printf(" TireID[0x%x] Pressure[%d] AccelZ[%d] AccelX[%d] Voltage[%d] Temperature[%d] FrameID[0x%x] RSSI[%d]\r\n",
		tire_id, pressure, accelZ, accelX, voltage, temperature, frame_id, rssi);

		uhf_save_data(tire_id, frame_id, pressure, voltage, temperature);

		//test
		if(tire_id != last_tire_id) {
			sprintf(str_buf, "%X", tire_id);
			GUI_Text(125, x+50, str_buf ,White, Black);
		}
		if(pressure != last_pressure) {
		sprintf(str_buf, "%dPSI  ", pressure);
		GUI_Text(125, x+75, str_buf ,White, Black);
		}
		if(accelZ != last_accelZ) {
		sprintf(str_buf, "%d   ", accelZ);
		GUI_Text(125, x+100, str_buf ,White, Black);
	  }
		if(accelX != last_accelX) {
		sprintf(str_buf, "%d   ", accelX);
		GUI_Text(125, x+125, str_buf ,White, Black);
		}
		if(voltage != last_voltage) {
		sprintf(str_buf, "%dmV   ", voltage*10);
		GUI_Text(125, x+150, str_buf ,White, Black);
		}
		if(temperature != last_temperature) {
		sprintf(str_buf, "%d#F     ", temperature);
		GUI_Text(125, x+175, str_buf ,White, Black);
		}
		if(frame_id != last_frame_id) {
		sprintf(str_buf, "%X", frame_id);
		GUI_Text(125, x+200, str_buf ,White, Black);
		}
		if(sequence != last_sequence) {
		sprintf(str_buf, "%d      ", sequence);
		GUI_Text(125, x+225, str_buf ,White, Black);
		}
		if(status != last_status) {
		sprintf(str_buf, "%d      ", status);
		GUI_Text(125, x+250, str_buf ,White, Black);
		}

		if(rssi != last_rssi) {
		sprintf(str_buf, "%ddBm   ", rssi);
		GUI_Text(125, x+275, str_buf ,White, Black);
	  }

		last_tire_id = tire_id;
		last_pressure = pressure;
		last_accelZ = accelZ;
		last_voltage = voltage;
		last_temperature = temperature;
		last_sequence = sequence;
		last_frame_id = frame_id;
		last_status = status;
		last_rssi = rssi;
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

void uhf_load_data(uint32_t truck_id, uint32_t tire_id, uint16_t *pressure, uint16_t *voltage, uint16_t *temp)
{
	int i;
	for(i=0;i<MAX_SAVE_COUNT;i++) {
		if(saved_list[i].truck_id == truck_id && saved_list[i].tire_id == tire_id) {
			break;
		}
	}
	if(i < MAX_SAVE_COUNT) {
		*pressure = saved_list[i].pressure;
		*voltage = saved_list[i].voltage;
		*temp = saved_list[i].temperature;
	} else {
		*pressure = 0;
		*voltage = 0;
		*temp = 0;
	}
}

void uhf_save_data(uint32_t truck_id, uint32_t tire_id, uint16_t pressure, uint16_t voltage, uint16_t temp)
{
	int i;

	for(i=0;i<MAX_SAVE_COUNT;i++) {
		if(saved_list[i].truck_id == truck_id && saved_list[i].tire_id == tire_id) {
			break;
		}
	}

	if(i < MAX_SAVE_COUNT) {
		saved_list[i].pressure = pressure;
		saved_list[i].voltage = voltage;
		saved_list[i].temperature = temp;
	} else {
		for(i=0;i<MAX_SAVE_COUNT;i++) {
			if(saved_list[i].truck_id == 0 && saved_list[i].tire_id == 0) {
				saved_list[i].pressure = pressure;
				saved_list[i].voltage = voltage;
				saved_list[i].temperature = temp;
				break;
			}
		}
	}
}
