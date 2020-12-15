#ifndef BLE_UART_H
#define BLE_UART_H

#include <stdlib.h>

#include "def.h"
#include "utils.h"

#include "uart.h"

void ble_uart_data_send(void);
void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length);
bool ble_attempt_to_send(uint8_t * data, uint8_t length);
void ble_uart_ready_data_send(int length);

#endif