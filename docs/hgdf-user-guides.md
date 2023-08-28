# hgdf 编程指南

## 1 快速入门

本章节旨在指导用户搭建 D04 负载仓硬件平台开发的软件环境，通过一个简单的示例展示如何使用 hgdf（HighGreat Development Framework）配置菜单，并编译、下载固件到 挂载硬件等步骤。

### 1.1 概述

D04 负载仓硬件平台 SoC 芯片支持以下功能：

- 2.4 Ghz Wi-Fi
- 高性能 Arm 64位 双核A72 + 四核A53处理器
- 多种外设（飞控、接收机、TOF、舵机等）

高巨为用户提供完整的软、硬件资源，进行 D04 负载仓硬件设备的开发。其中，高巨的软件开发环境 hgdf 旨在协助用户快速开发视觉图像应用，可满足用户对教学、模块学习、算法验证等多方面的要求。
### 1.2 准备工作

硬件：

- D04 负载仓硬件平台
- 可选外设（D04飞机、云台、舵机、接收机、TOF、SD卡、WiFi模块）
- 电脑 （Windows、Linux）

软件：

如需在 D04挂载硬件平台上使用 hgdf，请配置以下环境：

- 设置交叉编译工具链
- 获取 hgdf 软件开发框架。该框架基本包含挂载硬件平台二次开发所用到的API（软件库和源代码）和运行工具链的脚本。


### 1.3 编译第一个工程

进入交叉编译虚拟机，打开桌面文件夹onboard_sdk
```
$cd onboard_sdk
$cd hgdf/examples/test-tof
$mkdir build
$cd build
$cmake ..
$make -j2

// 此时 build 目录会生成编译完的可执行文件 test-tof
// 部署 test-tof 到机载运行查看效果

// 1、虚拟机连接type-c， type-c连机载
// 2、将 test-tof 拷贝到机载上
$adb push test-tof /tmp
// 3、进入机载终端，执行 test-tof
$adb shell
// 此时会进去机载终端
#cd /tmp
#./test-tof

// example目录下的其余demo操作同上
```



## 2 通用组件
### 2.1 log
提供日志功能

#### 2.1.1 API参考

##### 2.1.1.1 log_init
【描述】

飞控创建初始化

【语法】

`int log_init()`

【参数】
无

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：log.h

库文件：liblog.a

【注意】

【举例】

<SdkRoot>/example/test-log/main/main.cpp

##### 2.1.1.1 log_deinit
【描述】

飞控创建初始化

【语法】

`void log_deinit()`

【参数】
无

【返回值】
无

【需求】

头文件：log.h

库文件：liblog.a

【注意】

【举例】
<SdkRoot>/example/test-log/main/main.cpp

#### 2.1.2 数据类型
无

### 2.2 protocol
提供上位机交互的协议接口

#### 2.2.1 API参考

##### 2.2.1.1 hgprotocol_parse_char

【描述】

协议数据流解析

【语法】

`uint8_t hgprotocol_parse_char(uint8_t chan, uint8_t c, hgprotocol_message_t* r_message, hgprotocol_status_t* r_status)`

【参数】

| 参数名称   | 描述          | 输入/输出 |
| --------  | -----------   | --------- |
| chan      | 数据通道       | 输入      |
| c         | 接收到的字符   | 输入      |
| r_message | 获取的数据帧   | 输出      |
| r_status  | 获取的飞机状态 | 输出      |

【返回值】

| 返回值  | 描述       |
| ------ | ---------  |
| 0      | INCOMPLETE |
| 1      | OK         |
| 2      | BAD CRC    |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.2 hgprotocol_msg_to_send_buffer

【描述】

协议完整数据帧序列化

【语法】

`uint16_t hgprotocol_msg_to_send_buffer(uint8_t *buffer, const hgprotocol_message_t *msg)`

【参数】

| 参数名称 |      描述      | 输入/输出 |
| --------| -------------- | --------- |
| buffer  | 序列化后的数据流 | 输出      |
| msg     | 数据帧消息封装   | 输入      |

【返回值】

| 返回值 |   描述   |
| ------ | ------- |
| < 0    | 失败    |
| > 0    | 数据长度 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.3 hgprotocol_lcommand_encode

【描述】
 飞机控制指令数据帧封装

【语法】

`uint16_t hgprotocol_lcommand_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lcommand_t* lmsg)`

【参数】

| 参数名称      | 描述             | 输入/输出 |
| ------------ | ---------------- | --------- |
| system_id    | 系统id编号        | 输入      |
| component_id | 组件id编号        | 输入      |
| msg          | 数据帧消息封装     | 输出      |
| lmsg         | 飞机控制指令数据帧 | 输入      |

【返回值】

| 返回值 | 描述     |
| ------ | ------- |
| < 0    | 失败    |
| > 0    | 数据长度 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.4 hgprotocol_lcommand_decode

【描述】
 解析来自上位机的指令数据帧

【语法】

`void hgprotocol_lcommand_decode(const hgprotocol_message_t* msg, hgprotocol_lcommand_t* lmsg);`

【参数】

| 参数名称  | 描述            | 输入/输出 |
| -------- | --------------- | --------- |
| msg      | 上位机的数据帧   | 输入      |
| lmsg     | 获取的指令数据帧 | 输出      |
【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 无      | 无 |


【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.5 hgprotocol_lack_encode

【描述】
 封装返回给上位机的响应数据

【语法】

`uint16_t hgprotocol_lack_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lack_t* lmsg)`

【参数】

| 参数名称      | 描述             | 输入/输出 |
| --------     | ---------------  | ---------|
| system_id    | 系统id编号        | 输入      |
| component_id | 组件id编号        | 输入      |
| msg          | 封装协议消息数据帧 | 输出     |
| lmsg         | 响应ack数据帧     | 输入      |

【返回值】

| 返回值 | 描述     |
| ------ | ------- |
| < 0    | 失败    |
| > 0    | 数据长度 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.6 hgprotocol_lack_decode

【描述】
 解析上位机响应数据帧

【语法】

`void hgprotocol_lack_decode(const hgprotocol_message_t* msg, hgprotocol_lack_t* lmsg)`

【参数】

| 参数名称 | 描述            | 输入/输出 |
| --------| --------------- | -------- |
| msg     | 协议消息数据帧   | 输入      |
| lmsg    | ack响应数据帧    | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 无      | 无 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】
<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.7 hgprotocol_lack_extend_encode

【描述】
 封装上位机扩展响应数据帧

【语法】

`uint16_t hgprotocol_lack_extend_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lack_extend_t* lmsg)`

【参数】

| 参数名称 | 描述                   | 输入/输出 |
| --------| ---------------------- | -------- |
| system_id    | 系统id编号        | 输入      |
| component_id | 组件id编号        | 输入      |
| msg          | 封装协议消息数据帧 | 输出     |
| lmsg         | 响应扩展ack数据帧  | 输入      |

【返回值】

| 返回值 | 描述     |
| ------ | ------- |
| < 0    | 失败    |
| > 0    | 数据长度 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.8 hgprotocol_lack_extend_decode

【描述】
 解析上位机扩展响应数据帧

【语法】

`void hgprotocol_lack_extend_decode(const hgprotocol_message_t* msg, hgprotocol_lack_extend_t* lmsg)`

【参数】

| 参数名称 |       描述          | 输入/输出 |
| --------| ------------------- | -------- |
| msg     | 协议消息数据帧       | 输入      |
| lmsg    | ack扩展响应数据帧    | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 无      | 无 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.9 hgprotocol_lfileinfo_encode

【描述】
 封装文件信息数据帧

【语法】

`uint16_t hgprotocol_lfileinfo_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lfileinfo_t* lmsg)`

【参数】

| 参数名称      | 描述             | 输入/输出 |
| -------------| ---------------- | -------- |
| system_id    | 系统id编号        | 输入      |
| component_id | 组件id编号        | 输入      |
| msg          | 封装协议消息数据帧 | 输出     |
| lmsg         | 文件信息数据帧     | 输入      |

【返回值】

| 返回值 | 描述     |
| ------ | ------- |
| < 0    | 失败    |
| > 0    | 数据长度 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp


##### 2.2.1.10 hgprotocol_lfileinfo_decode

【描述】
 解析上位机响应数据帧

【语法】

`void hgprotocol_lfileinfo_decode(const hgprotocol_message_t* msg, hgprotocol_lfileinfo_t* lmsg)`

【参数】

| 参数名称 | 描述            | 输入/输出 |
| --------| --------------- | -------- |
| msg     | 协议消息数据帧   | 输入      |
| lmsg    | 文件消息数据帧    | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 无      | 无 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.11 hgprotocol_drone_stats_encode

【描述】
 封装飞机心跳信息数据帧

【语法】

`uint16_t hgprotocol_drone_stats_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_drone_stats_t* lmsg)`

【参数】

| 参数名称      | 描述             | 输入/输出 |
| -------------| ---------------- | -------- |
| system_id    | 系统id编号        | 输入      |
| component_id | 组件id编号        | 输入      |
| msg          | 封装协议消息数据帧 | 输出      |
| lmsg         | 心跳信息数据帧     | 输入      |

【返回值】

| 返回值 | 描述     |
| ------ | ------- |
| < 0    | 失败    |
| > 0    | 数据长度 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.12 hgprotocol_drone_stats_decode

【描述】
 解析飞机心跳信息数据帧

【语法】

`void hgprotocol_drone_stats_decode(const hgprotocol_message_t* msg, hgprotocol_drone_stats_t* lmsg)`

【参数】

| 参数名称 | 描述            | 输入/输出 |
| --------| --------------- | -------- |
| msg     | 协议消息数据帧   | 输入      |
| lmsg    | 心跳信息数据帧    | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 无      | 无 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.13 hgprotocol_swarm_track_encode

【描述】
 封装围捕消息数据帧

【语法】

`uint16_t hgprotocol_swarm_track_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_swarm_track_t* lmsg)`

【参数】

| 参数名称      | 描述             | 输入/输出 |
| -------------| ---------------- | -------- |
| system_id    | 系统id编号        | 输入      |
| component_id | 组件id编号        | 输入      |
| msg          | 封装协议消息数据帧 | 输出      |
| lmsg         | 围捕消息数据帧     | 输入      |

【返回值】

| 返回值 | 描述     |
| ------ | ------- |
| < 0    | 失败    |
| > 0    | 数据长度 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.14 hgprotocol_swarm_track_decode

【描述】
 解析围捕消息数据帧

【语法】

`void hgprotocol_swarm_track_decode(const hgprotocol_message_t* msg, hgprotocol_swarm_track_t* lmsg)`

【参数】

| 参数名称 | 描述            | 输入/输出 |
| --------| --------------- | -------- |
| msg     | 协议消息数据帧   | 输入      |
| lmsg    | ack响应数据帧    | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 无      | 无 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.15 hgprotocol_host_parmas_encode

【描述】
 封装飞机参数数据帧

【语法】

`uint16_t hgprotocol_host_parmas_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_host_parmas_t* lmsg)`

【参数】

| 参数名称      | 描述             | 输入/输出 |
| -------------| ---------------- | -------- |
| system_id    | 系统id编号        | 输入      |
| component_id | 组件id编号        | 输入      |
| msg          | 封装协议消息数据帧 | 输出      |
| lmsg         | 飞机参数数据帧     | 输入      |

【返回值】

| 返回值 | 描述     |
| ------ | ------- |
| < 0    | 失败    |
| > 0    | 数据长度 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.16 hgprotocol_lack_decode

【描述】
 解析飞机参数数据帧

【语法】

`void hgprotocol_host_parmas_decode(const hgprotocol_message_t* msg, hgprotocol_host_parmas_t* lmsg)`

【参数】

| 参数名称 | 描述            | 输入/输出 |
| --------| --------------- | -------- |
| msg     | 协议消息数据帧   | 输入      |
| lmsg    | 飞机参数数据帧    | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 无      | 无 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/main.cpp

##### 2.2.1.17 hgprotocol_track_send_encode

【描述】
 封装追踪数据帧

【语法】

`uint16_t hgprotocol_track_send_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_track_send_t* lmsg)`

【参数】

| 参数名称      | 描述             | 输入/输出 |
| -------------| ---------------- | -------- |
| system_id    | 系统id编号        | 输入      |
| component_id | 组件id编号        | 输入      |
| msg          | 封装协议消息数据帧 | 输出      |
| lmsg         | 飞机追踪数据帧     | 输入      |

【返回值】

| 返回值 | 描述     |
| ------ | ------- |
| < 0    | 失败    |
| > 0    | 数据长度 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp

##### 2.2.1.18 hgprotocol_track_send_decode

【描述】
 解析追踪数据帧

【语法】

`void hgprotocol_track_send_decode(const hgprotocol_message_t* msg, hgprotocol_track_send_t* lmsg)`

【参数】

| 参数名称 | 描述            | 输入/输出 |
| --------| --------------- | -------- |
| msg     | 协议消息数据帧   | 输入      |
| lmsg    | 追踪数据帧       | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 无      | 无 |

【需求】

头文件：hg_protocol.h hg_protocol_types.h

库文件：libhgprotocol.a

【注意】

【举例】

<SdkRoot>/example/test-hgprotocol/main/src/main.cpp


#### 2.2.2 数据类型

##### 2.2.2.1 hgprotocol_message_t

【说明】

协议消息数据结构

【定义】

```
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

```

##### 2.2.2.2 hgprotocol_lcommand_t
【说明】

指令控制协议数据帧数据结构

【定义】

```
typedef struct __hgprotocol_lcommand_t {
    uint64_t utc;
    uint16_t start_id;
    uint16_t end_id;
    uint16_t cmd;
    uint8_t ack;
    int32_t param[8];
} hgprotocol_lcommand_t;

```

##### 2.2.2.3 hgprotocol_lack_extend_t
【说明】

扩展ack响应结构

【定义】


```
typedef struct __hgprotocol_lack_extend_t {
    uint32_t token;
    uint16_t id;
    uint8_t cmd;
    uint8_t result;
    uint8_t type;
    int8_t extend[64];
} hgprotocol_lack_extend_t;

```

##### 2.2.2.4 hgprotocol_lack_t
【说明】

ack响应结构

【定义】


```
typedef struct __hgprotocol_lack_t {
    uint32_t token;
    uint16_t id;
    uint8_t cmd;
    uint8_t result;
    uint8_t type;
    int32_t param[8];
} hgprotocol_lack_t;

```

##### 2.2.2.5 hgprotocol_lfileinfo_t
【说明】

舞步文件信息结构

【定义】


```
typedef struct __hgprotocol_lfileinfo_t {
    uint32_t file_size;
    uint16_t id;
    uint8_t file_type;
    uint8_t version[3];
    uint8_t md5[16];
    int8_t file_name[64];
} hgprotocol_lfileinfo_t;

```

##### 2.2.2.6 hgprotocol_drone_stats_t
【说明】

心跳信息结构

【定义】


```
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

```

##### 2.2.2.7 hgprotocol_swarm_track_t
【说明】



【定义】


```
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
    uint8_t mav_target[3];//value is 0,1,2,...,7(代表相对于目标车不同的坐标偏移)
    float track_dist[3];//mav and vehicle horizontal  distance
} hgprotocol_swarm_track_t;

```

##### 2.2.2.8 point2f_t
【说明】


【定义】


```
typedef struct __hgprotocol_point2f_t{
	float x;
	float y;
} point2f_t;

```

##### 2.2.2.9 hgprotocol_host_parmas_t
【说明】

【定义】


```
typedef struct __hgprotocol_host_parmas_t{
    float track_height;
    float track_angle;//it is cloud angle,current will not be used
    point2f_t fence[5];//meter
    int mav_mask[3];
} hgprotocol_host_parmas_t;

```

##### 2.2.2.10 hgprotocol_track_send_t
【说明】



【定义】


```
typedef struct __hgprotocol_track_send_t{
    uint64_t time_stamp;//时间戳
    uint8_t fun_id;//0无功能运行，1集群围捕跟踪 (功能由上位机启动)
    uint8_t state;//状态机，不同功能状态机定义不同
    uint8_t state_status;//state所处的状态（例：state处于跟踪，state_status可以用来表征跟踪状态）
    float tx;//目标位置（例如：被跟踪的目标）
    float ty;//目标位置
    float tz;//目标位置
} hgprotocol_track_send_t;

```

##### 2.2.2.11 hgprotocol_status_t
【说明】



【定义】


```
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

```

##### 2.2.2.12 hgprotocol_parse_state_t
【说明】



【定义】


```
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
} hgprotocol_parse_state_t;
```

##### 2.2.2.13 hgprotocol_lack_extend_t
【说明】



【定义】


```
typedef enum {
    HGPROTOCOL_FRAMING_INCOMPLETE=0,
    HGPROTOCOL_FRAMING_OK=1,
    HGPROTOCOL_FRAMING_BAD_CRC=2
} mavlink_framing_t;

```

##### 2.2.2.14 MAV_PLANE_CMD
【说明】

飞控指令枚举

【定义】


```
typedef enum MAV_PLANE_CMD {
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

```

##### 2.2.2.15 ERRNO_ACK_E
【说明】

错误 ack响应枚举

【定义】


```
typedef enum ERRNO_ACK {
    CMD_OK = 0,          // 指令执行成功
    CMD_ERR,             // 指令执行失败
    CMD_SD_NOTEXIST,     // sd异常
    CMD_SD_NOTENOUGH,    // sd容量不足
    CMD_REPEAT,          // 指令重复
    CMD_FILE_NOEXIST     // 文件不存在
} ERRNO_ACK_E;
```




## 3 算法组件
### 3.1 track
提供物体跟踪的接口。

实例类HgTrack，提供以下类方法

#### 3.1.1 API参考

##### 3.1.1.1 extinit

【描述】

初始化跟踪框。

【语法】

`void extinit(int x, int y, int w, int h, uint8_t* img, int imgW, int imgH);`

【参数】

| 参数名称 | 描述         | 输入/输出 |
| -------- | ------------ | --------- |
| x    | 跟踪框坐标x | 输入      |
| y    | 跟踪框坐标y | 输入      |
| w    | 跟踪框坐标宽度 | 输入      |
| h    | 跟踪框坐标高度 | 输入      |
| img    | 图像数据指针（nv12） | 输入      |
| imgW    | 图像宽度 | 输入      |
| imgH    | 图像高度 | 输入      |

【返回值】

无

【需求】

头文件：hg_track.h

库文件：libtrack.a

【注意】

【举例】


##### 3.1.1.1 extupdate

【描述】

更新跟踪目标，输出坐标位置

【语法】

`float extupdate(uint8_t* img, int imgW, int imgH, int& x, int&y, int &w, int &h);`

【参数】

| 参数名称 | 描述         | 输入/输出 |
| -------- | ------------ | --------- |
| img    | 图像数据（nv12） | 输入      |
| imgW    | 图像宽度 | 输入      |
| imgH    | 图像高度 | 输入      |
| x    | 输出跟踪框坐标x | 输出      |
| y    | 输出跟踪框坐标y | 输出      |
| w    | 输出跟踪框坐标w | 输出      |
| h    | 输出跟踪框坐标h | 输出      |

【返回值】

无

【需求】

头文件：hg_track.h

库文件：libtrack.a
【注意】

【举例】


#### 3.1.2 数据类型




### 3.2 detect_digital
提供图形数字检测的接口

实例类HgDDigtal，提供以下类方法

#### 3.2.1 API参考

##### 3.2.1.1 DetectDigital

【描述】

检测数字

【语法】

`void DetectDigital(Mat im,vector<Marker>& markers);`

【参数】

| 参数名称 | 描述         | 输入/输出 |
| -------- | ------------ | --------- |
| im   | 图像数据 | 输入      |
| markers    | 检测信息 | 输出      |


【返回值】

无

【需求】

头文件：hg_detect_digtal.h

库文件：libdetect_digtal.a

【注意】

【举例】


#### 3.2.2 数据类型



## 4 媒体组件

### 4.1 media
该模块分为VI、VPSS、VENC。VI主要涉及摄像头数据的获取，VPSS针对VI的图像进行缩放等处理，VENC可将图像编码成H264码流

#### 4.1.1 API参考

##### 4.1.1.1 MPI_VI_CreateChn

【描述】

创建VI通道。

【语法】

`int MPI_VI_CreateChn(HGVI_CONFIG_S& param);`

【参数】

| 参数名称 | 描述         | 输入/输出 |
| -------- | ------------ | --------- |
| param    | 通道配置参数 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

<SdkRoot>/example/test-vi/main/src/main.cpp



##### 4.1.1.2 MPI_VI_StartStream

【描述】

开启VI通道数据流。

【语法】

`int MPI_VI_StartStream(int chn);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| chn      | 通道数字 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

<SdkRoot>/example/test-vi/main/src/main.cpp



##### 4.1.1.3 MPI_VI_DestoryChn

【描述】

销毁VI通道。

【语法】

`int MPI_VI_DestoryChn(int chn);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| chn      | 通道数字 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

<SdkRoot>/example/test-vi/main/src/main.cpp



##### 4.1.1.4 MPI_VPSS_CreateChn

【描述】

创建VPSS通道。

【语法】

`int MPI_VPSS_CreateChn(HGVPSS_CONFIG_S& param);`

【参数】

| 参数名称 | 描述         | 输入/输出 |
| -------- | ------------ | --------- |
| param    | 通道配置参数 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

<SdkRoot>/example/test-vi/main/src/main.cpp



##### 4.1.1.5 MPI_VPSS_DestoryChn

【描述】

创建VPSS通道。

【语法】

`int MPI_VPSS_DestoryChn(int chn);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| chn      | 通道数字 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

<SdkRoot>/example/test-vi/main/src/main.cpp



##### 4.1.1.6 MPI_VENC_CreateChn

【描述】

创建VPSS通道。

【语法】

`int MPI_VENC_CreateChn(HGVENC_CONFIG_S& param);`

【参数】

| 参数名称 | 描述         | 输入/输出 |
| -------- | ------------ | --------- |
| param    | 通道配置参数 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

test-venc



##### 4.1.1.7 MPI_VENC_DestoryChn

【描述】

创建VPSS通道。

【语法】

`int MPI_VENC_DestoryChn(int chn);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| chn      | 通道数字 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

test-venc



##### 4.1.1.8 MPI_SYS_Init

【描述】

初始化所有media模块

【语法】

`int MPI_SYS_Init();`

【参数】

无

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

test-vi、test-venc

##### 4.1.1.9 MPI_SYS_Bind

【描述】

通道绑定。

【语法】

`int MPI_SYS_Bind(const HGMPP_CHN_S* src, const HGMPP_CHN_S* dst);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| src      | 通道描述 | 输入      |
| dst      | 通道描述 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

test-venc

##### 4.1.1.10 MPI_SYS_UnBind

【描述】

通道解绑。

【语法】

`int MPI_SYS_UnBind(const HGMPP_CHN_S* src, const HGMPP_CHN_S* dst);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| src      | 通道描述 | 输入      |
| dst      | 通道描述 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmedia.h

库文件：libhgmedia.a

【注意】

【举例】

test-venc


#### 4.1.2 数据类型

##### 4.1.2.1 HGBOOL

【说明】



【定义】

```
typedef enum {
  FALSE = 0,
  TURE = 1,
} HGBOOL;
```



##### 4.1.2.2 HGMOD_ID_E

【说明】

枚举，模式ID号

【定义】

```
typedef enum _MOD_ID_E {
  HGID_UNKNOW = 0,  
  HGID_VENC = 4,
  HGID_VI = 9,
  HGID_VPSS = 17,
  HGID_BUTT,
} HGMOD_ID_E;
```



##### 4.1.2.3 HGIMAGE_TYPE_E

【说明】

枚举，图像类型

【定义】

```
typedef enum _IMAGE_TYPE_E {
  HGIMAGE_TYPE_UNKNOW = 0,
  HGIMAGE_TYPE_NV12 = 4,
  HGIMAGE_TYPE_RGB888 = 18,
  HGIMAGE_TYPE_BGR888 = 19,
  HGIMAGE_TYPE_BUTT   
} HGIMAGE_TYPE_E;
```



##### 4.1.2.4 HGIMAGE_INFO_S

【说明】

图像参数配置

【定义】

```
typedef struct _IMAGE_INFO_S {
  uint32_t width;
  uint32_t height;
  uint32_t horStride;
  uint32_t verStride;
  HGIMAGE_TYPE_E enImgType;
} HGIMAGE_INFO_S;
```

【成员】

| 成员名称  | 描述         |
| --------- | ------------ |
| width     | 图像宽度     |
| height    | 图像高度     |
| horStride | 图像水平对齐 |
| verStride | 图像垂直对齐 |
| enImgType | 图像类型     |



##### 4.1.2.5 HGMPP_CHN_S

【说明】



【定义】

```
typedef struct _dfMPP_CHN_S {
  HGMOD_ID_E modId;
  int devId;
  int chnId;
} HGMPP_CHN_S;
```



##### 4.1.2.6 HGVI_CONFIG_S

【说明】



【定义】

```
typedef struct _VI_CONFIG_S
{
  char* dev;
  int chn;
  int width;
  int height;
} HGVI_CONFIG_S;
```



##### 4.1.2.7 HGVPSS_CONFIG_S

【说明】



【定义】

```
typedef struct _VPSS_CONFIG_S
{
  int chn;
  HGIMAGE_TYPE_E srcType;
  int srcWidth;
  int srcHeight;
  int srcHorStride;
  int srcVirStride;
  HGIMAGE_TYPE_E dstType;
  int dstWidth;
  int dstHeight;
  int dstHorStride;
  int dstVirStride;
} HGVPSS_CONFIG_S;
```



##### 4.1.2.8 HGVENC_CONFIG_S

【说明】



【定义】

```
typedef struct _VENC_CONFIG_S
{
  int chn;
  int width;
  int height;
  int virWidth;
  int virHeight;
  int profile;
  int gop;
  int bitRate;
  int qpSet;
  int startQp;
  int stepQp;
  int maxIQp;
  int minIQp;
  int maxQp;
  int minQp;
} HGVENC_CONFIG_S;
```





### 4.2 muxer
该模块负责视频格式封装，目前提供MP4格式的封装

#### 4.2.1 API参考

##### 4.2.1.1 mp4_muxer_create

【描述】

创建 mp4 muxer context

【语法】

`void mp4_muxer_create(void* param, int width, int height);`

【参数】

| 参数名称 | 描述           | 输入/输出 |
| -------- | -------------- | --------- |
| param    | 文件描述符指针 | 输入      |
| width    | 图像宽度       | 输入      |
| height   | 图像高度       | 输入      |

【返回值】

无

【需求】

头文件：hgmuxer.h

库文件：libhgmuxer.a

【注意】

【举例】

<SdkRoot>/example/test-mp4/main/src/main.cpp



##### 4.2.1.1 mp4_muxer_destroy

【描述】

销毁mp4 muxer

【语法】

`void mp4_muxer_destroy();`

【参数】

无

【返回值】

无

【需求】

头文件：hgmuxer.h

库文件：libhgmuxer.a

【注意】

【举例】

<SdkRoot>/example/test-mp4/main/src/main.cpp



##### 4.2.1.1 mp4_muxer_write

【描述】

初始化所有media模块

【语法】

`int mp4_muxer_write(const void* data, int size);`

【参数】

| 参数名称 | 描述             | 输入/输出 |
| -------- | ---------------- | --------- |
| data     | h264码流数据指针 | 输入      |
| size     | h264码流大小     | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmuxer.h

库文件：libhgmuxer.a

【注意】

【举例】

<SdkRoot>/example/test-mp4/main/src/main.cpp



#### 4.2.2 数据类型

无



## 5 网络协议组件

### 5.1 net
通用网络接口

#### 5.1.1 API参考

##### 5.1.1.1 net_connect_init

【描述】

tcp网络通信服务端初始化

【语法】

`NetCon *net_connect_init(int ( *nc_open)(NetCon *nc,void *data), int ( *nc_close)(NetCon *nc, void *data), `
`int ( *nc_read)(NetCon *nc, void *data), int ( *nc_write)(NetCon *nc, void *data));`

【参数】

|               参数名称               |      描述          | 输入/输出  |
| ------------------------------------| -------------------| --------- |
| ( *nc_open)(NetCon *nc,void *data)  | 打开操作回调函数     |   输入    |
| ( *nc_close)(NetCon *nc,void *data) | 关闭操作回调函数     |   输入    |
| ( *nc_read)(NetCon *nc,void *data)  | 读取数据操作回调函数 |   输入    |
| ( *nc_write)(NetCon *nc,void *data) | 发送数据操作回调函数 |   输入    |


【返回值】

|      返回值     |  描述 |
| --------------- | -----|
| NetCon结构体指针 | 成功 |
|      NULL       | 失败 |

【需求】

头文件：hg_net_protocol.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-tcpserver/main/src//test-tcpserver.cpp

##### 5.1.1.2 net_uninit

【描述】

tcp网络通信服务端初始化

【语法】

`void net_uninit(NetCon *nc);`

【参数】

| 参数名称 |        描述      | 输入/输出  |
| ------- | -----------------| --------- |
|   nc    | 服务端网络描述结构 |   输入    |


【返回值】

|  返回值  |  描述  |
| -------  | ----- |
|     无   |  无   |  


【需求】

头文件：hg_net_protocol.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-tcpserver/main/src/test-tcpserver.cpp

##### 5.1.1.3 ncio_snd

【描述】

tcp网络通信服务端数据发送

【语法】

`int ncio_snd(NetCon *nc, void *data, int len);`

【参数】

| 参数名称 |              描述            | 输入/输出  |
| ------- | ---------------------------- | --------- |
|   nc    | 初始化后返回的NetCon结构体指针 |   输入     |
|   data  | 要发送的数据                  |   输入     |
|   len   | 要发送数据的长度              |   输入     |



【返回值】

|  返回值  |    描述    |
| -------  | --------- |
|    >0    |  成功      |
|    =0    |  网络错误  | 
|    <0    |  发送出错  |   


【需求】

头文件：hg_net_protocol.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-tcpserver/main/src/test-tcpserver.cpp


##### 5.1.1.4 ncio_rcv

【描述】

tcp网络通信服务端数据接收

【语法】

`int ncio_rcv(NetCon *nc, void *data, int len);`

【参数】

| 参数名称 |              描述            | 输入/输出  |
| ------- | ---------------------------- | --------- |
|   nc    | 初始化后返回的NetCon结构体指针 |   输入     |
|   data  | 要接收的数据                  |   输入     |
|   len   | 要接收数据的长度              |   输入     |



【返回值】

|  返回值  |    描述    |
| -------  | --------- |
|    >0    |  成功      |
|    =0    |  网络关闭  | 
|    <0    |  发送出错  |   


【需求】

头文件：hg_net_protocol.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-tcpserver/main/src/test-tcpserver.cpp


##### 5.1.1.5 sys_error

【描述】

网络初始化失败时，释放资源

【语法】

`void sys_error(NetCon *nc);`

【参数】

| 参数名称 |              描述               | 输入/输出  |
| ------- | ------------------------------- | --------- |
|   nc    | 释放初始化后返回的NetCon结构体指针 |   输入     |



【返回值】

|  返回值  |    描述    |
| -------  | --------- |
|    无    |    无     |


【需求】

头文件：hg_net_protocol.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-tcpserver/main/src/test-tcpserver.cpp


##### 5.1.1.6 net_data_listen

【描述】

服务端网络数据监听线程

【语法】

`static void *net_data_listen(NetCon *nc);`

【参数】

| 参数名称 |              描述               | 输入/输出  |
| ------- | ------------------------------- | --------- |
|   nc    | 初始化后返回的NetCon结构体指针    |   输入     |



【返回值】

|  返回值  |    描述    |
| -------  | --------- |
|    无    |    无     |


【需求】

头文件：hg_net_protocol.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-tcpserver/main/src/test-tcpserver.cpp

#### 5.1.2 数据类型
struct NetCon {
  int sfd; 
  int cfd; 
  bool run; 
  int con_cnt; 
  pthread_t thread; 

  int ( *nc_open)(NetCon *nc, void *data);  
  int ( *nc_close)(NetCon *nc, void *data);
  int ( *nc_read)(NetCon *nc, void *data);
  int ( *nc_write)(NetCon *nc, void *data);
};



### 5.2 rtsp
提供标准RTSP直播流的基本服务，可实现视频实时流传输。

实例类RtspServer，提供以下类方法

#### 5.2.1 API参考

##### 5.2.1.1 start

【描述】

开启RTSP服务。

【语法】

`void start();`

【参数】

无

【返回值】

无

【需求】

头文件：hgrtsp.h

库文件：libhgrtsp.a

【注意】

【举例】

test-rtsp



##### 5.2.1.2 stop

【描述】

关闭RTSP服务。

【语法】

`void stop();`

【参数】

无

【返回值】

无

【需求】

头文件：hgrtsp.h

库文件：libhgrtsp.a

【注意】

【举例】

test-rtsp



##### 5.2.1.3 getStatus

【描述】

获取RTSP服务状态

【语法】

`bool getStatus();`

【参数】

无

【返回值】

| 返回值 | 描述     |
| ------ | -------- |
| true   | 服务开启 |
| false  | 服务关闭 |

【需求】

头文件：hgrtsp.h

库文件：libhgrtsp.a

【注意】

【举例】

test-rtsp



##### 5.2.1.4 setBufferAllocCallback

【描述】

注册获取数据回调

【语法】

`voidsetBufferAllocCallback(const BufferAllocCallback& cb);`

【参数】

| 参数名称            | 描述         | 输入/输出 |
| ------------------- | ------------ | --------- |
| BufferAllocCallback | 回调函数指针 | 输入      |

【返回值】

无

【需求】

头文件：hgrtsp.h

库文件：libhgrtsp.a

【注意】

【举例】

test-rtsp



##### 5.2.1.4 setBufferAllocCallback

【描述】

注册释放数据回调

【语法】

`voidsetBufferAllocCallback(const BufferFreeCallback& cb);`

【参数】

| 参数名称           | 描述         | 输入/输出 |
| ------------------ | ------------ | --------- |
| BufferFreeCallback | 回调函数指针 | 输入      |

【返回值】

无

【需求】

头文件：hgrtsp.h

库文件：libhgrtsp.a

【注意】

【举例】

test-rtsp



#### 5.2.2 数据类型

##### 5.2.2.1 H264Attr

【说明】



【定义】

```
struct H264Attr
{
    void* data;
    int size;
    uint32_t timestamp;
};
```





## 6 外设组件

### 6.1 uart
串口通用接口

#### 6.1.1 API参考
##### 6.1.1.1 uart_open

【描述】

打开串口设备

【语法】

`int uart_open(const char* dev, int speed) `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| dev      | 串口设备  |   输入    |
| speed    | 波特率    |   输入    |



【返回值】

| 返回值 |  描述 |
| ------| ------|
| > 0   | 成功  |
| <= 0  | 失败  |

【需求】

头文件：uart.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/hgapp


##### 6.1.1.2 uart_close

【描述】

关闭串口设备

【语法】

`void uart_close(int fd) `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| fd      | 文件描述符 |   输入    |


【返回值】

| 返回值 |  描述 |
| ------| ------|
|   无  |   无  |


【需求】

头文件：uart.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/hgapp


##### 6.1.1.3 uart_set_opt

【描述】

设置串口属性

【语法】

`int uart_set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop) `

【参数】

| 参数名称  |   描述    | 输入/输出  |
| ---------| ----------| --------- |
| fd       | 文件描述符 |   输入    |
| nSpeed   | 波特率     |   输入    |
| nBits    | 数据位     |   输入    |
| nEvent   | 奇偶校验位 |   输入    |
| nStop    | 停止位     |   输入    |


【返回值】

| 返回值 |  描述 |
| ------| ------|
|   0  |   成功  |


【需求】

头文件：uart.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/hgapp

##### 6.1.1.4 uart_send

【描述】

串口数据发送

【语法】

`int uart_send(int fd, const unsigned char * buf, int len)`

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| fd      | 文件描述符 |   输入    |
| buf     | 发送的数据 |   输入    |
| len     | 数据长度   |   输入    |

【返回值】

| 返回值 |  描述    |
| ------| ---------|
|  > 0  | 数据长度  |
|  <= 0 | 发送错误  |

【需求】

头文件：uart.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/hgapp

##### 6.1.1.5 uart_recv

【描述】

串口数据接收

【语法】

`int uart_recv(int fd, char* buf, int len, int timeout_ms) `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| fd      | 文件描述符 |   输入    |
| buf     | 发送的数据 |   输入    |
| len     | 数据长度   |   输入    |


【返回值】

| 返回值 |  描述    |
| ------| ---------|
|  > 0  | 数据长度  |
|  <= 0 | 发送错误  |


【需求】

头文件：uart.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/hgapp


#### 6.1.2 数据类型
无

### 6.2 wifi
WiFi通用接口

#### 6.2.1 API参考
##### 6.2.1.1 console_run

【描述】

执行wifi设备操作语句

【语法】

`const bool console_run(const char *cmdline) `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| cmdline  |   指令   |   输入    |



【返回值】

| 返回值  |  描述 |
| -------| ----- |
|  true  | 成功  |
|  false | 失败  |


【需求】

头文件：hgwifi.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-wifi/main/src/hgwifi.cpp

##### 6.2.1.2 get_pid

【描述】

获取 pid

【语法】

`int get_pid(const char *Name) `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| Name      | 热点名称 |   输入    |



【返回值】

| 返回值 |  描述    |
| ------| ---------|
|  > 0  | pid      |
|  <= 0 | 错误     |


【需求】

头文件：hgwifi.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-wifi/main/src/hgwifi.cpp

##### 6.2.1.3 get_dnsmasq_pid

【描述】

获取dns pid

【语法】

`int get_dnsmasq_pid() `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| 无       | 无       |   无       |



【返回值】

| 返回值 |  描述    |
| ------| ---------|
|  > 0  | dns pid  |
|  <= 0 | 错误     |


【需求】

头文件：hgwifi.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-wifi/main/src/hgwifi.cpp

##### 6.2.1.4 get_hostapd_pid

【描述】

获取hostapd pid

【语法】

`int get_hostapd_pid() `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| 无       | 无       |   无       |



【返回值】

| 返回值 |  描述        |
| ------| -------------|
|  > 0  | hostapd pid  |
|  <= 0 | 错误         |


【需求】

头文件：hgwifi.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-wifi/main/src/hgwifi.cpp

##### 6.2.1.5 wifi_rtl_stop_hostapd

【描述】

关闭ap模式

【语法】

`int wifi_rtl_stop_hostapd()`

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| 无       | 无       |   无       |

【返回值】

| 返回值 |  描述        |
| ------| -------------|
|   0   | 成功         |
|  非 0 | 失败         |


【需求】

头文件：hgwifi.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-wifi/main/src/hgwifi.cpp

##### 6.2.1.6 create_hostapd_file

【描述】

创建hostapd文件

【语法】

`int create_hostapd_file(const char* name, const char* password, char *softap_name) `

【参数】

| 参数名称     |   描述        | 输入/输出  |
| ------------| --------------| --------- |
| name        |  ssid         |   输入    |
| password    | 密码          |   输入    |
| softap_name | 热点设备名称   |   输入    |


【返回值】

| 返回值 |  描述    |
| ------| ---------|
|  = 0  | 成功     |
|  < 0 | 失败      |


【需求】

头文件：hgwifi.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-wifi/main/src/hgwifi.cpp

##### 6.2.1.7 creat_dnsmasq_file

【描述】

创建dnsmasq文件

【语法】

`bool creat_dnsmasq_file() `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| 无       | 无       |   无       |




【返回值】

| 返回值  |  描述 |
| -------| ----- |
|  true  | 成功  |
|  false | 失败  |


【需求】

头文件：hgwifi.h

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-wifi/main/src/hgwifi.cpp

##### 6.2.1.8 wlan_access_point_start

【描述】

开启ap模式

【语法】

`iint wlan_access_point_start(const char* ssid, const char* password, char *softap_name) `

【参数】

| 参数名称     |   描述        | 输入/输出  |
| ------------| --------------| --------- |
| name        |  ssid         |   输入    |
| password    | 密码          |   输入    |
| softap_name | 热点设备名称   |   输入    |


【返回值】

| 返回值 |  描述 |
| ------| ------|
|  > 0  | 成功  |
|  <= 0 | 失败  |

【需求】

头文件：hgwifi.h

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-wifi/main/src/hgwifi.cpp

##### 6.2.1.9 check_wifi_chip_type_string

【描述】

检查wifi芯片类型

【语法】

`int check_wifi_chip_type_string(char *type) `

【参数】

| 参数名称  |   描述      | 输入/输出  |
| ---------| ------------| --------- |
| type     | 检查wifi类型 |   输入    |



【返回值】

| 返回值 |  描述 |
| ------| ------|
|   0   | 成功  |
|  -1   | 失败  |


【需求】

头文件：hgwifi.h

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-wifi/main/src/hgwifi.cpp


#### 6.2.2 数据类型


### 6.3 tof
提供外设红外感知传感器的接口

#### 6.3.1 API参考
##### 6.3.1.1 tof_parse_char

【描述】

tof 数据解析

【语法】

`int tof_parse_char(uint8_t b, tof_data_t *data);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| b       | 字符 | 输入      |
| data      | 解析完的信息 | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 1      | 成功 |
| 非1    | 失败 |

【需求】

头文件：hg_tof.h

库文件：libtof.a

【注意】

【举例】

<SdkRoot>/example/test-tof/main/src/main.cpp


#### 6.3.2 数据类型

##### 6.3.2.1 tof_data_t

【说明】

tof 解析数据信息

【定义】
```
typedef struct {
  int height;   // tof 高度信息，单位cm
  int strength; // tof 强度信息
} tof_data_t;   
```

### 6.6 mount
提供外设云台的控制接口

#### 6.6.1 API参考
##### 6.6.1.1 ptz_parse_char

【描述】

云台数据解析

【语法】

`int ptz_parse_char(const uint8_t b, ptz_payload_rx_data_status_t* msg);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| b       | 字符 | 输入      |
| msg      | 解析完的信息 | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 1      | 成功 |
| 非1    | 失败 |

【需求】

头文件：hg_ptz.h

库文件：libptz.a

【注意】

【举例】

<SdkRoot>/example/test-ptz/main/src/main.cpp

##### 6.6.2.1 ptz_pack_pitch_to_buffer

【描述】

控制云台俯仰

【语法】

`uint16_t ptz_pack_pitch_to_buffer(int16_t pitch, uint8_t *buffer);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| pitch       | 俯仰角度，+-[0, 9000] 0.01deg/s| 输入      |
| buffer      | 输出的协议控制buffer | 输出      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| size      | 输出控制协议buffer的size |

【需求】

头文件：hg_ptz.h

库文件：libptz.a

【注意】

【举例】

<SdkRoot>/example/test-ptz/main/src/main.cpp


#### 6.6.2 数据类型
##### 6.3.2.1 ptz_payload_rx_data_status_t

【说明】

云台解析数据信息

【定义】
```
typedef struct {
	int16_t roll;       // 预留 
    int16_t pitch;      // 云台俯仰角度 单位0.01deg
    int16_t yaw;        // 预留
    int16_t error_yaw;  // 预留
    int16_t  zoom_rate; // 预留
} ptz_payload_rx_data_status_t;  
```





### 6.5 fightctrl
提供外设飞控的控制接口

#### 6.5.1 API参考

##### 6.5.1.1 flightctrl_create

【描述】

创建初始化。

【语法】

`int flightctrl_create();`

【参数】

无

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.2 flightctrl_destroy

【描述】

注销。

【语法】

`int flightctrl_destroy();`

【参数】

无

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.3 flightctrl_register

【描述】

创建初始化。

【语法】

**void** flightctrl_register(**struct** mav_handler_t *****handler);

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| handler  | 回调句柄 | 输入      |

【返回值】

无

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.4 flightctrl_set_id

【描述】

设置飞机的编号

【语法】

`int flightctrl_set_id(int num);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| num      | 飞机编号 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.5 flightctrl_lock

【描述】

飞机上锁

【语法】

`int flightctrl_lock();`

【参数】

无

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.6 flightctrl_unlock

【描述】

飞机解锁

【语法】

`int flightctrl_unlock();`

【参数】

无

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.7 flightctrl_takoff

【描述】

飞机起飞

【语法】

`int flightctrl_takeoff(int height, int temperature);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| height   | 起飞高度 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.8 flightctrl_land

【描述】

飞机降落

【语法】

`int flightctrl_land();`

【参数】

无

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.9 flightctrl_up

【描述】

飞机向上飞行

【语法】

`int flightctrl_up(int cm);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| cm       | 飞行距离 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.10 flightctrl_down

【描述】

飞机向下飞行

【语法】

`int flightctrl_down(int cm);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| cm       | 飞行距离 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.11 flightctrl_left

【描述】

飞机向左飞行

【语法】

`int flightctrl_left(int cm);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| cm       | 飞行距离 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.12 flightctrl_right

【描述】

飞机向右飞行

【语法】

`int flightctrl_up(int cm);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| cm       | 飞行距离 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.13 flightctrl_forward

【描述】

飞机向前飞行

【语法】

`int flightctrl_forward(int cm);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| cm       | 飞行距离 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.14 flightctrl_back

【描述】

飞机向后飞行

【语法】

`int flightctrl_back(int cm);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| cm       | 飞行距离 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.15 flightctrl_cw

【描述】

飞机顺时针旋转

【语法】

`int flightctrl_cw(int degree);`

【参数】

| 参数名称 | 描述              | 输入/输出 |
| -------- | ----------------- | --------- |
| degree   | 旋转角度，[1-360] | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.16 flightctrl_ccw

【描述】

飞机逆时针旋转

【语法】

`int flightctrl_ccw(int degree);`

【参数】

| 参数名称 | 描述              | 输入/输出 |
| -------- | ----------------- | --------- |
| degree   | 旋转角度，[1-360] | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl



##### 6.5.1.16 flightctrl_go

【描述】

飞机以设置的速度飞到坐标(x,y,z)

【语法】

`int flightctrl_go(int x, int y, int z, int speed);`

【参数】

| 参数名称 | 描述      | 输入/输出 |
| -------- | --------- | --------- |
| x        | x坐标数值 | 输入      |
| y        | y坐标数值 | 输入      |
| z        | z坐标数值 | 输入      |
| speed    | 飞行速度  | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| 0      | 成功 |
| 非0    | 失败 |

【需求】

头文件：hgmav.h

库文件：libhgmav.a

【注意】

【举例】

test-flightctrl

#### 6.5.2 数据类型


### 6.6 servo
提供外设舵机的控制接口

#### 6.6.1 API参考
##### 6.6.1.1 hg_steering_engine_init

【描述】

舵机设备初始化

【语法】

`int hg_steering_engine_init() `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| 无       |   无     |   无      |



【返回值】

| 返回值  |  描述 |
| -------| ----- |
|  1     | 成功  |
|  非1   | 失败  |


【需求】

头文件：hg_steering_engine.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-servo/main/src/test-servo.cpp


##### 6.6.1.2 hg_steering_engine_write

【描述】

设置舵机参数

【语法】

`hg_steering_engine_write(HgSteeringEngine *wStreeingData) `

【参数】

| 参数名称       |   描述          | 输入/输出  |
| ------------- | --------------- |  --------- |
| wStreeingData | 舵机参数        |   输入     |


【返回值】

| 返回值  |  描述 |
| -------| ----- |
|  1     | 成功  |
|  非1   | 失败  |


【需求】

头文件：hg_steering_engine.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-servo/main/src/test-servo.cpp

##### 6.6.1.3 hg_steering_engine_read

【描述】

读取舵机设置参数

【语法】

`int hg_steering_engine_read(HgSteeringEngine *rStreeingData)`

【参数】

| 参数名称       |   描述          | 输入/输出  |
| ------------- | --------------- |  --------- |
| wStreeingData | 读取的舵机数据   |   输入     |


【返回值】

| 返回值  |  描述 |
| ------- | ----- |
|  <=0    | 失败  |
|  >0     | 成功  |


【需求】

头文件：hg_steering_engine.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-servo/main/src/test-servo.cpp

##### 6.6.1.4 hg_steering_engine_parse

【描述】

打印读取的舵机数据

【语法】

`int hg_steering_engine_parse(HgSteeringEngine *parse)`

【参数】

| 参数名称       |   描述          | 输入/输出  |
| ------------- | --------------- |  --------- |
| parse         | 读取的舵机数据   |   输入     |



【返回值】

| 返回值  |  描述 |
| -------| ----- |
|  1     | 成功  |
|  非1   | 失败  |


【需求】

头文件：hg_steering_engine.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-servo/main/src/test-servo.cpp

##### 6.6.1.5 hg_clkwise_rota_angle

【描述】

设置舵机占空比，控制旋转角度

【语法】

`int hg_clkwise_rota_angle(uint8_t dutyCycle) `

【参数】

| 参数名称    |   描述   | 输入/输出  |
| -----------| ---------| --------- |
| dutyCycle  |   pwm周期 |   输入    |



【返回值】

| 返回值  |  描述 |
| -------| ----- |
|  1     | 成功  |
|  非1   | 失败  |


【需求】

头文件：hg_steering_engine.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-servo/main/src/test-servo.cpp

##### 6.6.1.6 hg_steering_engine_freq

【描述】

设置舵机频率

【语法】

`int hg_steering_engine_freq(uint32_t freq) `

【参数】

| 参数名称  |   描述   | 输入/输出  |
| ---------| ---------| --------- |
| freq     |   频率   |   输入     |



【返回值】

| 返回值  |  描述 |
| -------| ----- |
|  1     | 成功  |
|  非1   | 失败  |


【需求】

头文件：hg_steering_engine.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-servo/main/src/test-servo.cpp

##### 6.6.1.7 hg_steering_engine_uninit

【描述】

舵机设备反初始化

【语法】

`int hg_steering_engine_uninit(HgSteeringEngine *steeringEngine)`

【参数】

| 参数名称         |   描述    | 输入/输出  |
| ----------------| ----------| --------- |
| steeringEngine  | 舵机结构体 |   输入    |



【返回值】

| 返回值  |  描述 |
| -------| ----- |
|  1     | 成功  |
|  非1   | 失败  |


【需求】

头文件：hg_steering_engine.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-servo/main/src/test-servo.cpp

##### 6.6.1.8 hg_streeing_engine_crc

【描述】

crc数据校验

【语法】

`uint16_t hg_streeing_engine_crc(StreeingEngineData *dataCrc, uint8_t length) `

【参数】

| 参数名称 |   描述     | 输入/输出  |
| --------| ---------- | --------- |
| dataCrc | 数据帧     |   输入     |
| length  | 数据帧长度  |   输入     |


【返回值】

| 返回值  |  描述      |
| ---------| -------- |
|  crc    | 校验值     |


【需求】

头文件：hg_steering_engine.h 

库文件：无

【注意】

【举例】

<SdkRoot>/example/test-servo/main/src/test-servo.cpp



#### 6.6.2 数据类型

【说明】

舵机数据帧机构

【定义】


```
typedef struct {
  uint8_t head = 0xa5;
  uint8_t addr = 0x3d;
  uint8_t cmd;
  uint8_t data[4] = {0};
  uint8_t crc[2] = {0}; //crc[0]:crc_H, crc[1]:crc_L
}StreeingEngineData;


```

【说明】

舵机描述结构体

【定义】


```
typedef struct {
  int fd;
  bool isRunning = false;
  char rdata[3] = {0};
  StreeingEngineData seData;
}HgSteeringEngine;


```


### 6.7 remote
提供外设接收机的协议接口

#### 6.7.1 API参考
##### 6.7.1.1 ibus_decoder

【描述】

接收机协议解析

【语法】

`bool ibus_decoder(uint8_t ch);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| ch       | 字符 | 输入      |

【返回值】

| 返回值 | 描述 |
| ------ | ---- |
| ture      | 成功 |
| false    | 失败 |

【需求】

头文件：hg_rc.h

库文件：librc.a

【注意】

【举例】

<SdkRoot>/example/test-rc/main/src/main.cpp

##### 6.7.1.1 ibus_get_values

【描述】

接收机通道获取

【语法】

`void ibus_get_values(uint16_t *value, uint16_t num_value);`

【参数】

| 参数名称 | 描述     | 输入/输出 |
| -------- | -------- | --------- |
| value       | 通道数组 | 输出      |
| num_value       | 通道个数 | 输入      |

【返回值】
无

【需求】

头文件：hg_rc.h

库文件：librc.a

【注意】

【举例】

<SdkRoot>/example/test-rc/main/src/main.cpp


#### 6.7.2 数据类型
无


## 示例工程
### test-protocol
交互协议接口使用示例，协议的封包解包



### test-tcpsever
通用网络接口使用示例，创建 tcp server



### test-tcpclient
通用网络接口使用示例，创建 tcp client



### test-udp
通用网络接口使用示例， udp server、 udp client



###  test-wifi
外设wifi接口使用示例，配置wifi ap模式或sta模式等功能



### test-fightctl
外设飞控接口使用示例
```
// 程序用法

// 执行，飞行器解锁，5秒后上锁
#./test-flightctl
```


### test-ptz
外设云台接口使用示例，控制云台俯仰、获取云台相关信息
```
// 程序用法

// 执行，打印云台当前俯仰角度，同时云台向下转动
#./test-ptz
```

### test-rc
外设接收机接口使用示例，获取接收机8个通道值
```
// 程序用法

// 执行，打印接收机的8个通道值
#./test-rc
```


### test-servo
外设舵机接口使用示例



### test-tof
外设tof接口使用示例
```
// 程序用法

// 执行，打印tof 的高度和强度信息
#./test-tof
```

### test-log

日志接口使用示例
```
// 程序用法

// 执行，打印不同log等级的显示
#./test-log
```


### test-vi
媒体接口使用示例，获取摄像头数据保存为out_nv12.yuv文件

```
// 程序用法

// 默认输出文件/tmp/out_n12.yuv，保存10帧图像
#./test-vi-vpss
```



### test-vi-vpss
媒体接口使用示例，获取摄像头数据经过缩放处理保存为out_nv12.yuv文件

```
// 程序用法

// 默认输出文件/tmp/out_n12.yuv，按Ctrl+c 停止保存文件
#./test-vi-vpss
```



### test-venc
媒体接口使用示例，输入yuv文件编码成h264文件

```
// 程序用法

// 默认输入/tmp/1080p_nv12.yuv，默认输出/tmp/out.h264
#./test-venc
Or
// 指定输入和输出
#./test-venc -i/root/out_nv12.yuv -o/root/out.h264
```

### test-mp4
媒体接口使用示例，录像输出mp4文件

```
// 程序用法

// 输入/dev/video0，输出/tmp/out.mp4
#./test-mp4 -d/dev/video0 -o/tmp/out.mp4
```

### test-rtsp

流媒体接口使用示例，获取摄像头数据，编码h264，打包码流数据，然后rtsp发出去

```
// 程序用法

// 负载仓终端输入
#./test-rtsp

// PC VLC 测试
// 打开VLC -> 媒体 -> 打开网络串流 -> 输入下面URL即可看到实时流
// rtsp://<负载仓IP>:8554/live/ch0
// rtsp://192.168.144.114:8554/live/ch0
```



### hgapp
整合所有模块的全功能应用

