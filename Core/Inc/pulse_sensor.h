/**
 * @file pulse.h
 * @brief Advanced Pulse Sensor Module for Real-Time Heart Rate Monitoring
 *
 * This module provides comprehensive pulse detection and heart rate analysis
 * for STM32F4-based systems. It implements an adaptive threshold algorithm
 * with hysteresis filtering, signal quality assessment, and robust BPM
 * calculation suitable for wearable health monitoring applications.
 *
 * Features:
 * - Adaptive pulse threshold with hysteresis
 * - Signal quality scoring (0-100)
 * - Physiologically-bound BPM validation (40-180 BPM)
 * - Debounce protection against false detections
 * - Rolling signal amplitude tracking
 *
 * @author Kunal
 * @date 2025
 * @version 1.0
 */

#ifndef INC_PULSE_SENSOR_H
#define INC_PULSE_SENSOR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"

//==============================================================================
// PULSE DETECTION CONFIGURATION
//==============================================================================

/** @brief Threshold ratio for pulse detection relative to signal amplitude */
#define PULSE_THRESHOLD_RATIO 0.7f

/** @brief Hysteresis buffer to prevent noise-induced false triggers */
#define PULSE_HYSTERESIS 50

/** @brief Number of beat intervals stored for BPM averaging */
#define BEAT_HISTORY_SIZE 6

/** @brief Size of rolling signal history buffer */
#define SIGNAL_HISTORY_SIZE 20

/** @brief Minimum beat interval (ms) - corresponds to 180 BPM maximum */
#define MIN_BEAT_INTERVAL_MS 333

/** @brief Maximum beat interval (ms) - corresponds to 40 BPM minimum */
#define MAX_BEAT_INTERVAL_MS 1500

/** @brief Minimum signal amplitude required for valid pulse detection */
#define MIN_SIGNAL_AMPLITUDE 20

/** @brief Debounce time to prevent rapid false pulse detections */
#define DEBOUNCE_TIME_MS 100

/** @brief Interval for resetting signal min/max tracking */
#define SIGNAL_UPDATE_INTERVAL_MS 2000

//==============================================================================
// CORE PULSE PROCESSING FUNCTIONS
//==============================================================================

/**
 * @brief Initialize the pulse sensor module
 *
 * Resets all internal state variables, clears beat history, and prepares
 * the module for pulse detection. Must be called before any other pulse
 * sensor functions.
 *
 * @note This function should be called during system initialization
 */
void pulse_sensor_init(void);

/**
 * @brief Process new ADC sample for pulse detection
 *
 * Analyzes incoming ADC data to detect pulse events using adaptive
 * thresholding with hysteresis. Updates internal signal tracking,
 * beat timing, and triggers BPM recalculation when beats are detected.
 *
 * @param adc_value Raw ADC reading from pulse sensor (0-4095 for 12-bit ADC)
 *
 * @note Call this function for each new ADC sample, typically at 100Hz or higher
 */
void pulse_sensor_process(uint16_t adc_value);

/**
 * @brief Check if a new pulse beat was detected
 *
 * Returns the beat detection status from the last call to pulse_sensor_process().
 * This flag is automatically cleared after being read.
 *
 * @return true if a new beat was detected since last check
 * @return false if no new beat detected
 *
 * @note Use immediately after pulse_sensor_process() for real-time beat events
 */
bool pulse_sensor_beat_detected(void);

//==============================================================================
// BPM CALCULATION AND VALIDATION
//==============================================================================

/**
 * @brief Get current calculated BPM value
 *
 * Returns the beats per minute calculated from recent beat interval history.
 * BPM is computed using a rolling average of the most recent beat intervals
 * to provide stable readings.
 *
 * @return Current BPM value (40-180 range when valid, 0 if insufficient data)
 *
 * @note Always check pulse_sensor_bpm_valid() before using this value
 */
uint16_t pulse_sensor_get_bpm(void);

/**
 * @brief Validate current BPM reading reliability
 *
 * Checks if the current BPM reading meets quality criteria:
 * - Sufficient beat history for averaging
 * - BPM within physiological range (40-180 BPM)
 * - Signal quality above minimum threshold
 *
 * @return true if BPM reading is reliable and valid
 * @return false if BPM reading should be discarded
 */
bool pulse_sensor_bpm_valid(void);

//==============================================================================
// SIGNAL QUALITY AND DIAGNOSTICS
//==============================================================================

/**
 * @brief Assess pulse signal quality
 *
 * Evaluates signal quality based on amplitude consistency, beat regularity,
 * and signal-to-noise characteristics. Higher scores indicate more reliable
 * pulse detection.
 *
 * @return Signal quality score (0-100):
 *         - 80-100: Excellent signal, reliable BPM
 *         - 60-79:  Good signal, mostly reliable
 *         - 40-59:  Fair signal, use with caution
 *         - 0-39:   Poor signal, unreliable data
 */
uint8_t get_signal_quality(void);

/**
 * @brief Get current signal amplitude
 *
 * Returns the calculated amplitude (peak-to-peak) of the pulse signal
 * based on recent min/max tracking. Useful for sensor placement feedback
 * and signal strength monitoring.
 *
 * @return Current signal amplitude (ADC units)
 */
uint16_t get_signal_amplitude(void);

/**
 * @brief Get minimum signal value in current window
 *
 * Returns the lowest ADC value recorded in the current signal tracking
 * window. Resets periodically based on SIGNAL_UPDATE_INTERVAL_MS.
 *
 * @return Minimum ADC value in window
 */
uint16_t get_signal_min(void);

/**
 * @brief Get maximum signal value in current window
 *
 * Returns the highest ADC value recorded in the current signal tracking
 * window. Resets periodically based on SIGNAL_UPDATE_INTERVAL_MS.
 *
 * @return Maximum ADC value in current tracking window
 */
uint16_t get_signal_max(void);

#endif // PULSE_SENSOR_H
