/*
 * service.c
 *
 *  Created on: 12 Tem 2022
 *      Author: ibrhm
 */
#include "bluenrg1_gap.h"
#include "bluenrg1_gatt_aci.h"
#include "service.h"
#include <stdio.h>
#include "stdlib.h"
uint8_t SERVICE_UUID[16]={0X6A,0X9A,0X0C,0X20,0X00,0X08,0XFC,0XFB,0XB4,0XA3,0X12,0X16,0X9E,0XBC,0X7C,0X11};
uint8_t characteristic_tx_UUID[16]={0X6A,0X9A,0X0C,0X20,0X00,0X08,0XFC,0XFB,0XB4,0XA3,0X12,0X15,0X9E,0XBC,0X7C,0X11};
uint8_t characteristic_rx_UUID[16]={0X6A,0X9A,0X0C,0X20,0X00,0X08,0XFC,0XFB,0XB4,0XA3,0X12,0X14,0X9E,0XBC,0X7C,0X11};

uint16_t chat_service_handle,mychar_tx_handle,mychar_rx_handle;
uint8_t connected=FALSE;
uint8_t set_connectable=1;
uint16_t connection_handle=0;
uint8_t notification_enabled=FALSE;
tBleStatus add_simple_services(){
	tBleStatus ret;
	Service_UUID_t service_uuid;
	Char_UUID_t char_tx_uuid,char_rx_uuid;
	BLUENRG_memcpy(service_uuid.Service_UUID_128,SERVICE_UUID,16);
	ret=aci_gatt_add_service(UUID_TYPE_128,&service_uuid ,PRIMARY_SERVICE, 7, &chat_service_handle);
	BLUENRG_memcpy(char_tx_uuid.Char_UUID_128,characteristic_tx_UUID,16);
	BLUENRG_memcpy(char_rx_uuid.Char_UUID_128,characteristic_rx_UUID,16);

	//Add characteristic
	/*aci_gatt_add_char(Service_Handle, Char_UUID_Type, Char_UUID, Char_Value_Length,
	 Char_Properties, Security_Permissions, GATT_Evt_Mask, Enc_Key_Size, Is_Variable, Char_Handle)*/
	//Char_tx Characteristic

	ret=aci_gatt_add_char(chat_service_handle, UUID_TYPE_128,&char_tx_uuid, 100,CHAR_PROP_NOTIFY ,ATTR_PERMISSION_NONE ,0, 0, 1, &mychar_tx_handle);
	//Char rx Characterisitic
	ret=aci_gatt_add_char(chat_service_handle, UUID_TYPE_128,&char_rx_uuid, 40,CHAR_PROP_WRITE|CHAR_PROP_WRITE_WITHOUT_RESP ,ATTR_PERMISSION_NONE ,GATT_NOTIFY_ATTRIBUTE_WRITE, 0, 1, &mychar_rx_handle);

	return ret;
}
uint8_t rcv_data[40];
void process_rx_event(uint8_t *received_string,uint8_t no_bytes){
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	for(int i=0;i<no_bytes;i++){
		rcv_data[i]=received_string[i];
	}
	char *incoming_data=(char *)rcv_data;

	printf("gelen data=%s\n",incoming_data);


}

void send_data(uint8_t *send_data,uint8_t no_bytes){
	tBleStatus ret;
	//Update characteristic value
	ret=aci_gatt_update_char_value(chat_service_handle, mychar_tx_handle, 0, no_bytes,send_data);
	if(ret!=BLE_STATUS_SUCCESS){
			printf("Failed to update_char_value\r\n");
		}

}
/*void GAP_ConnectionComplete_CB(uint8_t addr[6],uint16_t handle){
	connected=TRUE;
	connection_handle=handle;
	printf("Connection Complete...\r\n");

}
void GAP_DisconnectionComplete_CB(void){
	printf("Disconnection Complete....\r\n");
}*/

void attribute_modified_callback(uint16_t handle,uint8_t data_length,uint8_t *att_data){
	if(handle==mychar_rx_handle+1){
		process_rx_event(att_data, data_length);
	}
	/*
	 * For enabling notification  the configuration data is {0x00,0x01}
	 * In this case ,the handle of the attribute for Tx Characteristic is offset by two from the characteristic handle
	 */
	else if(handle==mychar_tx_handle+2){
		if(att_data[0]==0x01){
			notification_enabled=TRUE;
		}
	}

}

void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
									uint16_t Attr_Handle,
									uint16_t Offset,
									uint16_t Attr_Data_Length,
									uint8_t Attr_Data[])
{
	attribute_modified_callback(Attr_Handle,Attr_Data_Length,Attr_Data);
}
 void aci_gatt_notification_event(uint16_t Connection_Handle,
                                 uint16_t Attribute_Handle,
                                 uint8_t Attribute_Value_Length,
                                 uint8_t Attribute_Value[])
 {
	 if(Attribute_Handle==mychar_tx_handle+2){
		 process_rx_event(Attribute_Value, Attribute_Value_Length);
	 }



                   }
//New Connection is implemented
void hci_le_connection_complete_event(uint8_t Status,
									  uint16_t Connection_Handle,
									  uint8_t Role,
									  uint8_t Peer_Adress_Type,
									  uint8_t Peer_Adress[6],
									  uint16_t Conn_Interval,
									  uint16_t Conn_Latency,
									  uint16_t Supervision_Timeout,
									  uint8_t Master_Clock_Accuracy)
{
	connected=TRUE;
	connection_handle=Connection_Handle;

}
void hci_disconnection_complete_event(uint8_t Status,
							         uint16_t Connection_Handle,
									 uint8_t Reason)
{
	connected=FALSE;
	set_connectable=1;
	connection_handle=0;
	printf("Disconnected\r\n");


}

void APP_UserEvRx(void *pdata){
	uint32_t i;
	hci_spi_pckt *hci_pckt=(hci_spi_pckt*)pdata;
	//Process event packet
	if(hci_pckt->type==HCI_EVENT_PKT){
		//Get data from packet
		hci_event_pckt *event_pckt=(hci_event_pckt*)hci_pckt->data;
		//Process meta data event
		if(event_pckt->evt==EVT_LE_META_EVENT){
			//gET META DATA
			evt_le_meta_event *evt=(void*)event_pckt->data;
			//Process each meta data event
			for(i=0;i<sizeof(hci_le_meta_events_table)/sizeof(hci_le_meta_events_table_type);i++){
					if(evt->subevent==hci_le_meta_events_table[i].evt_code){
						hci_le_meta_events_table[i].process((void*)evt->data);

					}

			}


		}
		//Process Vendor eVENT
		else if(event_pckt->evt==EVT_VENDOR){
			//Get vendor data packet from trhe packet
			evt_blue_aci * blue_evt=(void*)event_pckt->data;
			for(i=0;i<sizeof(hci_vendor_specific_events_table)/sizeof(hci_vendor_specific_events_table_type);i++){
				if(blue_evt->ecode==hci_vendor_specific_events_table[i].evt_code){
					hci_vendor_specific_events_table[i].process((void*)blue_evt->data);
				}
			}

		}
		else{
			for(i=0;i<sizeof(hci_events_table)/sizeof(hci_events_table_type);i++){
				if(event_pckt->evt==hci_events_table[i].evt_code){
					hci_events_table[i].process((void *)event_pckt->data);
				}

			}


		}


	}



}






