/**
 * @file app_bluenrg.c
 * @brief BlueNRG BLE Stack Application Implementation
 * @author STM32 BLE Pulse Sensor Project
 * @version 1.0
 *
 * This file implements the BlueNRG BLE stack initialization and processing
 * for the STM32 pulse sensor application. It handles device configuration,
 * GATT/GAP setup, and advertising management.
 */

#include "app_bluenrg.h"

uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x02};

void bluenrg_init(void)
{
	tBleStatus ret;
	uint8_t bdaddr[BDADDR_SIZE];
	const char *gap_name = "STM32BLE";
	uint8_t name_len = (uint8_t)strlen(gap_name);

	uint16_t service_handle, dev_name_char_handle, appearance_char_handle;

	BLUENRG_memcpy(bdaddr, SERVER_BDADDR, sizeof(SERVER_BDADDR));

	/* Initialize the BlueNRG HCI */
	hci_init(APP_UserEvtRx, 0);

	/* Reset BlueNRG hardware */
	hci_reset();

	/* Wait until the BlueNRG is ready */
	HAL_Delay(100);

	/* Configure device address */
	ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
	                                 CONFIG_DATA_PUBADDR_LEN,
	                                 bdaddr);
	if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in aci_hal_write_config_data: 0x%02X\r\n", ret);
	}

	/* Initialize the GATT */
	ret = aci_gatt_init();

	if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in aci_gatt_init: 0x%02X\r\n", ret);
	}

	/* Initialize the GAP */
	ret = aci_gap_init(GAP_PERIPHERAL_ROLE,
	                   0,
	                   GAP_DEV_NAME_LEN,
	                   &service_handle,
	                   &dev_name_char_handle,
	                   &appearance_char_handle);

	if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in aci_gap_init: 0x%02X\r\n", ret);
	}

	/* Update device name characteristic value */
	ret = aci_gatt_update_char_value(service_handle,
	                              dev_name_char_handle,
	                              0,
	                              name_len,
	                              (uint8_t *)gap_name);

	if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in aci_gatt_update_char_value: 0x%02X\r\n", ret);
	}


	/* Add custom services */
	ret = add_services();

	if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in add_simple_service: 0x%02X\r\n", ret);
	}
}

void bluenrg_process(void)
{
    static uint8_t advertising_started = 0;
    tBleStatus ret;

    if (!advertising_started)
    {
        advertising_started = 1;

        /* Prepare advertising data with complete local name */
        uint8_t local_name[] = {
            sizeof("STM32BLE") + 1,
            AD_TYPE_COMPLETE_LOCAL_NAME,
            'S','T','M','3','2','B','L','E'
        };

        /* Configure device as discoverable and connectable */
        ret = aci_gap_set_discoverable(
            ADV_IND,                 // Advertising type (connectable undirected)
            ADV_INT_MIN,			 // Minimum advertising interval
            ADV_INT_MAX,			 // Maximum advertising interval
            PUBLIC_ADDR,			 // Using public bluetooth address
            NO_WHITE_LIST_USE,	     // No white list for filtering
            sizeof(local_name),	   	 // Advertising data length
            local_name,				 // Advertising data payload
            0,						 // Scan response data length
            NULL,  					 // Scan response data
            0,	 					 // No slave connection interval min
            0	 					 // No slave connection interval max
        );

        if (ret != BLE_STATUS_SUCCESS) {
			printf("Error in aci_gap_set_discoverable: 0x%02X\r\n", ret);
		}
    }

    /* Process HCI events */
    hci_user_evt_proc();
}



