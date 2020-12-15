#include "utils.h"

uint8_t crc8(uint8_t *buffer, uint16_t len)
{
    uint8_t crc = 0xFF;
    uint8_t i;
 
    while (len--)
    {
        crc ^= *buffer++;
 
        for (i = 0; i < 8; i++)
            crc = crc & 0x80 ? (crc << 1) ^ 0x1D : crc << 1;
    }
 
    crc ^= 0xFF;
    return crc;
}