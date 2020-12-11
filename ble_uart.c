#include "ble_uart.h"

uint16_t                         m_name_cnt = 0;
bool                             DATA_FLAG = false;
uint16_t                         m_data_length = 0;
uint16_t                         m_data_cnt = 0;
bool                             ADC_FLAG = false;
uint16_t                         global_cnt = 0;
uint8_t                          m_data[1024];
ble_nus_t                        m_nus;                                      /**< Structure to identify the Nordic UART Service. */

void ble_uart_data_send()
{
    uint32_t err_code;
    uint16_t data[10];
    uint8_t byte_data[20];
    int i;
    //uint8_t tmp = 0;
    bool inc = false;

    while (m_data_cnt < m_data_length)
    //while (true)
    {
        for (i = 0; i < 10; i++)
        {
            //printf("Global cnt: %d\n", global_cnt);
            data[i] = global_cnt + i;
        }
        
        for (i = 0; i < 20; i += 2)
        {
            byte_data[i] = data[i/2] & 0xff;
            byte_data[i+1] = (data[i/2] >> 8) & 0xff;
            if (m_data_cnt+i < m_data_length)
                m_data[m_data_cnt+i] = byte_data[i];
            if (m_data_cnt+i+1 < m_data_length)
                m_data[m_data_cnt+i+1] = byte_data[i+1];
            if (m_data_cnt + i == m_data_length - 4)
            {
                //printf("Crc count: %d %d %d\n", m_data_cnt, global_cnt, m_data[1020]);
                byte_data[i+1] = 0;
                byte_data[i] = crc8(m_data, m_data_length-4);
            }
            if (m_data_cnt + i == m_data_length - 2)
            {
                inc = true;
                byte_data[i+1] = 0;
                byte_data[i] = m_name_cnt;
            }
            // if (circular_buf_get(cbuf, &tmp))
            // {
            //     printf("Added data\n");
            //     byte_data[i] = tmp;
            // }
            // else
            // {
            //     printf("Empty buffer\n");
            //     break;
            // }
        }
        if ((m_data_length - m_data_cnt) < 20)
        {
            //printf("1\n");
            err_code = ble_nus_string_send(&m_nus, byte_data, m_data_length - m_data_cnt);
        }
        else
        {
            err_code = ble_nus_string_send(&m_nus, byte_data, 20);
            //printf("1: %ld\n", err_code);
        }
        if (err_code == BLE_ERROR_NO_TX_PACKETS ||
        err_code == NRF_ERROR_INVALID_STATE || 
        err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
        {
            //printf("break\n");
            break;
        }
        APP_ERROR_CHECK(err_code);
        if (inc)
        {
            inc = false;
            m_name_cnt++;
        }
        m_data_cnt += 20;
        global_cnt += 10;
        //printf("m_data_cnt: %d, length: %d\n", m_data_cnt, m_data_length);
        if (m_data_length <= m_data_cnt && m_data_length % 20 == 0)
        {
            printf("Send zero package\n");
            // ble_gatts_hvx_params_t hvx_params;
            uint8_t buffer[20];
            uint16_t len=1;
            buffer[0] = 5;
            err_code = ble_nus_string_send(&m_nus, buffer, len);

            // hvx_params.p_data = &buffer[0];
            // hvx_params.p_len = &len;
            // hvx_params.offset = 0;
            // hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
            // hvx_params.handle = m_nus.rx_handles.value_handle;

            // sd_ble_gatts_hvx(m_nus.conn_handle, &hvx_params);
            printf("Sended zero package\n");
        }
        //printf("Cnt: %d\n", m_data_cnt);
    }
}

/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
    uint32_t err_code;
    //char *end = (char *)(p_data + length - 1);
    char str[] = "name", str2[] = "adc on ", str1[] = "get ";
    char substr[length];
    char name[14];
    strncpy(substr, (char *)p_data, length);
    //uint32_t data_length = atoi((char *)p_data), cnt;

    printf("Str: %s\n", (char *)(p_data));
    if (length >= 4)
    {
        if (strncmp(substr, str, 4) == 0)
        {
            DATA_FLAG = false;
            sprintf(name, "Nordic UART: %d", m_name_cnt);
            m_name_cnt++;
            err_code = ble_nus_string_send(&m_nus, (uint8_t *)name, 14);
            APP_ERROR_CHECK(err_code);
        }
        else if (strncmp(substr, str1, 3) == 0)
        {
            // for (int i = 0; i < length; i++)
            // {
            //     printf("%x ", p_data[i]);
            // }
            //m_data_length = atoi((char *)(p_data + 4));
            
            
            //strtol((char *)p_data, &(p_data+4), 10);
            m_data_length = strtoull((char *)(substr + 4), NULL, 10);
            printf("Data length: %d\n", m_data_length);
            //printf("Str: %s\n", substr);
            //printf("length: %d\n", length);
            m_data_cnt = 0;
            global_cnt = 0;
            if (m_data_length > (1 * 1024))
            {
                DATA_FLAG = false;
                printf("Error\n");
                err_code = ble_nus_string_send(&m_nus, (uint8_t *)"Data size is too big", 20);
                APP_ERROR_CHECK(err_code);
            } else {
                DATA_FLAG = true;
                //app_uart_put('b');
                ble_uart_data_send();
            }
        }
        else if (strncmp(substr, str2, 7) == 0)
        {
            m_data_length = strtoull((char *)(substr + 7), NULL, 10);
        }
    }
    else
    {
        err_code = ble_nus_string_send(&m_nus, (uint8_t *)"Wrong command", 13);
        APP_ERROR_CHECK(err_code);
        DATA_FLAG = false;
        ADC_FLAG = true;
    }
}
/**@snippet [Handling the data received over BLE] */