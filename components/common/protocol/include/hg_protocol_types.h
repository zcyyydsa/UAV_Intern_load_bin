#ifndef __HG_PROTOCOL_TYPES_H_
#define __HG_PROTOCOL_TYPES_H_

#include <stdint.h>

#define HGPROTOCOL_STX 0x88
#define HGPROTOCOL_CORE_HEADER_LEN 5 ///< Length of core header (of the comm. layer): message length (1 byte) + message sequence (1 byte) + message system id (1 byte) + message component id (1 byte) + message type id (1 byte)
#define HGPROTOCOL_NUM_HEADER_BYTES (HGPROTOCOL_CORE_HEADER_LEN + 1) ///< Length of all header bytes, including core and checksum
#define HGPROTOCOL_NUM_CHECKSUM_BYTES 2
#define HGPROTOCOL_NUM_NON_PAYLOAD_BYTES (HGPROTOCOL_NUM_HEADER_BYTES + HGPROTOCOL_NUM_CHECKSUM_BYTES)

#define HGPROTOCOL_COMM_NUM_BUFFERS 4
#define HGPROTOCOL_MAX_PAYLOAD_LEN 255

typedef enum {
    HGPROTOCOL_PARSE_STATE_UNINIT=0,
    HGPROTOCOL_PARSE_STATE_IDLE,
    HGPROTOCOL_PARSE_STATE_GOT_STX,
    HGPROTOCOL_PARSE_STATE_GOT_SEQ,
    HGPROTOCOL_PARSE_STATE_GOT_LENGTH,
    HGPROTOCOL_PARSE_STATE_GOT_SYSID,
    HGPROTOCOL_PARSE_STATE_GOT_COMPID,
    HGPROTOCOL_PARSE_STATE_GOT_MSGID,
    HGPROTOCOL_PARSE_STATE_GOT_PAYLOAD,
    HGPROTOCOL_PARSE_STATE_GOT_CRC1,
    HGPROTOCOL_PARSE_STATE_GOT_BAD_CRC1
} hgprotocol_parse_state_t; ///< The state machine for the comm parser

typedef enum {
    HGPROTOCOL_FRAMING_INCOMPLETE=0,
    HGPROTOCOL_FRAMING_OK=1,
    HGPROTOCOL_FRAMING_BAD_CRC=2
} mavlink_framing_t;

typedef struct __protocol_status {
    uint8_t msg_received;               ///< Number of received messages
    uint8_t buffer_overrun;             ///< Number of buffer overruns
    uint8_t parse_error;                ///< Number of parse errors
    hgprotocol_parse_state_t parse_state;  ///< Parsing state machine
    uint8_t packet_idx;                 ///< Index in current packet
    uint8_t current_rx_seq;             ///< Sequence number of last packet received
    uint8_t current_tx_seq;             ///< Sequence number of last packet sent
    uint16_t packet_rx_success_count;   ///< Received packets
    uint16_t packet_rx_drop_count;      ///< Number of packet drops
} hgprotocol_status_t;

typedef struct __hgprotocol_message {
	uint16_t checksum; ///< sent at end of packet
	uint8_t magic;   ///< protocol magic marker
	uint8_t len;     ///< Length of payload
	uint8_t seq;     ///< Sequence of packet
	uint8_t sysid;   ///< ID of message sender system/aircraft
	uint8_t compid;  ///< ID of the message sender component
	uint8_t msgid;   ///< ID of message in payload
	uint64_t payload64[(HGPROTOCOL_MAX_PAYLOAD_LEN+HGPROTOCOL_NUM_CHECKSUM_BYTES+7)/8];
} hgprotocol_message_t;


typedef enum MAV_PLANE_CMD
{
    MAV_CMD_DRONE_BASE = 0,
    MAV_CMD_SET_POS_END,    // 1
    MAV_CMD_SET_POS_GLOBAL, // 2
    MAV_CMD_SET_YAW,        // 3
    MAV_CMD_SETID,          // 4
    MAV_CMD_TAKEOFF,        // 5
    MAV_CMD_LAND,           // 6
    MAV_CMD_LOCK,           // 7
    MAV_CMD_UNLOCK,         // 8
    MAV_CMD_UP,             // 9
    MAV_CMD_DOWN,           // 10
    MAV_CMD_LEFT,           // 11
    MAV_CMD_RIGHT,          // 12
    MAV_CMD_FORWARD,        // 13
    MAV_CMD_BACK,           // 14
    MAV_CMD_STOP,           // 15
    MAV_CMD_CW,             // 16
    MAV_CMD_CCW,            // 17
    MAV_CMD_GO,             // 18
    MAV_CMD_TIMESYNC,

    MAV_CMD_MEDIA_BASE = 50,
    MAV_CMD_TAKEPHOTO,      // 51
    MAV_CMD_RECORD,         // 52
    MAV_CMD_FILELIST,

    MAV_CMD_PERIPHERALS_BASE = 100,
    MAV_CMD_PTZ,            // 101
    MAV_CMD_SERVO,          // 102
    MAV_CMD_WIFI_MODE,      // 103

    MAV_CMD_MSG_BASE = 150,
    MAV_CMD_NET_DISCONNECT, // 151

    MAV_CMD_ALGO_BASE = 200,
} MAV_PLANE_CMD;

typedef enum ERRNO_ACK
{
    CMD_OK = 0,          // 指令执行成功
    CMD_ERR,             // 指令执行失败
    CMD_SD_NOTEXIST,     // sd异常
    CMD_SD_NOTENOUGH,    // sd容量不足
    CMD_REPEAT,          // 指令重复
    CMD_FILE_NOEXIST     // 文件不存在
} ERRNO_ACK_E;


#define HGPROTOCOL_MSG_ID_LCOMMAND 0
#define HGPROTOCOL_MSG_ID_LACK 1
#define HGPROTOCOL_MSG_ID_LACK_EXTEND 2
#define HGPROTOCOL_MSG_ID_LFILEINFO 3
#define HGPROTOCOL_MSG_ID_DRONE_STATS 4
#define HGPROTOCOL_MSG_ID_SWARM_TRACK 5
#define HGPROTOCOL_MSG_ID_HOST_PARMAS 6
#define HGPROTOCOL_MSG_ID_TRACK_SEND 7

typedef struct __hgprotocol_lcommand_t {
    uint64_t utc;
    uint16_t start_id;
    uint16_t end_id;
    uint16_t cmd;
    uint8_t ack;
    int32_t param[8];
} hgprotocol_lcommand_t;

typedef struct __hgprotocol_lack_extend_t {
    uint32_t token;
    uint16_t id;
    uint8_t cmd;
    uint8_t result;
    uint8_t type;
    int8_t extend[64];
} hgprotocol_lack_extend_t;

typedef struct __hgprotocol_lack_t {
    uint32_t token;
    uint16_t id;
    uint8_t cmd;
    uint8_t result;
    uint8_t type;
    int32_t param[8];
} hgprotocol_lack_t;

typedef struct __hgprotocol_lfileinfo_t {
    uint32_t file_size;
    uint16_t id;
    uint8_t file_type;
    uint8_t version[3];
    uint8_t md5[16];
    int8_t file_name[64];
} hgprotocol_lfileinfo_t;

// drone -> app
typedef struct __hgprotocol_drone_stats_t
{
    uint64_t utc;                /*<  Unix Timestamp in milliseconds*/
    int32_t lat;                 /*<  Latitude * 1E7*/
    int32_t lon;                 /*<  Longitude * 1E7*/
    int32_t alt;                 /*<  Altitude * 1E3*/
    float x;                     /*<  Position X*/
    float y;                     /*<  Position Y*/
    float z;                     /*<  Position Z*/
    uint32_t aux_token;          /*<  Aux dance step token*/
    uint32_t time_token;         /*<  Aux dance step token*/
    int16_t yaw;                 /*<  Yaw * 10*/
    uint16_t id;                 /*<  Drone ID*/
    int16_t temperature_imu;     /*<  Temperature Imu * 100*/
    uint16_t version;            /*<  Version of firmware*/
    uint16_t sensor_status;      /*<  sensor status*/
    uint8_t stats;               /*<  System stats*/
    uint8_t temperature_battery; /*<  Temperature Battery*/
    uint8_t battery_cycles;      /*<  Battery Cycles*/
    uint8_t battery;             /*<  Battery volumn*/
    uint8_t rtk_stats;           /*<  Rtk status and satellite used*/
    uint8_t md5[16];             /*<  MD5 of dance step file*/
    uint8_t rk_version[8];       /*<  Version of rock chip*/
    uint8_t type;                /*<  Drone Battery Status*/
    uint8_t formation_status;    /*<  Drone Formation Status*/
    uint8_t land_reason;         /*<  Drone land reason*/
    uint8_t imu_status[2];       /*<  imu status*/
    uint8_t mag_status[2];       /*<  mag status*/
    uint8_t no_used[8];          /*<  no used*/
} __attribute__((packed)) hgprotocol_drone_stats_t;

// app -> linux
typedef struct __hgprotocol_swarm_track_t
{
    uint64_t time_stamp;//时间戳
    uint8_t fun_id;//0无功能运行，1集群围捕跟踪
    uint8_t swarm_state;//集群状态机
    uint8_t state[3];//每台飞机状态机
    uint8_t state_status[3];//每台飞机状态机state所处的状态
    //0 disable飞机不参与功能或强制下线，1 offline飞机不在线（飞机和上位机失联），2 onlne飞机在线，3 飞机在线并且跟踪正常， 4 飞机在线但跟踪目标丢失
    uint8_t mav_status[3];
    float pos[3*5];// 台飞机世界坐标位置，数据排列为mav0(x,y,z,yaw,tracked vehicle id),mav1(x,y,z,yaw,tracked vehicle id)，mav2(x,y,z,yaw,tracked vehicle id)
    float vehicle0[4];//小车绝对坐标:x,y,z,yaw;yaw will not use
    float vehicle1[4];
    uint8_t mav_target[6];//track_target_vehicle_id[3]+value[3],value is 0,1,2,...,7(代表相对于目标车不同的坐标偏移)
    float track_dist[6];//mav and vehicle horizontal distance[3] + track height[3]
    uint8_t is_candidate[3];//飞机是否是补位机
} hgprotocol_swarm_track_t;

// app -> linux
typedef struct __hgprotocol_point2f_t{
	float x;
	float y;
} point2f_t;
typedef struct __hgprotocol_host_parmas_t{
    float track_height;
    float track_angle;//it is cloud angle,current will not be used
    point2f_t fence[5];//meter
    int mav_mask[3];
} hgprotocol_host_parmas_t;

// linux -> app
typedef struct __hgprotocol_track_send_t{
    uint64_t time_stamp;//时间戳
    uint8_t fun_id;//0无功能运行，1集群围捕跟踪 (功能由上位机启动)
    uint8_t state;//状态机，不同功能状态机定义不同
    uint8_t state_status;//state所处的状态（例：state处于跟踪，state_status可以用来表征跟踪状态）
    uint8_t tracked_vehicle_id;//被跟踪的小车id
    float tx;//目标位置（例如：被跟踪的目标）
    float ty;//目标位置
    float tz;//目标位置
} hgprotocol_track_send_t;

#endif


