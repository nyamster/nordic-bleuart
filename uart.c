#include "uart.h"

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
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            err_code = app_uart_get(&data_array[index]);
            APP_ERROR_CHECK(err_code);
            //printf("Data ready: %c\n", data_array[index]);
            //err_code = app_uart_put(data_array[index]);
            
            // CRITICAL_REGION_ENTER();
            // circular_buf_put(cbuf, data_array[index]);
            // CRITICAL_REGION_EXIT();
            index++;

            // if (index >= (BLE_NUS_MAX_DATA_LEN))
            // {
            //     err_code = ble_nus_string_send(&m_nus, data_array, index);
            //     if (err_code != NRF_ERROR_INVALID_STATE)
            //     {
            //         APP_ERROR_CHECK(err_code);
            //     }

            //     index = 0;
            // }
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