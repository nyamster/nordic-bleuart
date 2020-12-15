#include "uart.h"

bool UART_FLAG = false;
static int index = 0;

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' i.e '\r\n' (hex 0x0D) or if the string has reached a length of
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
static void uart_event_handle(app_uart_evt_t * p_event)
{
    //static uint8_t data_array[1024];
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            err_code = app_uart_get(&m_data[index]);
            APP_ERROR_CHECK(err_code);
            //printf("Data ready: %c\n", data_array[index]);
            //err_code = app_uart_put(data_array[index]);
            
            // CRITICAL_REGION_ENTER();
            // circular_buf_put(cbuf, data_array[index]);
            // CRITICAL_REGION_EXIT();
            index++;
            //printf("Index: %d\n", index);

            if (index >= UART_PACKET_LEN)
            {
                printf("Send over ble via uart\n");
                //bool ble_buffer_available;
                err_code = app_uart_put(0);
                APP_ERROR_CHECK(err_code);
                UART_FLAG = true;
                m_data_cnt = 0;
                ble_uart_ready_data_send(index);
                //ble_buffer_available = ble_attempt_to_send(data_array, index);
                // err_code = ble_nus_string_send(&m_nus, data_array, index);
                // if (err_code != NRF_ERROR_INVALID_STATE)
                // {
                //     APP_ERROR_CHECK(err_code);
                // }
                //if (ble_buffer_available) index = 0;
                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            printf("APP_UART_COMMUNICATION_ERROR\n");
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_TX_EMPTY:
            //app_uart_put('b');
            break;

        case APP_UART_DATA:
            //printf("Data ready1\n");
            break;

        case APP_UART_FIFO_ERROR:
            printf("APP_UART_FIFO_ERROR\n");
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;
        default:
            break;
    }
}
/**@snippet [Handling the data received over UART] */


/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
void uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        0x7,
        0x8,
        UART_PIN_DISCONNECTED,
        UART_PIN_DISCONNECTED,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}
/**@snippet [UART Initialization] */