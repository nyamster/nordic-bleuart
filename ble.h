#ifndef BLE_H
#define BLE_H

#include "def.h"
#include "ble_hci.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "ble_nus.h"
#include "bsp_btn_ble.h"

#include "ble_uart.h"

void ble_stack_init(void);
void services_init(void);

#endif