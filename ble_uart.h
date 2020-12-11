#ifndef BLE_UART_H
#define BLE_UART_H

#include <stdlib.h>

#include "def.h"
#include "utils.h"

void ble_uart_data_send(void);
void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length);

#endif