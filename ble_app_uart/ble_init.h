#ifndef ADVERTISING_H
#define ADVERTISING_H

#include "def.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"

#include "ble_uart.h"

void advertising_init(void);
void conn_params_init(void);
void gap_params_init(void);
void services_init(void);

#endif