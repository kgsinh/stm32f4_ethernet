#include "adc.h"

void pa4_adc_init(void)
{
	/* Enable clock for GPIOA */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set PA4 to analog mode */
	GPIOA->MODER |= (3U << (4 * 2)); // MODER1[1:0] = 11 (Analog mode)

	/* Enable clock for ADC1 */
	RCC->APB2ENR |= ADC1EN;

	/* ADC Prescaler configuration: PCLK2 divided by 4 */
	ADC->CCR &= ~(3U << 16); // Clear bits 17:16
	ADC->CCR |= (1U << 16); // Set bits 17:16 to 01

	/* Sample time configuration for channel 1 = 144 cycles */
	ADC1->SMPR2 &= ~(0x7 << 12); // Clear SMP1[2:0]
	ADC1->SMPR2 |= (5U << (3 * 4)); // SMP1[2:0] = 101 (144 cycles)

	/* Configure ADC1 */
	ADC1->SQR1 = 0;    // Set sequence length to 1
	ADC1->SQR3 = 4;          // Set channel 1 (PA1) as the first conversion in regular sequence

	/* Enable EOC interrupt */
	ADC1->CR1 |= ADC_CR1_EOCIE;

	/* Enable ADC IRQ in NVIC */
	NVIC_SetPriority(ADC_IRQn, 1);
	NVIC_EnableIRQ(ADC_IRQn);

	/* Turn on ADC1 */
	ADC1->CR2 |= ADC_CR2_ADON;

	HAL_Delay(10); // Wait for ADC to stabilize
}

void pa4_adc_start(void)
{
	/* Start continuous conversion */
	ADC1->CR2 |= ADC_CR2_CONT;

	/* Start the conversion */
	ADC1->CR2 |= ADC_CR2_SWSTART;
}

void ADC_IRQHandler(void)
{
	if (ADC1->SR & ADC_SR_EOC) // Check if EOC flag is set
	{
		uint16_t adc_value = ADC1->DR; // Read the converted value
		pulse_sensor_process(adc_value); // Process the ADC value

		ADC1->SR &= ~ADC_SR_EOC; // Clear EOC flag
	}
}

