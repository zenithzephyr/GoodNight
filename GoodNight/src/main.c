/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

#include "GLCD.h"

#define ADC_CLOCK 6000000

//SPI
struct spi_device spi_device_conf = {
	.id = 0
};

/** Interrupt Handlers */
void ADC_Handler(void)
{
	// Check the ADC conversion status
	uint32_t status;
	uint32_t result;

	status = adc_get_status(ADC);

	if(status & (1 << ADC_CHANNEL_0)) {
		result = adc_get_channel_value(ADC, ADC_CHANNEL_0);
		printf("ADC0 result = %x\t", (unsigned int)result);
	}

	if(status & (1 << ADC_CHANNEL_1)) {
		result = adc_get_channel_value(ADC, ADC_CHANNEL_1);
		printf("ADC1 result = %x\t", (unsigned int)result);
	}
	if(status & (1 << ADC_CHANNEL_2)) {
		result = adc_get_channel_value(ADC, ADC_CHANNEL_2);
		printf("ADC2 result = %x\t", (unsigned int)result);
	}
	if(status & (1 << ADC_CHANNEL_3)) {
		result = adc_get_channel_value(ADC, ADC_CHANNEL_3);
		printf("ADC3 result = %x\t", (unsigned int)result);
	}
	if(status & (1 << ADC_CHANNEL_4)) {
		result = adc_get_channel_value(ADC, ADC_CHANNEL_4);
		printf("ADC4 result = %x\t", (unsigned int)result);
	}
	if(status & (1 << ADC_CHANNEL_5)) {
		result = adc_get_channel_value(ADC, ADC_CHANNEL_5);
		printf("ADC5 result = %x\t", (unsigned int)result);
	}
	if(status & (1 << ADC_CHANNEL_8)) {
		result = adc_get_channel_value(ADC, ADC_CHANNEL_8);
		printf("ADC8 result = %x\t", (unsigned int)result);
	}
	if(status & (1 << ADC_CHANNEL_9)) {
		result = adc_get_channel_value(ADC, ADC_CHANNEL_9);
		printf("ADC9 result = %x\t", (unsigned int)result);
	}

	printf("\r\n");
}

void SysTick_Handler(void)
{
	//TODO: FIXME
	//uint32_t status;
	//status = adc_get_status(ADC);
	//printf("ADC Status(%x)\r\n", status);
	//if(adc_get_status(ADC) & (1 << ADC_CHANNEL_0)) {
	//adc_start(ADC);
	//	printf("Start ADC\r\n");
	//}
}

void UART1_Handler(void)
{
	uint8_t rx_data;
	uint32_t status = uart_get_status(UART1);

	if(status & UART_SR_RXRDY){
		//read
		uart_read(UART1, &rx_data);
		//reply back the same
#if 0
		while (!(UART1->UART_SR & UART_SR_TXRDY));
		uart_write(UART1, rx_data); //send data
#else
		while (!(UART0->UART_SR & UART_SR_TXRDY));
		uart_write(UART0, rx_data); //send data
#endif
	}
}

static void UHF_DI_Handler(const uint32_t id, const uint32_t index)
{
	uint32_t status;
	status = pio_get_interrupt_status(PIOA);
	printf("UHF DI Interrupt (%lu)\r\n", status);

	if ((id == ID_PIOA) && (index == (1 << UHF_DO_GPIO))){
		//if (pio_get(PIOA, PIO_TYPE_PIO_INPUT, (1 << UHF_DO_GPIO)))
		//pio_clear(PIOA, PIO_PA23);
		//else
		//pio_set(PIOA, PIO_PA23);
	}
}

/** configures */
static void configure_led(void)
{
	//LED Reset
	gpio_set_pin_low(LED0_GPIO);
	gpio_set_pin_high(LED1_GPIO);
}

static void configure_spi(void)
{
	//Init SPI module as master
	spi_master_init(SPI);

	 //Setup parameters for the slave device
	spi_master_setup_device(SPI, &spi_device_conf, SPI_MODE_0, 50000000, 0); //50MHz

	//Allow the module to transfer data
	spi_enable(SPI);
}

static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

static void configure_wifi(void)
{
	uint8_t atstart[13] = {0xA5, 0x01, 0xF2, 0x00, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x61, 0x2F, 0x52, 0xEF};

	usart_serial_options_t usart_options = {
		.baudrate = CONF_WIFI_UART_BAUDRATE,
		#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_WIFI_UART_PARITY,
		#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};
	/* Configure console UART. */
	usart_serial_init((usart_if)CONF_WIFI_UART, &usart_options);

	uart_enable_interrupt(UART1,UART_IER_RXRDY);
	NVIC_EnableIRQ(UART1_IRQn);

	//reset pins
	gpio_set_pin_low(WIFI_WAKE_GPIO);
	gpio_set_pin_low(WIFI_RESET_GPIO);
	gpio_set_pin_low(WIFI_EN_GPIO);
	delay_ms(100);
	gpio_set_pin_high(WIFI_WAKE_GPIO);
	gpio_set_pin_high(WIFI_EN_GPIO);
	delay_ms(10);
	gpio_set_pin_high(WIFI_RESET_GPIO);
	delay_ms(100);

	//Start AT Command
	usart_serial_write_packet((usart_if)CONF_WIFI_UART,atstart, 13);
	printf("Send ATCommand Start\r\n");


}

static void configure_adc(void)
{
	pmc_enable_periph_clk(ID_ADC);
	adc_init(ADC, sysclk_get_cpu_hz(), ADC_CLOCK, ADC_STARTUP_TIME_15);
	adc_configure_timing(ADC, 1, ADC_SETTLING_TIME_3, 1);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0);

	adc_check(ADC, sysclk_get_cpu_hz());
	//adc_set_resolution(ADC, ADC_MR_LOWRES_BITS_12);

	adc_enable_channel(ADC, ADC_CHANNEL_0);
	adc_enable_channel(ADC, ADC_CHANNEL_1);
	adc_enable_channel(ADC, ADC_CHANNEL_2);
	adc_enable_channel(ADC, ADC_CHANNEL_3);
	adc_enable_channel(ADC, ADC_CHANNEL_4);
	adc_enable_channel(ADC, ADC_CHANNEL_5);
	adc_enable_channel(ADC, ADC_CHANNEL_8);
	adc_enable_channel(ADC, ADC_CHANNEL_9); //ABS

	adc_enable_interrupt(ADC, ADC_ISR_EOC0|ADC_ISR_EOC1|ADC_ISR_EOC2|ADC_ISR_EOC3|ADC_ISR_EOC4|ADC_ISR_EOC5|ADC_ISR_EOC8|ADC_ISR_EOC9);
	NVIC_EnableIRQ(ADC_IRQn);
}

/** test functions */
static void uhf_test()
{
	//UHF DI
	gpio_configure_pin(UHF_DO_GPIO, UHF_DO_FLAGS | PIO_PULLUP);
	pio_handler_set(PIOA, ID_PIOA, (1 << UHF_DO_GPIO), PIO_IT_EDGE, UHF_DI_Handler);
	pio_enable_interrupt(PIOA, (1 << UHF_DO_GPIO));
}

static void wifi_test()
{
	delay_ms(1000);
	usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"AT\r\n", 4);
	delay_ms(1000);
	usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"AT+LIST\r\n", 9);
	delay_ms(1000);
	usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"AT+CHIPID\r\n",11);
	delay_ms(1000);
	usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"AT+AP=WINC1500,0,1\r\n",20);
}

static void led_test()
{
		gpio_toggle_pin(LED0_GPIO);
		printf("LED0 Toggle\r\n");
		usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"LED0 Toggle\r\n", 13);
		gpio_toggle_pin(LED1_GPIO);
		printf("LED1 Toggle\r\n");
		usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"LED1 Toggle\r\n", 13);
}

static void rtt_test()
{
	uint32_t rtt_value; //used in UHF, UWAVE (30uS resolution)

	rtt_value = rtt_read_timer_value(RTT);
	printf("RTT = %lu\r\n", rtt_value);
}

static void spi_test()
{
	int i;
	//Buffer to send data to SPI slave
	uint8_t txdata[4]={0x90, 0x00, 0x00, 0x00};
	//Buffer to receive data from SPI slave
	uint8_t rxdata[128] = {0, };

	// Select the slave device with chip select
	spi_select_device(SPI,&spi_device_conf);
	// Send the data to slave
	spi_write_packet(SPI, txdata, 4);
	// Read data from slave
	spi_read_packet(SPI, rxdata,2);

	printf("Read Device ID : 0x%X 0x%X\r\n", rxdata[0], rxdata[1]);
	spi_deselect_device(SPI,&spi_device_conf);

	delay_ms(10);

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
}

/** main */
int main (void)
{
	sysclk_init();

	board_init();

	delay_init(sysclk_get_cpu_hz());

	configure_console();
	printf("GoodNight Service Started.\r\n");

	if (SysTick_Config(sysclk_get_cpu_hz() / 10)) { //100ms
		printf("-F- Systick configuration error\r\n");
		while(1) {
		}
	}
	printf("SysTick Configured.\r\n");

	rtt_init(RTT, 1);
	printf("RTT Initialized.\r\n");

	configure_wifi();
	printf("Wifi UART Configured.\r\n");

	configure_adc();	printf("ADC Configured.\r\n");	configure_spi();	printf("SPI Configured.\r\n");	configure_led();	printf("LED Configured.\r\n");	// ADC Start	//adc_start(ADC);

	wifi_test();
	//spi_test();
	//LCD_Test();
	while(1) {
		spi_test();
		//led_test();
		delay_s(1);
	}
}
