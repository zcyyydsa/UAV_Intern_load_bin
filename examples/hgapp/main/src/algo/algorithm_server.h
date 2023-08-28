#ifndef __ALGORITHM_SERVER_H_
#define __ALGORITHM_SERVER_H_

#include <thread>
#include <chrono>

//#include <mavlink.h>
//#include <mavlink_msg_heartbeat.h>
#include "hgmedia.h"
#include "hg_protocol_types.h"
#include "timeAlign.h"
#include "hg_track.h"
#include "hg_detect_digital.h"
#include <mutex>

#define TEST_TRACK_USE_CLOUD        0
#define TEST_SINGLE_MAV_TRACK       0
#define ONLY_TEST_TRACKING          0
#define TEST_GOTO_NAV_POINT         0

#define MAV_NUM                     3
#define VEHICLE_START_NUM           4
#define IMAGE_WIDTH			        1048//
#define IMAGE_HEIGHT			    780//

//if raw(2096,1560) resize to (1280,720) 
#define CAMERA_FOCAL_LEN_X          (511.3626) 
#define CAMERA_FOCAL_LEN_Y          (511.3626)
#define DIGITAL_MARKER_SIZE         (0.215) //meter
#define MAX_MAV_HEIGHT              (4.0)//meter
#define TRACK_MAX_MOVE              (0.4f)
#define MARKER_SIZE                 (0.21)

using namespace hgdf;

// typedef struct _sSwarmTrack{

//     uint64_t time_stamp;//时间戳
//     uint8_t fun_id;//0无功能运行，1集群围捕跟踪
//     uint8_t swarm_state;//集群状态机
//     uint8_t state[MAV_NUM];//每台飞机状态机
//     uint8_t state_status[MAV_NUM];//每台飞机状态机state所处的状态
//     //0 disable飞机不参与功能或强制下线，1 offline飞机不在线（飞机和上位机失联），2 onlne飞机在线，3 飞机在线并且跟踪正常， 4 飞机在线但跟踪目标丢失
//     uint8_t mav_status[MAV_NUM];
//     float pos[MAV_NUM*5];//3 台飞机世界坐标位置，数据排列为mav0(x,y,z,yaw,tracked vehicle id),mav1(x,y,z,yaw,tracked vehicle id),mav3(x,y,z,yaw,tracked vehicle id)
//     float vehicle0[4];//小车绝对坐标:x,y,z,yaw;yaw will not use
//     float vehicle1[4];
//     uint8_t mav_target[MAV_NUM+MAV_NUM];//value is 0,1,2...,7(代表相对于目标车不同的坐标偏移)

//     _sSwarmTrack(){
//         time_stamp=0;
//         fun_id=0;
//         swarm_state=0;
//         memset(state,0,sizeof(uint8_t)*MAV_NUM);
//         memset(state_status,0,sizeof(uint8_t)*MAV_NUM);
//         memset(mav_status,0,sizeof(uint8_t)*MAV_NUM);
//         memset(pos,0,sizeof(float)*MAV_NUM*5);
//     }
// }sSwarmTrack;

// typedef struct _sHostParmas{
//     float track_height;
//     float track_angle;
//     Point2f fence[5];//meter
//     int mav_mask[MAV_NUM];
//     _sHostParmas(){
//         track_height=2.5;
//         track_angle=30/57.3;
//         fence[0]=Point2f(-20,-20);
//         fence[1]=Point2f(-20,20);
//         fence[2]=Point2f(20,20);
//         fence[3]=Point2f(20,-20);
//         fence[5]=Point2f(0,10);
//     }
// }sHostParmas;

// typedef struct _sTrackSend{
//     uint64_t time_stamp;//时间戳
//     uint8_t fun_id;//0无功能运行，1集群围捕跟踪 (功能由上位机启动)
//     uint8_t state;//状态机，不同功能状态机定义不同
//     uint8_t state_status;//state所处的状态（例：state处于跟踪，state_status可以用来表征跟踪状态）
//     float tx;//目标位置（例如：被跟踪的目标）
//     float ty;//目标位置
//     float tz;//目标位置
// }sTrackSend;

typedef enum {
    TRACK_IDLE=-1,
	TRACK_INIT = 0,
	TRACK_RUNNING,
    EXCHANGE_TARGET,//mav id is 2,and marker id is changed
	TRACK_LOST
}eTrackStatus;

//state and swarm_state 
typedef enum eStateSwarm{
    IDLE=0,     //无人机处于关闭或未启动状态，等待指令或准备起飞
    INITING,    //定桩授时及飞机在线检测，在线后发送配置文件参数给飞机，飞机接收到回传成功接收消息
    INIT_DONE,
    TAKEOFF,    //race mav takeoff
    TAKEOFF_DONE,//takeoff done
    START_SWARM_TRACK, //start swarm track,cal mavs target position
    GOTO_START_POINT, //go to race start point,if done check to next state
    START_DETECT_DIGITAL, //detect and track digital marker,but will not control mav to track,goto next state: 状态切换条件：当检测到两台车辆上的数字标签并且加起来等于3后，开始集群跟踪模式
    SWARM_TRACKING,//
}eStateSwarm;

class ShareData;
class AlgorithmServer
{
public:
    AlgorithmServer();
    ~AlgorithmServer();

    void start();
    void stop();

private:
    //void handleMavlinkFromMav(const mavlink_message_t *msg);

private:
    void TrackThread();
    void TopicSubThread();
    void CtlMavToTrack();
    void SendResultControlMav(uint8_t cmd, float x, float y, float z, float yaw, int16_t speed);
    void StateManager();
    void CalTrackPos();
    void CtlCloudPitchByPos(float pitch);//pitch is rad,down is +
    void LockTrackCloudPitch();
    int TrackStatusCheck();
    int MavChangeTrackTarget();
    int GotoTargetVehicle();
    int GotoNavPoint();
    void SendTrackResultToApp();
    float Wrap_pi(float bearing);

private:
    std::thread m_trackThread;
    std::thread m_topicSubThread;
    bool m_trackStatus;
    bool m_topicStatus;

    int16_t m_mountPitch;
    float mCloudPitch;//rad

    ShareData* mpSD;
    HgTrack* mpTrack;
    cv::Rect mTrackRect;
    HgDDigtal* mpDDigital;
    vector<Marker> mMarkers;
    int mRectIsNew;
    eTrackStatus mTrackStatus;
    // hgprotocol_swarm_track_t mSTD;
    Mat mImBGR;
    float mTrackYaw;//该参数是被计算出来的
    float mTrackCloudPitch;//rad,when tracking, lock cloud angle to mTrackCloudPitch,该参数是被计算出来的
    float mTrackDist;
    float mTrackHeight;
    int mTrackVehicleId;//marker id mav need to track
    int mMavVehicleRela;//value is 0,1,2,...,7(代表相对于目标车不同的坐标偏移)
    // int mIsCandidate[3];
    Vec6f mMavTargetPos;//data(x,y,z,yaw,timestamp,quality),mav follow vehicle by set position
    double mTimeStart;
    double mTimeStartDelay;
    int16_t mLastSpeed;
    double mTimeLastTrack;
    int mMarkSize;
};

class ShareData
{
public:
    ShareData();
    ~ShareData();
    void SetSwarmTrackStatus(hgprotocol_swarm_track_t data);//3 mav
    int GetSwarmTrackStatus(hgprotocol_swarm_track_t& data,unsigned long long time_stamp=0);
    double GetTimeMs();
    int GetMavHeight(float& h);//ret 0 success,-1 failed
    void SetMavHeight(float h);
    int GetMavYaw(float& yaw);//ret 0 success,-1 failed
    void SetMavYaw(float yaw);
    int GetMavPos(Point3f& pos);//ret 0 success,-1 failed
    void SetMavPos0(Point3f pos);
    int GetVehiclePos(Vec6d& vehicle,int id);//ret 0 success,-1 failed
    void SetVehiclePos(Vec6d vehicle0,Vec6d vehicle1);
    void SetMavPos(Vec6d mav_pos,int mav_id);
    void GetMavPos(Vec6d& mav_pos,int mav_id);
public:
    TimeAlign* mpTimeAlign;
    hgprotocol_swarm_track_t mSTD;//swarm track status
    std::mutex mMutexSTD;
    hgprotocol_track_send_t mTrackSend;
    hgprotocol_host_parmas_t mHostParams;
    int mHostParamsInited;
    int mMavIdInited;
    int mMavId;
    int mMavPadId;//补位机ID,-1 uninitialized 
    std::mutex mMutexMavHeight;
    Point2d mMavHeight;//mav real time heith,(height,timstamp),timstamp ms
    std::mutex mMutexMavYaw;
    Point2d mMavYaw;//mav real time yaw,(yaw,timstamp),timstamp ms
    std::mutex mMutexMavPos;
    Vec4d mMavPos;//(x,y,z,timestamp) 
    std::mutex mMutexVehPos;
    Vec6d mVehicle[2];//data type[x,y,z,yaw,timestamp,quality]
    std::mutex mMutexAllMavPos;
    Vec6d mAllMavPos[MAV_NUM];
};

#endif