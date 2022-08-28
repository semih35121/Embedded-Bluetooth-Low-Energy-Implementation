/*
 * service.h
 *
 *  Created on: 12 Tem 2022
 *      Author: ibrhm
 */

#ifndef INC_SERVICE_H_
#define INC_SERVICE_H_
#include "bluenrg1_types.h"
#include <stdio.h>
tBleStatus add_simple_services();
void APP_UserEvRx(void *pdata);
void send_data(uint8_t *send_data,uint8_t no_bytes);

#endif /* INC_SERVICE_H_ */
