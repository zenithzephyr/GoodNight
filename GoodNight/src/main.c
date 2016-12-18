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

#define ADC_CLOCK 6000000

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
}

void UART1_Handler(void)
{
	uint8_t rx_data;
	uint32_t status = uart_get_status(UART1);
	
	if(status & UART_SR_RXRDY){
		//read
		uart_read(UART1, &rx_data);
		
		//reply back the same
		while (!(UART1->UART_SR & UART_SR_TXRDY));
		uart_write(UART1, rx_data); //send data
	}
}

void ADC_Handler(void)
{
	// Check the ADC conversion status
	if ((adc_get_status(ADC) & ADC_ISR_DRDY) == ADC_ISR_DRDY)
	{
		// Get latest digital data value from ADC and can be used by application
		uint32_t result = adc_get_latest_value(ADC);
		printf("ADC result = %x\r\n", (unsigned int)result);
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

static void adc_setup(void)
{
	adc_init(ADC, sysclk_get_main_hz(), ADC_CLOCK, 8);
	adc_configure_timing(ADC, 0, ADC_SETTLING_TIME_3, 1);
	adc_set_resolution(ADC, ADC_MR_LOWRES_BITS_12);
	adc_enable_channel(ADC, ADC_CHANNEL_0);
	adc_enable_channel(ADC, ADC_CHANNEL_1);
	adc_enable_channel(ADC, ADC_CHANNEL_2);
	adc_enable_channel(ADC, ADC_CHANNEL_3);
	adc_enable_channel(ADC, ADC_CHANNEL_4);
	adc_enable_channel(ADC, ADC_CHANNEL_5);
	adc_enable_channel(ADC, ADC_CHANNEL_8);
	adc_enable_channel(ADC, ADC_CHANNEL_9); //ABS

	adc_enable_interrupt(ADC, ADC_IER_DRDY);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0);
}
int main (void)
{
	sysclk_init();

	board_init();

	delay_init(sysclk_get_cpu_hz());

	rtt_init(RTT, 1);

	configure_console();

	printf("GoodNight Service Started.\r\n");

	configure_wifi();

	adc_setup();	adc_start(ADC);
	gpio_set_pin_low(LED0_GPIO);
	gpio_set_pin_high(LED1_GPIO);

	//UHF DI
	gpio_configure_pin(UHF_DO_GPIO, UHF_DO_FLAGS | PIO_PULLUP);
	pio_handler_set(PIOA, ID_PIOA, (1 << UHF_DO_GPIO), PIO_IT_EDGE, UHF_DI_Handler);
	pio_enable_interrupt(PIOA, (1 << UHF_DO_GPIO));
	NVIC_EnableIRQ(PIOA_IRQn);

	
	while(1) {
		gpio_toggle_pin(LED0_GPIO);
		printf("LED0 Toggle\r\n");
		usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"LED0 Toggle\r\n", 13);
		gpio_toggle_pin(LED1_GPIO);
		printf("LED1 Toggle\r\n");
		usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"LED1 Toggle\r\n", 13);
		rtt_value = rtt_read_timer_value(RTT);
		printf("RTT = %lu\r\n", rtt_value);
		delay_s(1);
	}
}
