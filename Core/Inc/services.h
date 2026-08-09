/**
 * @file services.h
 * @brief BLE GATT Services for STM32 Pulse Sensor Application
 *
 * Defines BLE services and characteristics for transmitting pulse sensor data
 * including BPM readings and temperature measurements via BlueNRG stack.
 *
 * Device: STM32 with BlueNRG BLE module
 * Application: Real-time pulse monitoring over BLE
 */

#ifndef INC_SERVICES_H_
#define INC_SERVICES_H_

/* ========================================================================== */
/*                              INCLUDES                                      */
/* ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include "bluenrg1_types.h"
#include "bluenrg1_gap.h"
#include "bluenrg1_gatt_aci.h"
#include "pulse_sensor.h"

/* ========================================================================== */
/*                           FUNCTION PROTOTYPES                             */
/* ========================================================================== */

/**
 * @brief Initialize and add all GATT services to BLE stack
 *
 * Sets up the complete GATT database including:
 * - Pulse sensor service with BPM characteristic
 * - Temperature service (if applicable)
 * - Device Information Service
 *
 * @return BLE_STATUS_SUCCESS on success, error code otherwise
 * @note Call this after BlueNRG initialization but before advertising
 */
tBleStatus add_services(void);

/**
 * @brief Add standard Device Information Service (0x180A)
 *
 * Adds DIS with standard characteristics like:
 * - Manufacturer Name
 * - Model Number
 * - System ID
 *
 * @return BLE_STATUS_SUCCESS on success, error code otherwise
 */
tBleStatus add_device_info_service(void);

/**
 * @brief Update BPM characteristic with new pulse data
 *
 * Sends BPM data to connected BLE clients. Should be called when:
 * - New valid BPM reading is available (40-180 range)
 * - Signal quality is sufficient (>= 50)
 * - BPM value has changed from last transmission
 *
 * @param data   Pointer to BPM value (typically uint16_t*)
 * @param length Size of data in bytes (typically 2 for uint16_t)
 *
 * @note This function handles GATT notifications automatically
 */
void update_bpm_data(uint16_t *data, uint16_t length);

/**
 * @brief Update temperature characteristic with sensor data
 *
 * Transmits temperature readings to connected BLE clients.
 *
 * @param data   Pointer to temperature value
 * @param length Size of data in bytes
 *
 * @note Temperature data format should be consistent with characteristic definition
 */
void update_temp_data(uint16_t *data, uint16_t length);

/**
 * @brief Process incoming BLE events and user data
 *
 * Event handler for BLE stack callbacks including:
 * - Connection/disconnection events
 * - Characteristic read/write requests
 * - Notification confirmations
 *
 * @param pckt Pointer to received HCI packet
 *
 * @note Called from main loop via bluenrg_process()
 */
void APP_UserEvtRx(void *pckt);

#endif /* INC_SERVICES_H_ */
