#ifndef __PTZ_H_
#define __PTZ_H_

#include "stdint.h"

#define PTZ_DEV		"/dev/ttysWK0" 

#pragma pack(1)
typedef struct
{
	int16_t roll;   
    int16_t pitch; /*0.01deg*/
    int16_t yaw;
    int16_t error_yaw;
    int16_t  zoom_rate;
} ptz_payload_rx_data_status_t;
typedef union
{
    ptz_payload_rx_data_status_t        payload_rx_data_status;
    uint8_t raw[1];
}ptz_rx_buf_t;
#pragma pack()

int ptz_parse_char(const uint8_t b, ptz_payload_rx_data_status_t* msg);
uint16_t ptz_pack_pitch_to_buffer(int16_t pitch, uint8_t *buffer);
uint16_t ptz_pack_acc_to_buffer(float ax, float ay, float az, uint8_t *buffer);

#endif
