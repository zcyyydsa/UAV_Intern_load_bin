#ifndef TIMEALIGN_H
#define TIMEALIGN_H



#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

typedef struct imu_frame  //receive
{
    float x_rate_sensor;
    float y_rate_sensor;
    float z_rate_sensor;
    float ground_distance;
    float roll;
    float pitch;
    float yaw;

    uint64_t timestamp;
    uint64_t systime;
    int16_t delta_time;
    int64_t serialDiff;

}__attribute__((packed)) imu_frame;

typedef struct fly_data  //receive
{
    uint64_t timestamp;
	float yaw; 
	float accx; 
	float accy; 
	float accz; 
	float vel_x; 
	float vel_y;
	float vel_z;
	float x; 
	float y; 
	float z;

}__attribute__((packed)) fly_data;


#if 0
typedef struct __mavlink_optical_flow_t
{
 uint64_t time_usec; /*< Timestamp (UNIX)*/
 uint64_t flow_timespan;
 float flow_comp_m_x; /*< Flow in meters in x-sensor direction, angular-speed compensated*/
 float flow_comp_m_y; /*< Flow in meters in y-sensor direction, angular-speed compensated*/
 float ground_distance; /*< Ground distance in meters. Positive value: distance known. Negative value: Unknown distance*/
 float flow_x; /*< Flow in pixels * 10 in x-sensor direction (dezi-pixels)*/
 float flow_y; /*< Flow in pixels * 10 in y-sensor direction (dezi-pixels)*/
 float flow_rot;
 uint8_t sensor_id; /*< Sensor ID*/
 uint8_t quality; /*< Optical flow quality / confidence. 0: bad, 255: maximum quality*/
 uint8_t rot_quality; /*< Optical flow rotation quality / confidence. 0: bad, 255: maximum quality*/
 uint8_t hover_flag;  /*<hover flag.>*/
 uint64_t result_timespan;
}__attribute__((packed)) mavlink_optical_flow_t;
#endif

typedef struct __mavlink_attitude_t {
 uint64_t timestamp; /*< Timestamp (milliseconds since system boot)*/
 float roll; /*< Roll angle (rad, -pi..+pi)*/
 float pitch; /*< Pitch angle (rad, -pi..+pi)*/
 float yaw; /*< Yaw angle (rad, -pi..+pi)*/
 float rollspeed; /*< Roll angular speed (rad/s)*/
 float pitchspeed; /*< Pitch angular speed (rad/s)*/
 float yawspeed; /*< Yaw angular speed (rad/s)*/
 float ground_distance;
 uint16_t deltatime;
}mavlink_attitude_t;



#define ATTI_SIZE_MAX 	128   //imu data buffer size
#define TIME_UNIT       (1000000) //time unit
#define IMG_FRAMERATE   (50)// //img frame rate
#define IMU_RATE        (100)//  //IMU frame rate
#define IMU_CYCLE       (1000000/IMU_RATE)
#define ATTI_DELAY      0 //(0.0065*TIME_UNIT)

#define FLY_DATA_RATE        (20)//  //IMU frame rate
#define FLY_DATA_CYCLE       (1000000/FLY_DATA_RATE)
#define ATTI_SYSOFFSET_WINDOW 128  //data align window size  , to avg the offset from stm32 to rk1108

class TimeAlign
{
public:
    TimeAlign();
    ~TimeAlign();

    uint64_t img_atti_time; //current img time in stm32 timeSys
    // int SetImgAttiTime(uint64_t cap_time); //set img time and set imu data
	int SetImgAttiTime(uint64_t cap_time,imu_frame imu_out);
    int SetImuData(mavlink_attitude_t tData);//receive imu data and set rk time
    // int SetFlyData(mavlink_report_flight_data_t tData);//receive fly data and set rk time
    imu_frame getImuData();
    fly_data getFlyData();
    int imu_curr_idx=0;//the new imu index in image call back
    int AttiDataPrev(int index, int offset); //to next index
    int AttiDataNext(int index, int offset);//to prev index

    static imu_frame raw_imu[ATTI_SIZE_MAX];  //imu data buffer
    fly_data raw_fly[ATTI_SIZE_MAX];  //fly data buffer
    uint64_t atti_stm32TorkOffset_bufs[ATTI_SYSOFFSET_WINDOW];//data align slide window ,
    int offsetIndex=0;
    int isOffsetFull=0;
    int AttiOffsetNext(int index, int offset); //to next align slide window index
    int FindImuIndexByImgStamp(uint64_t imgTimeStamp);//find the imu index which near to the img time stampe
    imu_frame imgImuFrameData;
    fly_data imgFlyFrameData;
    uint64_t last_img_atti_time;

    pthread_mutex_t imu_data_mutex_;
    pthread_mutex_t fly_data_mutex_;
};

#endif
