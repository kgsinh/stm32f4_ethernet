#include "services.h"


uint8_t HEALTH_SERVICE_UUID[16] = {0x00, 0x00, 0x18, 0x0F, 0x00, 0x10, 0x00, 0x80,
						  	  	  0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB, 0x00, 0x00};
uint8_t BPM_CHAR_UUID[16] = {0x00, 0x00, 0x19, 0x0F, 0x00, 0x10, 0x00, 0x80,
						 	 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB, 0x00, 0x00};

uint8_t WEATHER_SERVICE_UUID[16] = {0x00, 0x00, 0x1A, 0x20, 0x00, 0x10, 0x00, 0x80,
						 	  	   0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB, 0x00, 0x00};
uint8_t TEMP_CHAR_UUID[16] = {0x00, 0x00, 0x2A, 0x20, 0x00, 0x10, 0x00, 0x80,
						 	  0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB, 0x00, 0x00};

uint16_t health_service_handle, health_bpm_char_handle;
uint16_t weather_service_handle, weather_temp_char_handle;

uint8_t connected_device = 0;
uint8_t set_connectable = 1;
uint16_t connection_handle = 0;
uint8_t notification_enabled = 0;

tBleStatus add_services(void)
{
	tBleStatus ret;
	Service_UUID_t health_service_uuid, weather_service_uuid;
	Char_UUID_t    health_bpm_char_uuid, weather_temp_char_uuid;

	BLUENRG_memcpy(health_service_uuid.Service_UUID_128, HEALTH_SERVICE_UUID, 16);
	BLUENRG_memcpy(health_bpm_char_uuid.Char_UUID_128, BPM_CHAR_UUID, 16);
	BLUENRG_memcpy(weather_service_uuid.Service_UUID_128, WEATHER_SERVICE_UUID, 16);
	BLUENRG_memcpy(weather_temp_char_uuid.Char_UUID_128, TEMP_CHAR_UUID, 16);

	/* Add health service */
	ret = aci_gatt_add_service(UUID_TYPE_128,
	                           &health_service_uuid,
	                           PRIMARY_SERVICE,
	                           7,
	                           &health_service_handle);

	if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in aci_gatt_add_service: 0x%02X\r\n", ret);
		return ret;
	}

	/* Add weather service */
	ret = aci_gatt_add_service(UUID_TYPE_128,
	                           &weather_service_uuid,
	                           PRIMARY_SERVICE,
	                           7,
	                           &weather_service_handle);

	if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in aci_gatt_add_service: 0x%02X\r\n", ret);
		return ret;
	}

	/* Add health characteristic - bpm */
	ret = aci_gatt_add_char(health_service_handle,
	                         UUID_TYPE_128,
	                         &health_bpm_char_uuid,
	                         2,
	                         CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	                         ATTR_PERMISSION_NONE,
	                         GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
	                         0,
	                         0,
	                         &health_bpm_char_handle);

	if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in aci_gatt_add_char: 0x%02X\r\n", ret);
		return ret;
	}

	/* Add weather characteristic - temperature */
	ret = aci_gatt_add_char(weather_service_handle,
	                         UUID_TYPE_128,
	                         &weather_temp_char_uuid,
	                         4,
	                         CHAR_PROP_READ,
	                         ATTR_PERMISSION_NONE,
	                         GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
	                         0,
	                         0,
	                         &weather_temp_char_handle);

	if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in aci_gatt_add_char: 0x%02X\r\n", ret);
		return ret;
	}

	return BLE_STATUS_SUCCESS;
}

tBleStatus add_device_info_service(void)
{
    tBleStatus ret;
    uint16_t service_handle;
    Service_UUID_t service_uuid;
    service_uuid.Service_UUID_16 = 0x180A; // Device Information Service

    ret = aci_gatt_add_service(UUID_TYPE_16, &service_uuid, PRIMARY_SERVICE, 4, &service_handle);

    if (ret != BLE_STATUS_SUCCESS) {
		printf("Error in aci_gatt_add_service (DIS): 0x%02X\r\n", ret);
		return ret;
	}

    return ret;
}

void update_bpm_data(uint16_t *data, uint16_t length)
{
	tBleStatus ret;

	/* Update health characteristic value - bpm */
	ret = aci_gatt_update_char_value(health_service_handle,
									 health_bpm_char_handle,
									 0,
									 length,
									 (uint8_t *)data);

	if (ret != BLE_STATUS_SUCCESS) {
		printf("BPM DATA: Error in aci_gatt_update_char_value: 0x%02X\r\n", ret);
	}

}

void update_temp_data(uint16_t *data, uint16_t length)
{
	tBleStatus ret;

	/* Update weather characteristic value - temperature */
	ret = aci_gatt_update_char_value(weather_service_handle,
									 weather_temp_char_handle,
									 0,
									 length,
									 (uint8_t *)data);

	if (ret != BLE_STATUS_SUCCESS) {
		printf("TEMP DATA: Error in aci_gatt_update_char_value: 0x%02X\r\n", ret);
	}
}

void Read_Request_CB(uint16_t handle)
{
	if(connection_handle != 0) {
		aci_gatt_allow_read(connection_handle);
	}
}

void aci_gatt_read_permit_req_event(uint16_t Connection_Handle,
								 uint16_t Attr_Handle,
								 uint16_t Offset)
{
	Read_Request_CB(Attr_Handle);
}

void hci_le_connection_complete_event(uint8_t Status,
									   uint16_t Connection_Handle,
									   uint8_t Role,
									   uint8_t Peer_Address_Type,
									   uint8_t Peer_Address[6],
									   uint16_t Conn_Interval,
									   uint16_t Conn_Latency,
									   uint16_t Supervision_Timeout,
									   uint8_t Master_Clock_Accuracy)
{
	connected_device = 1;
	connection_handle = Connection_Handle;
}

void hci_le_disconnection_complete_event(uint8_t Status,
										 uint16_t Connection_Handle,
										 uint8_t Reason)
{
	connected_device = 0;
	notification_enabled = 0;
	set_connectable = 1;
	connection_handle = 0;
}

void APP_UserEvtRx(void *pckt)
{
	uint32_t i;
	hci_spi_pckt *hci_pckt = pckt;

	if (hci_pckt->type != HCI_EVENT_PKT){
		return;
	}

	hci_event_pckt *event_pckt = (hci_event_pckt *)hci_pckt->data;

	switch (event_pckt->evt) {
		case EVT_LE_META_EVENT: {

			/* Get data from the event packet */
			hci_event_pckt *event_pckt = (hci_event_pckt *)hci_pckt->data;

			/* Process meta data event */
			evt_le_meta_event *meta_event = (evt_le_meta_event *)event_pckt->data;

			for (i = 0; i< (sizeof(hci_le_meta_events_table)/sizeof(hci_le_meta_events_table_type)); i++) {

				if (meta_event->subevent == hci_le_meta_events_table[i].evt_code) {
					hci_le_meta_events_table[i].process(meta_event->data);
					break;
				}
			}
		}
		break;

		case EVT_VENDOR: {
			/* Get data from the event packet */
			evt_blue_aci *blue_evt = (evt_blue_aci *)event_pckt->data;

			/* Process each vendor specific event */
			for (i = 0; i< (sizeof(hci_vendor_specific_events_table)/sizeof(hci_vendor_specific_events_table[0])); i++) {

				if (blue_evt->ecode == hci_vendor_specific_events_table[i].evt_code) {
					hci_vendor_specific_events_table[i].process(blue_evt->data);
					break;
				}
			}

		}
		break;

		default:
			for(i = 0; i< (sizeof(hci_events_table)/sizeof(hci_events_table[0])); i++) {

				if (event_pckt->evt == hci_events_table[i].evt_code) {
					hci_events_table[i].process(event_pckt->data);
					break;
				}
			}
			break;

}
}




















