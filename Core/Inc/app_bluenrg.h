/**
 * @file app_bluenrg.h
 * @brief STM BlueNRG(X-NUCLEO-BNRG2A1) BLE Stack Interface for STM32 Pulse Sensor Application
 *
 * This header provides function declarations and configuration constants for
 * integrating the BlueNRG Bluetooth Low Energy stack with the STM32 pulse
 * sensor firmware. Handles BLE initialization, event processing, and device
 * advertising for pulse data transmission.
 *
 * @author Kunal
 * @version 1.0
 * @date 2026-01-25
 */

#ifndef INC_APP_BLUENRG_H_
#define INC_APP_BLUENRG_H_

/* ========================================================================== */
/*                              INCLUDES                                      */
/* ========================================================================== */
#include "services.h"           // Custom BLE service definitions
#include "bluenrg_conf.h"       // BlueNRG stack configuration
#include "bluenrg1_types.h"     // BlueNRG data type definitions
#include "bluenrg1_gap.h"       // Generic Access Profile interface
#include "bluenrg1_aci.h"       // Application Controller Interface
#include "bluenrg1_hci_le.h"    // Host Controller Interface for Low Energy
#include "stdio.h"              // Standard I/O functions
#include "stdlib.h"             // Standard library functions
#include "hci.h"                // Host Controller Interface definitions

/* ========================================================================== */
/*                        CONFIGURATION CONSTANTS                            */
/* ========================================================================== */

/**
 * @defgroup BLE_Config BLE Configuration Parameters
 * @brief Essential constants for BLE stack configuration
 * @{
 */

/** @brief Bluetooth Device Address size in bytes */
#define BDADDR_SIZE 6

/** @brief Maximum GAP device name length in characters */
#define GAP_DEV_NAME_LEN 8

/** @brief Minimum advertising interval (20ms in 0.625ms units) */
#define ADV_INT_MIN 0x20

/** @brief Maximum advertising interval (40ms in 0.625ms units) */
#define ADV_INT_MAX 0x40

/** @} */ // End of BLE_Config group

/* ========================================================================== */
/*                         FUNCTION DECLARATIONS                             */
/* ========================================================================== */

/**
 * @defgroup BLE_Core_Functions BLE Stack Core Functions
 * @brief Primary functions for BLE stack management
 * @{
 */

/**
 * @brief Initialize the BlueNRG BLE stack and configure device parameters
 *
 * Performs complete BLE stack initialization including:
 * - Hardware and communication interface setup
 * - GAP (Generic Access Profile) configuration
 * - GATT (Generic Attribute Profile) service setup
 * - Device name configuration ('STM32BLE')
 * - Advertising parameter configuration
 * - BDADDR setup (12:34:00:E1:80:02)
 *
 * @note Must be called once during system startup before any BLE operations
 * @note Ensure SPI communication with BlueNRG module is properly configured
 *
 * @pre System clock and GPIO initialization completed
 * @post BLE stack ready for advertising and connections
 *
 * @return None
 */
void bluenrg_init(void);

/**
 * @brief Process pending BLE stack events and maintain connection state
 *
 * Handles all incoming BLE events and manages the application state:
 * - Connection/disconnection events
 * - GATT characteristic read/write requests
 * - Advertising state management
 * - Data transmission acknowledgments
 * - Error handling and recovery
 *
 * @note Call continuously in main application loop for responsive BLE operation
 * @note Non-blocking function - returns immediately if no events pending
 *
 * @pre bluenrg_init() must be called first
 * @post All pending BLE events processed
 *
 * @return None
 */
void bluenrg_process(void);

/** @} */ // End of BLE_Core_Functions group



#endif /* INC_APP_BLUENRG_H_ */
