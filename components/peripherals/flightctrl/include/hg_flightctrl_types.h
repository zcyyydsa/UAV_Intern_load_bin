#ifndef __HG_FLIGHTCTRL_TPYES_H_
#define __HG_FLIGHTCTRL_TPYES_H_

#include <stdint.h>

#define DRONE_ID_ACC 233
//#define DRONE_ID_ACK 209
#define DRONE_ID_ACK 165
#define DRONE_ID_STATS 207
#define DRONE_ID_ATTITUDE 30
#define DRONE_ID_DEV_ERR 255

typedef struct __flightctrl_hdr_t {
    void* (*get)(void* param, int bytes);
    void (*release)(void* param, void* packet);
    void (*onrecv)(void* param, const void* packet, uint16_t bytes, int msgid);
} flightctrl_hdr_t;

typedef struct __flightctrl_cmd_t {
    int32_t pos_data[3];
    float yaw;
    uint8_t cmd; 
    uint8_t ack;
    uint8_t reserve[8]; 
} flightctrl_cmd_t;

typedef struct __flightctrl_ack_t {
    uint32_t token; /*<  Command token*/
    uint16_t id; /*<  Drone ID*/
    uint8_t cmd; /*<  Command 1-Takeoff 2-Land 3-Prepare 4-Arm 5-Disarm 6-TimeSync 7-CalMag*/
    uint8_t result; /*<  Result*/
    uint8_t type; /*<  Drone is formation or aux 0-formation 1-aux*/
} flightctrl_ack_t;


#endif