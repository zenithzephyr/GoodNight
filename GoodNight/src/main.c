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

void ADC_IrqHandler(void)
{
	// Check the ADC conversion status
	if ((adc_get_status(ADC) & ADC_ISR_DRDY) == ADC_ISR_DRDY)
	{
		// Get latest digital data value from ADC and can be used by application
		uint32_t result = adc_get_latest_value(ADC);
	}
}
void adc_setup(void)
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

	adc_setup();	adc_start(ADC);
	gpio_set_pin_low(LED0_GPIO);
	gpio_set_pin_high(LED1_GPIO);

	while(1) {
		gpio_toggle_pin(LED0_GPIO);
		gpio_toggle_pin(LED1_GPIO);
		delay_s(1);
	}
}
