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

#include "wifi.h"
#include "uhf.h"
#include "spi_flash.h"

#include "GLCD.h"

#define ADC_CLOCK 6000000

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
		wifi_recv_data(&rx_data, 1);
		//uart_write(UART0, rx_data); //send data
#endif
	}
}

/** configures */
static void configure_led(void)
{
	//LED Reset
	gpio_set_pin_low(LED0_GPIO);
	gpio_set_pin_high(LED1_GPIO);
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

	spi_flash_init();

	wifi_init();

	LCD_Initializtion();

	uhf_init();

	configure_adc();	printf("ADC Configured.\r\n");	configure_led();	printf("LED Configured.\r\n");	// ADC Start	//adc_start(ADC);
	//wifi_test();
	//spi_test();
	//uhf_test();

	while(1) {
		//spi_test();
		//led_test();
		wifi_parse_buf();
		uhf_test();
		//delay_s(1);
	}
}
