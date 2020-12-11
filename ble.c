#include "ble.h"

static ble_nus_t                        m_nus;                                      /**< Structure to identify the Nordic UART Service. */

static uint16_t                         m_name_cnt = 0;
static bool                             DATA_FLAG = false;

static uint16_t                         m_data_length = 0;
static uint16_t                         m_data_cnt = 0;

static bool                             ADC_FLAG = false;

static uint16_t                         global_cnt = 0;
static uint8_t                          m_data[1024];

static void ble_uart_data_send()
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
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
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

/**@brief Function for the application's SoftDevice event handler.
 *
 * @param[in] p_ble_evt SoftDevice event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            printf("Connected\n");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GAP_EVT_DISCONNECTED:
            printf("Disconnected\n");
            m_name_cnt = 0;
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break; // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GAP_EVT_SEC_PARAMS_REQUEST

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_SYS_ATTR_MISSING

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTC_EVT_TIMEOUT

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_TIMEOUT

        case BLE_EVT_USER_MEM_REQUEST:
            printf("BLE_EVT_USER_MEM_REQUEST\n");
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_EVT_USER_MEM_REQUEST
        case BLE_EVT_TX_COMPLETE:
            //printf("Event\n");
            if (DATA_FLAG)
                ble_uart_data_send();
            //printf("Cnt: %d\n", m_data_cnt);
            break;
        case BLE_GATTS_EVT_HVC:
            //printf("BLE_GATTS_EVT_HVC\n");
            // if (DATA_FLAG && m_data_cnt < m_data_length)
            //     ble_uart_data_send();
            // break;
        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            printf("Interval: %d\n", p_ble_evt->evt.gap_evt.params.conn_param_update.conn_params.max_conn_interval);
            break;
        // case BLE_GATTS_EVT_WRITE:
        //     printf("Write\n");
        //     break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

#if (NRF_SD_BLE_API_VERSION == 3)
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                       NRF_BLE_MAX_MTU_SIZE);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a SoftDevice event to all modules with a SoftDevice
 *        event handler.
 *
 * @details This function is called from the SoftDevice event interrupt handler after a
 *          SoftDevice event has been received.
 *
 * @param[in] p_ble_evt  SoftDevice event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);

}

/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize SoftDevice.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
#if (NRF_SD_BLE_API_VERSION == 3)
    ble_enable_params.gatt_enable_params.att_mtu = NRF_BLE_MAX_MTU_SIZE;
#endif
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing services that will be used by the application.
 */
void services_init(void)
{
    uint32_t       err_code;
    ble_nus_init_t nus_init;

    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}