#ifndef __DEVICE_SERVER_H_
#define __DEVICE_SERVER_H_

#include <thread>
#include <chrono>
#include <mutex>
#include "uorb_topic.h"
#include "hg_wifi.h"
#include "uart.h"
#include "hg_ptz.h"
#include "hg_tof.h"
#include "hg_rc.h"
#include "hg_flightctrl.h"
#include "hg_flightctrl_types.h"
// #define D04_EDU   1

//using namespace hgdf;

class DeviceServer
{
public:
    DeviceServer();
    ~DeviceServer();

    int init();
    void start();
    void stop();

private:
    void ptzSetPitch(int16_t pitch);
    void ptzSetAcc(float ax, float ay, float az);
    void droneCtrl(drone_ctrl_t *msg);
    void sendAck2Msg(uint8_t cmd, int type, uint8_t result);

private:
    void TopicSubThread();
    void TofRecvThread();
    void RcRecvThread();
    void PtzRecvThread();
    int wifiModeSwitch(int32_t status);


private:
    std::thread m_topicSubThread;
    std::thread m_tofRecvThread;
    std::thread m_rcRecvThread;
    std::thread m_ptzRecvThread;
    std::once_flag wflag;
    bool m_topicStatus;
    bool m_tofStatus;
    bool m_rcStatus;
    bool m_ptzStatus;

    int m_ptzFd;
    int m_servoFd;
    int m_rcFd;
    int m_tofFd;

    flightctrl_hdr_t hdr;
    int16_t m_ptzCurDegree;
    int16_t m_droneId;
};

#endif