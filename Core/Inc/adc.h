/**
 * @file adc.h
 * @brief ADC Interface for STM32F4 Pulse Sensor Application
 *
 * This header provides function declarations and macro definitions for configuring
 * and operating ADC1 on PA4 for pulse sensor data acquisition. The implementation
 * supports interrupt-driven ADC sampling for real-time pulse detection.
 *
 * @author Kunal
 * @date 2025
 * @version 1.0
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "pulse_sensor.h"

/* Clock Enable Definitions */
/** @brief RCC AHB1ENR bit to enable GPIOA clock */
#define GPIOAEN         (1U<<0)

/** @brief RCC APB2ENR bit to enable ADC1 clock */
#define ADC1EN          (1U<<8)

/** @brief ADC channel selection bit for Channel 1 */
#define ADC_CH1         (1U<<1)

/* Function Prototypes */

/**
 * @brief Initialize ADC1 and configure PA4 for analog input
 *
 * This function performs the following initialization steps:
 * - Enables GPIOA and ADC1 clocks
 * - Configures PA4 as analog input
 * - Sets up ADC1 for 12-bit resolution
 * - Configures channel 4 (PA4) with 144-cycle sampling time
 * - Enables ADC End-of-Conversion interrupt
 * - Sets ADC prescaler to PCLK2/4
 *
 * @note This function must be called before pa4_adc_start()
 * @see pa4_adc_start()
 */
void pa4_adc_init(void);

/**
 * @brief Start continuous ADC conversion on PA4
 *
 * Activates ADC1 and begins continuous conversion mode. The ADC will
 * automatically trigger conversions and generate interrupts on completion.
 * Each conversion result is processed by the ADC IRQ handler.
 *
 * @note Requires pa4_adc_init() to be called first
 * @note ADC stabilization delay (10ms) is included in this function
 * @see pa4_adc_init()
 */
void pa4_adc_start(void);

/**
 * @brief Read the most recent ADC conversion result
 *
 * Returns the last ADC conversion value from the Data Register (DR).
 * In interrupt-driven mode, this value is updated automatically by
 * the ADC IRQ handler after each conversion.
 *
 * @return uint16_t Raw 12-bit ADC value (0-4095)
 * @retval 0-4095 ADC conversion result corresponding to 0V-3.3V input
 *
 * @note In continuous mode, this represents the most recent sample
 * @note Reading DR clears the EOC flag automatically
 */
uint16_t pa4_adc_read(void);

#endif /* INC_ADC_H_ */
