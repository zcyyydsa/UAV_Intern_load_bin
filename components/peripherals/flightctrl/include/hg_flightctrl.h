#ifndef __HG_FLIGHTCTRL_H_
#define __HG_FLIGHTCTRL_H_

#include "hg_flightctrl_types.h"

namespace hgdf {

/// @brief 初始化
/// @return 0-ok, <0-error
int flightctrl_create();

/// @brief 注销
void flightctrl_destroy();

/// @brief 注册飞控消息ack的回调
/// @param handler 
void flightctrl_register(flightctrl_hdr_t *hdr);

/// @brief 设置飞机编号
/// @param num : drone id
/// @return 0-ok, <0-error
int flightctrl_set_id(int num);

/// @brief 上锁
/// @return 0-ok, <0-error
int flightctrl_lock();

/// @brief 解锁
/// @return 0-ok, <0-error
int flightctrl_unlock();

/// @brief 起飞
/// @param 高度
/// @param 温度 
/// @return 0-ok, <0-error
int flightctrl_takeoff(int height, int temperature);

/// @brief 降落
/// @return 0-ok, <0-error
int flightctrl_land();

/// @brief 向上飞x厘米
/// @param cm [20-500]
/// @return 0-ok, <0-error
int flightctrl_up(int cm);

/// @brief 向下飞x厘米
/// @param cm [20-500]
/// @return 0-ok, <0-error
int flightctrl_down(int cm);

/// @brief 向左飞x厘米
/// @param cm [20-500]
/// @return 0-ok, <0-error
int flightctrl_left(int cm);

/// @brief 向右飞x厘米
/// @param cm [20-500]
/// @return 0-ok, <0-error
int flightctrl_right(int cm);

/// @brief 向前飞x厘米
/// @param cm [20-500]
/// @return 0-ok, <0-error
int flightctrl_forward(int cm);

/// @brief 向后飞x厘米
/// @param cm [20-500]
/// @return 0-ok, <0-error
int flightctrl_back(int cm);

/// @brief 停止运动并悬停，任何时候都可以
/// @return 0-ok, <0-error
int flightctrl_stop();

/// @brief 顺时针旋转x°
/// @param degree [1-360]
/// @return 0-ok, <0-error
int flightctrl_cw(int degree);

/// @brief 逆时针旋转x°
/// @param degree [1-360]
/// @return 0-ok, <0-error
int flightctrl_ccw(int degree);

/// @brief 以设置速度(cm/s)飞往坐标(x,y,z)
/// @param x 
/// @param y 
/// @param z 
/// @param speed 
/// @return 
int flightctrl_go(int x, int y, int z, int speed, int yaw);

int flightctrl_tof(int height, uint8_t quality, uint64_t timestampe);

int flightctrl_rc(uint16_t *val);

int flightctrl_command(flightctrl_cmd_t *cmd);

int flightctrl_testmsg();

}

#endif
