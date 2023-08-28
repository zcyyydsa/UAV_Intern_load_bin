#include "hg_tof.h"

static uint8_t tof_data_crc(char *data, int length)
{
    int res = 0;
    uint8_t crc = 0;

    for (int i = 0; i < length - 1; ++i)
    {
        res += data[i];
    }
    crc = res & 0xff;
    return crc;
}

int tof_decode_state = 0;
char tof_buf[9] = {0};
int tof_parse_char(uint8_t b, tof_data_t *data)
{
    int ret = -1;
    if (tof_decode_state == 0) {
        tof_buf[tof_decode_state] = b;
        if (b == 0x59) {
            tof_decode_state++;
        } else {
            tof_decode_state = 0;
        }
    } else if (tof_decode_state == 1) {
        tof_buf[tof_decode_state] = b;
        if (b == 0x59) {
            tof_decode_state++;
        } else {
            tof_decode_state = 0;
        }        
    } else {
        switch (tof_decode_state) {
            case 8:
                tof_buf[tof_decode_state] = b;
                if (b == tof_data_crc(tof_buf, 9)) {
                    data->height = tof_buf[2] + tof_buf[3]*256;
                    data->strength = tof_buf[4] + tof_buf[4]*256;
                    ret = 1;
                }
                tof_decode_state = 0;
                break;
            default:
                if (tof_decode_state < 8) {
                    tof_buf[tof_decode_state] = b;
                    tof_decode_state++;
                } else {
                    tof_decode_state = 0;
                }
                break;
        }
    }

    return ret;
}
