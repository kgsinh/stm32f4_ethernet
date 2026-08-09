#include "pulse_sensor.h"


// Configuration constants
static uint16_t signal_min = 4095;
static uint16_t signal_max = 0;
static uint16_t signal_threshold = 2048;
static uint32_t last_threshold_update = 0;
static uint32_t last_beat_time = 0;
static uint32_t last_detection_time = 0;
static uint32_t beat_intervals[BEAT_HISTORY_SIZE];
static uint8_t beat_index = 0;
static uint8_t valid_beats = 0;
static uint16_t current_bpm = 0;
static bool above_threshold = false;
static bool new_beat = false;
static uint16_t signal_history[SIGNAL_HISTORY_SIZE];
static uint8_t signal_history_index = 0;

// Private function declarations
static void calculate_bpm(void);
static void update_signal_bounds(uint16_t adc_value);

void pulse_sensor_init(void) {
    signal_min = 4095;
    signal_max = 0;
    signal_threshold = 2048;
    last_threshold_update = HAL_GetTick();
    last_beat_time = 0;
    last_detection_time = 0;
    beat_index = 0;
    valid_beats = 0;
    current_bpm = 0;
    above_threshold = false;
    new_beat = false;
    signal_history_index = 0;

    // Clear arrays
    memset(beat_intervals, 0, sizeof(beat_intervals));
    memset(signal_history, 0, sizeof(signal_history));
}

void pulse_sensor_process(uint16_t adc_value) {
    uint32_t now = HAL_GetTick();

    // Update signal history for quality assessment
    signal_history[signal_history_index] = adc_value;
    signal_history_index = (signal_history_index + 1) % SIGNAL_HISTORY_SIZE;

    // Update signal bounds and calculate dynamic threshold
    update_signal_bounds(adc_value);

    // Debounce check - prevent rapid false detections
    if (now - last_detection_time < DEBOUNCE_TIME_MS) {
        return;
    }

    // Rising edge detection with dynamic threshold
    if (!above_threshold && adc_value > signal_threshold) {
        above_threshold = true;

        // Record beat timing if we have a previous beat
        if (last_beat_time > 0) {
            uint32_t interval = now - last_beat_time;

            // Validate interval is within reasonable BPM range
            if (interval >= MIN_BEAT_INTERVAL_MS && interval <= MAX_BEAT_INTERVAL_MS) {
                // Store the interval
                beat_intervals[beat_index] = interval;
                beat_index = (beat_index + 1) % BEAT_HISTORY_SIZE;

                if (valid_beats < BEAT_HISTORY_SIZE) {
                    valid_beats++;
                }

                // Calculate BPM when we have enough data
                if (valid_beats >= 3) {
                    calculate_bpm();
                    new_beat = true;
                    printf("Beat! Interval: %lu ms, BPM: %d, Quality: %d\n\r",
                           interval, current_bpm, get_signal_quality());
                }
            }
        }

        last_beat_time = now;
        last_detection_time = now;
    }

    // Falling edge detection with hysteresis
    if (above_threshold && adc_value < (signal_threshold - 50)) {
        above_threshold = false;
    }
}

bool pulse_sensor_beat_detected(void) {
    if (new_beat) {
        new_beat = false;
        return true;
    }
    return false;
}

uint16_t pulse_sensor_get_bpm(void) {
    return current_bpm;
}

bool pulse_sensor_bpm_valid(void) {
    return (current_bpm >= 40 && current_bpm <= 180 &&
            valid_beats >= 3 && get_signal_quality() >= 50);
}

uint8_t get_signal_quality(void) {
    uint16_t amplitude = (signal_max > signal_min) ? (signal_max - signal_min) : 0;

    // Base quality assessment on signal amplitude
    if (amplitude < 20) {
        return 10;  // Very poor signal
    } else if (amplitude < 50) {
        return 30;  // Poor signal
    } else if (amplitude < 100) {
        return 60;  // Moderate signal
    } else {
        return 85;  // Good signal
    }
}

// Additional diagnostic functions
uint16_t get_signal_amplitude(void) {
    return (signal_max > signal_min) ? (signal_max - signal_min) : 0;
}

uint16_t get_signal_min(void) {
    return signal_min;
}

uint16_t get_signal_max(void) {
    return signal_max;
}

uint16_t get_signal_threshold(void) {
    return signal_threshold;
}

static void calculate_bpm(void) {
    if (valid_beats < 3) return;

    // Use the most recent intervals for averaging
    uint8_t samples = (valid_beats < BEAT_HISTORY_SIZE) ? valid_beats : BEAT_HISTORY_SIZE;
    uint32_t total = 0;

    for (uint8_t i = 0; i < samples; i++) {
        total += beat_intervals[i];
    }

    uint32_t avg_interval = total / samples;
    current_bpm = (60000 / avg_interval);
}

static void update_signal_bounds(uint16_t adc_value) {
    uint32_t now = HAL_GetTick();

    // Reset min/max periodically to adapt to lighting changes
    if (now - last_threshold_update > SIGNAL_UPDATE_INTERVAL_MS) {
        signal_min = 4095;
        signal_max = 0;
        last_threshold_update = now;
    }

    // Update bounds
    if (adc_value < signal_min) {
        signal_min = adc_value;
    }
    if (adc_value > signal_max) {
        signal_max = adc_value;
    }

    // Calculate dynamic threshold
    if (adc_value > signal_min) {
        signal_threshold = signal_min + (uint16_t)((signal_max - signal_min) * PULSE_THRESHOLD_RATIO);
    }
}
