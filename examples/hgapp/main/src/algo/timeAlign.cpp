#include "timeAlign.h"
#include "pthread.h"

imu_frame TimeAlign::raw_imu[ATTI_SIZE_MAX] = {};

TimeAlign::TimeAlign()
{
    imu_curr_idx=0;
    offsetIndex=0;
    isOffsetFull=0;
}

TimeAlign::~TimeAlign()
{
}


int TimeAlign::AttiDataPrev(int index, int offset)
{
    return ((ATTI_SIZE_MAX + index - offset) % ATTI_SIZE_MAX);
}

int TimeAlign::AttiDataNext(int index, int offset)
{
    return ((index + offset) % ATTI_SIZE_MAX);
}

int TimeAlign::AttiOffsetNext(int index, int offset)
{
    return ((index + offset) % ATTI_SYSOFFSET_WINDOW);
}

//imu id timestamp is smaller than image timestamp
int TimeAlign::FindImuIndexByImgStamp(uint64_t imgTimeStamp){

   int nowIdx=imu_curr_idx;
   int maxIndex=32;
   //printf("raw_imu[nowIdx].timestamp-imgTimeStamp:%lld %d \n",raw_imu[nowIdx].timestamp-imgTimeStamp,nowIdx);
   while(maxIndex--){
        // printf("nowIdx:%d %lld\n",nowIdx,raw_imu[nowIdx].timestamp-imgTimeStamp);
        int64_t delta=((int64_t)raw_imu[nowIdx].timestamp-(int64_t)imgTimeStamp);
        if(delta<=0){		//2500(use one imu)  4000(use all imu:imu250HZ)
            return nowIdx;
        }
        nowIdx=AttiDataPrev(nowIdx,1);
  }
   return nowIdx;
}

int TimeAlign::SetImuData(mavlink_attitude_t tData){
    pthread_mutex_lock(&imu_data_mutex_);

    int imu_idx = AttiDataNext(imu_curr_idx, 1);
	raw_imu[imu_idx].x_rate_sensor=tData.rollspeed;
	raw_imu[imu_idx].y_rate_sensor=tData.pitchspeed;
	raw_imu[imu_idx].z_rate_sensor=tData.yawspeed;
	raw_imu[imu_idx].ground_distance=tData.ground_distance;
	raw_imu[imu_idx].roll		 = tData.roll;
	raw_imu[imu_idx].pitch 		 = tData.pitch;
	raw_imu[imu_idx].yaw		 = tData.yaw;
	raw_imu[imu_idx].timestamp=tData.timestamp;
	// raw_imu[imu_idx].systime=time;
	raw_imu[imu_idx].delta_time=tData.deltatime;
    //printf("%d \n",raw_imu[imu_idx].delta_time);
    // printf("SetImuData  tData.timestamp %lld tData.rollspeed %f  tData.ground_distance %f \n 0 \n",tData.timestamp,tData.rollspeed,tData.ground_distance);
	raw_imu[imu_idx].serialDiff=raw_imu[imu_idx].timestamp-raw_imu[imu_idx].systime;	
	imu_curr_idx=imu_idx;
    pthread_mutex_unlock(&imu_data_mutex_);
}

imu_frame TimeAlign::getImuData(){
  return raw_imu[imu_curr_idx];
}

fly_data TimeAlign:: getFlyData(){
  return raw_fly[imu_curr_idx];
}

// int TimeAlign::SetImgAttiTime(uint64_t cap_time)
// {
//     pthread_mutex_lock(&imu_data_mutex_);
//     last_img_atti_time=img_atti_time;
//     int flag=0;
//     double coff;
//     double k,b;
//     //find the near imu data from img_atti_time
//     int imuNearimg_index=FindImuIndexByImgStamp(cap_time);//small one 
//     int imuNearimg_index1=AttiDataNext(imuNearimg_index,1);//bigger one
//     int imuNearimg_index0=AttiDataNext(imuNearimg_index,1);//minimum one

//     int64_t time_min = cap_time - IMU_CYCLE*3;
//     int64_t time_max = cap_time + IMU_CYCLE*3;
//     if((int64_t)raw_imu[imuNearimg_index].timestamp>=time_min && (int64_t)raw_imu[imuNearimg_index].timestamp<=time_max){

//         if((int64_t)raw_imu[imuNearimg_index].timestamp<(int64_t)raw_imu[imuNearimg_index1].timestamp &&
//             (int64_t)raw_imu[imuNearimg_index1].timestamp>=time_min && (int64_t)raw_imu[imuNearimg_index1].timestamp<=time_max)
//         {
//             flag=0;
//             coff=double(cap_time-(int64_t)raw_imu[imuNearimg_index].timestamp)/double((int64_t)raw_imu[imuNearimg_index1].timestamp-(int64_t)raw_imu[imuNearimg_index].timestamp);
//             // printf("timeAlign coff%f, (%lld,%lld,%lld)\n",coff,cap_time/1000,raw_imu[imuNearimg_index].timestamp/1000,(int64_t)raw_imu[imuNearimg_index1].timestamp/1000);
//             params.RcvGyroRate[0]   = coff*raw_imu[imuNearimg_index1].x_rate_sensor  + (1.0-coff)*raw_imu[imuNearimg_index].x_rate_sensor;
//             params.RcvGyroRate[1]   = coff*raw_imu[imuNearimg_index1].y_rate_sensor  + (1.0-coff)*raw_imu[imuNearimg_index].y_rate_sensor;
//             params.RcvGyroRate[2]   = coff*raw_imu[imuNearimg_index1].z_rate_sensor  + (1.0-coff)*raw_imu[imuNearimg_index].z_rate_sensor;
//             params.sonar            = coff*raw_imu[imuNearimg_index1].ground_distance+ (1.0-coff)*raw_imu[imuNearimg_index].ground_distance;
//             params.attitude_angle[0]= coff*raw_imu[imuNearimg_index1].roll           + (1.0-coff)*raw_imu[imuNearimg_index].roll;
//             params.attitude_angle[1]= coff*raw_imu[imuNearimg_index1].pitch          + (1.0-coff)*raw_imu[imuNearimg_index].pitch;
//             params.attitude_angle[2]= coff*raw_imu[imuNearimg_index1].yaw            + (1.0-coff)*raw_imu[imuNearimg_index].yaw;
//             // params.timestamp       = raw_imu[imuNearimg_index].timestamp;// will not use,if use change and match it
//         }
//         else if(abs(int64_t(cap_time)-int64_t(raw_imu[imuNearimg_index].timestamp))<=IMU_CYCLE*2 )
//         {
//             flag=0;
//             params.RcvGyroRate[0]   = raw_imu[imuNearimg_index].x_rate_sensor;
//             params.RcvGyroRate[1]   = raw_imu[imuNearimg_index].y_rate_sensor;
//             params.RcvGyroRate[2]   = raw_imu[imuNearimg_index].z_rate_sensor;
//             params.sonar            = raw_imu[imuNearimg_index].ground_distance;
//             params.attitude_angle[0]= raw_imu[imuNearimg_index].roll;
//             params.attitude_angle[1]= raw_imu[imuNearimg_index].pitch;
//             params.attitude_angle[2]= raw_imu[imuNearimg_index].yaw;
//             // printf("");
//         }
//         else{
//             flag=-1;
//            // printf("Imu data timestamp is after image more than one frame!\n");
//         }
  
//     }
//     else{
//         flag=-1;
//         // printf("timeAlign  (%lld,%lld,%lld)\n",cap_time/1000,raw_imu[imuNearimg_index].timestamp/1000,(int64_t)raw_imu[imuNearimg_index1].timestamp/1000);
//     }

//     pthread_mutex_unlock(&imu_data_mutex_);
//     return flag;
// }

int TimeAlign::SetImgAttiTime(uint64_t cap_time,imu_frame imu_out)
{
    pthread_mutex_lock(&imu_data_mutex_);
    last_img_atti_time=img_atti_time;
    int flag=0;
    double coff;
    double k,b;
    //find the near imu data from img_atti_time
    int imuNearimg_index=FindImuIndexByImgStamp(cap_time);//small one 
    int imuNearimg_index1=AttiDataNext(imuNearimg_index,1);//bigger one
    int imuNearimg_index0=AttiDataNext(imuNearimg_index,1);//minimum one

    int64_t time_min = cap_time - (TIME_UNIT / IMU_RATE)*3;
    int64_t time_max = cap_time + (TIME_UNIT / IMU_RATE)*3;
    if((int64_t)raw_imu[imuNearimg_index].timestamp>=time_min && (int64_t)raw_imu[imuNearimg_index].timestamp<=time_max){

        if((int64_t)raw_imu[imuNearimg_index].timestamp<(int64_t)raw_imu[imuNearimg_index1].timestamp &&
            (int64_t)raw_imu[imuNearimg_index1].timestamp>=time_min && (int64_t)raw_imu[imuNearimg_index1].timestamp<=time_max)
        {
            flag=0;
            coff=double(cap_time-(int64_t)raw_imu[imuNearimg_index].timestamp)/double((int64_t)raw_imu[imuNearimg_index1].timestamp-(int64_t)raw_imu[imuNearimg_index].timestamp);
            // printf("timeAlign coff%f, (%lld,%lld,%lld)\n",coff,cap_time/1000,raw_imu[imuNearimg_index].timestamp/1000,(int64_t)raw_imu[imuNearimg_index1].timestamp/1000);
            imu_out.x_rate_sensor   = coff*raw_imu[imuNearimg_index1].x_rate_sensor  + (1.0-coff)*raw_imu[imuNearimg_index].x_rate_sensor;
            imu_out.y_rate_sensor   = coff*raw_imu[imuNearimg_index1].y_rate_sensor  + (1.0-coff)*raw_imu[imuNearimg_index].y_rate_sensor;
            imu_out.z_rate_sensor   = coff*raw_imu[imuNearimg_index1].z_rate_sensor  + (1.0-coff)*raw_imu[imuNearimg_index].z_rate_sensor;
            imu_out.ground_distance            = coff*raw_imu[imuNearimg_index1].ground_distance+ (1.0-coff)*raw_imu[imuNearimg_index].ground_distance;
            imu_out.roll= coff*raw_imu[imuNearimg_index1].roll           + (1.0-coff)*raw_imu[imuNearimg_index].roll;
            imu_out.pitch= coff*raw_imu[imuNearimg_index1].pitch          + (1.0-coff)*raw_imu[imuNearimg_index].pitch;
            imu_out.yaw= coff*raw_imu[imuNearimg_index1].yaw            + (1.0-coff)*raw_imu[imuNearimg_index].yaw;
            // imu_out.timestamp       = raw_imu[imuNearimg_index].timestamp;// will not use,if use change and match it
        }
        else if(cap_time-(int64_t)raw_imu[imuNearimg_index].timestamp<=TIME_UNIT / IMU_RATE )
        {
            flag=0;
            imu_out.x_rate_sensor   = raw_imu[imuNearimg_index].x_rate_sensor;
            imu_out.y_rate_sensor   = raw_imu[imuNearimg_index].y_rate_sensor;
            imu_out.z_rate_sensor   = raw_imu[imuNearimg_index].z_rate_sensor;
            imu_out.ground_distance            = raw_imu[imuNearimg_index].ground_distance;
            imu_out.roll= raw_imu[imuNearimg_index].roll;
            imu_out.pitch= raw_imu[imuNearimg_index].pitch;
            imu_out.yaw= raw_imu[imuNearimg_index].yaw;
            // printf("");
        }
        else{
            flag=-1;
           // printf("Imu data timestamp is after image more than one frame!\n");
        }
  
    }
    else{
        flag=-1;
        // printf("timeAlign  (%lld,%lld,%lld)\n",cap_time/1000,raw_imu[imuNearimg_index].timestamp/1000,(int64_t)raw_imu[imuNearimg_index1].timestamp/1000);
    }

    pthread_mutex_unlock(&imu_data_mutex_);
    return flag;
}
