/*
 * app_bluenrg.c
 *
 *  Created on: 6 Tem 2022
 *      Author: ibrhm
 */

#include "bluenrg1_hci_le.h"
#include "bluenrg1_aci.h"
#include "bluenrg1_gap.h"
#include "bluenrg1_types.h"
#include "hci.h"
#include "bluenrg_conf.h"
#include "app_bluenrg.h"
#include <stdio.h>
#include "service.h"
#include <string.h>
#include "stm32f411xe.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#define BADDR_SIZE 6
uint8_t SERVER_BADDR[]={0X10,0X02,0X03,0X04,0X05,0X06};
void bluenrg_init(void){
	const char *name="Semih35";
	tBleStatus ret;
	uint8_t bdaddr[BADDR_SIZE];
	BLUENRG_memcpy(bdaddr,SERVER_BADDR,sizeof(SERVER_BADDR));
	uint16_t service_handle,dev_name_char_handle,appearence_char_handle;




	/* Initialize HCI*/
	hci_init(APP_UserEvRx, NULL);
	/* Reset HCI*/
	hci_reset();
	HAL_Delay(100);
	 /*Configure DEVİCE ADRESS*/
	ret=aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, bdaddr);
	if(ret!=BLE_STATUS_SUCCESS){
		printf("Failed to configure Device Adress\r\n");
	}
	 /*Inıtialize GATT Server*/
	ret=aci_gatt_init();
	if(ret!=BLE_STATUS_SUCCESS){
		printf("aci_gati_init FAILED\r\n");
	}

	 /*Inıtialize GAP Service*/
	 aci_gap_init(GAP_PERIPHERAL_ROLE, 0, 0X07, &service_handle, &dev_name_char_handle, &appearence_char_handle);

	 /*Update Device  name Characteristic*/
	ret=aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, strlen(name), (uint8_t*)name);
	if(ret!=BLE_STATUS_SUCCESS){
		printf("aci_gatt_update char_value: FAILED\r\n");
	}
	ret=add_simple_services();
	if(ret!=BLE_STATUS_SUCCESS){
		printf("ADDİNG SİMPLE_SERVİESS PROCESS FAILED\r\n");
	}




}
char  *data="GLLOG402,12,1,1,1,300,400";

GPIO_PinState btn_state;
void bluenrg_process(void){

	tBleStatus ret;
	uint8_t local_name[]={AD_TYPE_COMPLETE_LOCAL_NAME,'B','L','E','-','G','-','U','P'};
	/*
	 * Sett device name general discovaerable mode
	 */
	/*aci_gap_set_discoverable(Advertising_Type, Advertising_Interval_Min, Advertising_Interval_Max, Own_Address_Type,
			Advertising_Filter_Policy, Local_Name_Length, Local_Name, Service_Uuid_length, Service_Uuid_List,
			Slave_Conn_Interval_Min, Slave_Conn_Interval_Max)*/
	ret=aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE, sizeof(local_name), local_name, 0, NULL, 0, 0);
	if(ret!=BLE_STATUS_SUCCESS){
		printf("aci_gap_set discoverable  failed\r\n");
	}
	//Process user event
	hci_user_evt_proc();
	btn_state=HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13);
	if(btn_state==GPIO_PIN_RESET){
		while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)==GPIO_PIN_RESET);
		send_data((uint8_t *)data, strlen(data));

}
}












