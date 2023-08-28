#ifndef _UORB_TOPIC_H_
#define _UORB_TOPIC_H_

#include "topic.h"
#include "stdbool.h"

typedef enum __HGE {
    HG_ERR = -1,
    HG_OK = 0,  
} HG_E;

typedef struct __mount_control_t
{
    int32_t input_a;       /*<  Pitch (centi-degrees) or lat (degE7), depending on mount mode.*/
    int32_t input_b;       /*<  Roll (centi-degrees) or lon (degE7) depending on mount mode.*/
    int32_t input_c;       /*<  Yaw (centi-degrees) or alt (cm) depending on mount mode.*/
    uint8_t target_system; /*<  System ID.*/
    uint8_t component;     /*<  Component ID.*/
    int8_t zoom_speed;     /*<  zoom speed (-7 ~ +7).*/
    uint8_t save_position; /*<  If "1" it will save current trimmed position on EEPROM (just valid for NEUTRAL and LANDING).*/
} mount_control_t;

typedef struct __mount_status_t
{
    int16_t roll;          /*< [cdeg] Pitch.*/
    int16_t pitch;         /*< [cdeg] Roll.*/
    int16_t yaw;           /*< [cdeg] Yaw.*/
    int16_t error_yaw;     /*< [cdeg] error_yaw.*/
    int16_t zoom_rate;     /*< [0.1] zoom_rate.*/
    uint8_t target_system; /*<  System ID.*/
    uint8_t component;     /*<  Component ID.*/
} mount_status_t;

typedef struct __drone_pos_acc_t
{
    float acc[3]; /*<  body acc*/
    float local_pos[3]; /*<  local acc*/
} drone_pos_acc_t;

typedef struct __drone_attitude_t {
    uint64_t timestamp; /*< Timestamp (milliseconds since system boot)*/
    float roll; /*< Roll angle (rad, -pi..+pi)*/
    float pitch; /*< Pitch angle (rad, -pi..+pi)*/
    float yaw; /*< Yaw angle (rad, -pi..+pi)*/
    float rollspeed; /*< Roll angular speed (rad/s)*/
    float pitchspeed; /*< Pitch angular speed (rad/s)*/
    float yawspeed; /*< Yaw angular speed (rad/s)*/
    float ground_distance;
    uint16_t deltatime;
}__attribute__((packed)) drone_attitude_t;

typedef struct __drone_ctrl_t
{
    uint64_t utc;
    uint8_t cmd;
    int32_t param[4];
} drone_ctrl_t;

typedef struct __media_ctrl_t
{
    uint8_t cmd;
    int32_t param[4];
} media_ctrl_t;

typedef struct __drone_ack_t {
    uint32_t token;
    uint16_t id;
    uint8_t cmd;
    uint8_t result;
} drone_ack_t;

typedef struct __drone_id_t {
    uint16_t id;
} drone_id_t;
 
// device_server publisher
TOPIC_DECLARE(mount_status);
TOPIC_DECLARE(drone_attitude);
TOPIC_DECLARE(drone_pos_acc);
TOPIC_DECLARE(drone_stats);
TOPIC_DECLARE(drone_ack);
TOPIC_DECLARE(drone_id);

// msg_server publisher
TOPIC_DECLARE(mount_control);
TOPIC_DECLARE(drone_ctrl);
TOPIC_DECLARE(swarm_track);
TOPIC_DECLARE(host_parmas);
TOPIC_DECLARE(media_ctrl);

// algo_server publisher
TOPIC_DECLARE(track_send);
TOPIC_DECLARE(drone_ctrl_algo);

#endif