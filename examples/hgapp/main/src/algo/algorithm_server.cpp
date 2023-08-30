#include "algorithm_server.h"

#include "log.h"
#include "uorb_topic.h"

#include "hg_flightctrl_types.h"

TOPIC_DEFINE(track_send, hgprotocol_track_send_t);
TOPIC_DEFINE(drone_ctrl_algo, flightctrl_cmd_t);

AlgorithmServer::AlgorithmServer()
    : m_trackStatus(false)
    , m_topicStatus(false)
{
    mpSD=new ShareData();
    mpTrack=new HgTrack();
    mpDDigital=new HgDDigtal("/userdata/imcode_d04_0-3.yml");
    mRectIsNew=0;
    mTrackStatus=TRACK_IDLE;
    mTrackRect.x=0;
    mTrackRect.y=0;
    mTrackRect.width=10;
    mTrackRect.height=10;
    mTrackYaw=0;//该参数是被计算出来的
    mTrackCloudPitch=45/57.3;//rad,when tracking lock cloud angle to mTrackCloudPitch,该参数是被计算出来的
    mTrackDist=0.8;
    mTrackHeight=2.0;
    // mIsCandidate=0;
    mTimeStart=0;
    mLastSpeed=1*100;
    mTimeLastTrack=0;
    mMarkSize=MARKER_SIZE*CAMERA_FOCAL_LEN_X/2.0;
    mTrackVehicleId=0;
}

AlgorithmServer::~AlgorithmServer()
{
    if(mpSD)
    {
        delete mpSD;
        mpSD=nullptr;
    }
    if(mpTrack)
    {
        delete mpTrack;
        mpTrack=nullptr;
    }
    if(mpDDigital)
    {
        delete mpDDigital;
        mpDDigital=nullptr;
    }
}

void AlgorithmServer::start()
{
    m_trackStatus = true;
    m_trackThread = std::thread(std::bind(&AlgorithmServer::TrackThread, this));

    m_topicStatus = true;
    m_topicSubThread = std::thread(std::bind(&AlgorithmServer::TopicSubThread, this));
}

void AlgorithmServer::stop()
{
    m_trackStatus = false;
    m_trackThread.join();

    m_topicStatus = false;
    m_topicSubThread.join();
}

void AlgorithmServer::TopicSubThread()
{
	uint64_t mount_status_sub = 0;
    uint64_t swarm_track_sub = 0;
    uint64_t host_parmas_sub = 0;
    uint64_t drone_id_sub = 0;
    uint64_t drone_attitude_sub = 0;
    uint64_t drone_pos_acc_sub = 0;
    
	mount_status_t mount_status;
    hgprotocol_swarm_track_t msg_swarm;
    hgprotocol_host_parmas_t msg_host;

	if (topic_subscribe_auto(TOPIC_ID(mount_status), &mount_status_sub, (char*)"ptz", 10) == -1) {
        HGLOG_ERROR("can not subscribe ORB_ID(mount_status)");
    }
    if (topic_subscribe_auto(TOPIC_ID(swarm_track), &swarm_track_sub, (char*)"swarm_track", 10) == -1) {
        HGLOG_ERROR("can not subscribe ORB_ID(swarm_track)");
    }
    if (topic_subscribe_auto(TOPIC_ID(host_parmas), &host_parmas_sub, (char*)"host_parmas", 10) == -1) {
        HGLOG_ERROR("can not subscribe ORB_ID(host_parmas)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_id), &drone_id_sub, (char*)"drone_id", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(drone_id)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_attitude), &drone_attitude_sub, (char*)"drone_attitude", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(drone_id)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_pos_acc), &drone_pos_acc_sub, (char*)"drone_pos_acc", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(drone_id)");
    }

	while(m_topicStatus) 
    {
		if (topic_check(&mount_status_sub, 0) == 0) {
			topic_copy(TOPIC_ID(mount_status), &mount_status);

            m_mountPitch = mount_status.pitch;
            mCloudPitch=(m_mountPitch/100.0)/57.3;
            // HGLOG_INFO("[algo_serrve] mount pitch={}", mount_status.pitch);
            // printf("----pitch----%d  %f\n",m_mountPitch,mCloudPitch*57.3);
		}
		if (topic_check(&swarm_track_sub, 0) == 0) {
            uint8_t swarm_state_last=msg_swarm.swarm_state;
			topic_copy(TOPIC_ID(swarm_track), &msg_swarm);
            printf("swarm_state:%d\n",msg_swarm.swarm_state);
            printf("fun_id:%d\n",msg_swarm.fun_id);
            printf("is candidate:%d,%d,%d\n",msg_swarm.is_candidate[0],msg_swarm.is_candidate[1],msg_swarm.is_candidate[2]);
            printf("mTrackVehicleId:%d,%d,%d\n",msg_swarm.mav_target[0],msg_swarm.mav_target[1],msg_swarm.mav_target[2]);
            printf("mMavVehicleRela:%d,%d,%d\n",msg_swarm.mav_target[0+3],msg_swarm.mav_target[1+3],msg_swarm.mav_target[2+3]);
            // printf("mTrackYaw:%f\n",mTrackYaw);
            printf("mTrackCloudPitch:%f\n",mTrackCloudPitch);
            // printf("mav0(%f,%f,%f,%f,%f)  mav1(%f,%f,%f,%f,%f)   mav2(%f,%f,%f,%f,%f)",msg_swarm.pos[0*5+0],msg_swarm.pos[0*5+1],msg_swarm.pos[0*5+2],msg_swarm.pos[0*5+3],msg_swarm.pos[0*5+4]
            // ,msg_swarm.pos[1*5+0],msg_swarm.pos[1*5+1],msg_swarm.pos[1*5+2],msg_swarm.pos[1*5+3],msg_swarm.pos[1*5+4]
            // ,msg_swarm.pos[2*5+0],msg_swarm.pos[2*5+1],msg_swarm.pos[2*5+2],msg_swarm.pos[2*5+3],msg_swarm.pos[2*5+4]);
            // printf("vehicle0(%f,%f,%f,%f),   vehicle1(%f,%f,%f,%f)",msg_swarm.vehicle0[0],msg_swarm.vehicle0[1],msg_swarm.vehicle0[2],msg_swarm.vehicle0[3],msg_swarm.vehicle1[0],msg_swarm.vehicle1[1],msg_swarm.vehicle1[2],msg_swarm.vehicle1[3]);
            printf("track_dist(%f,%f,%f),track_height(%f,%f,%f)\n",msg_swarm.track_dist[0],msg_swarm.track_dist[1],msg_swarm.track_dist[2],msg_swarm.track_dist[3],msg_swarm.track_dist[4],msg_swarm.track_dist[5]);
            printf("mpSD->mMavIdInited %d,id %d\n",mpSD->mMavIdInited,mpSD->mMavId);
            printf("swarm_state_last %d\n",swarm_state_last);
            // do something
            double ts=mpSD->GetTimeMs();
            // printf("ts %f\n",ts);
            // mpSD->mSTD=msg_swarm;
            mpSD->SetSwarmTrackStatus(msg_swarm);
            // mIsCandidate[0]=msg_swarm.is_candidate[0];mIsCandidate[1]=msg_swarm.is_candidate[1];mIsCandidate[2]=msg_swarm.is_candidate[2];
            if(mpSD->mMavIdInited&&msg_swarm.swarm_state==START_SWARM_TRACK){
                mpSD->mTrackSend.time_stamp=ts;
                mpSD->mTrackSend.fun_id=msg_swarm.fun_id;
                if(swarm_state_last!=START_SWARM_TRACK){
                    mpSD->mTrackSend.state=START_SWARM_TRACK;
                    mpSD->mTrackSend.state_status=TRACK_INIT;
                }
                int mav_pad_id=-1;
                for(int i=0;i<MAV_NUM;i++){
                    if(msg_swarm.is_candidate[i]==1){
                        // mpSD->mMavPadId=i;
                        mav_pad_id=i;
                        break;
                    }
                }
                if(mav_pad_id != mpSD->mMavPadId){
                    mpSD->mMavPadId=mav_pad_id;
                }
                mTrackDist=msg_swarm.track_dist[mpSD->mMavId]/100.0;
                mTrackHeight=msg_swarm.track_dist[MAV_NUM+mpSD->mMavId]/100.0;
                if(mTrackStatus!=EXCHANGE_TARGET&&mpSD->mMavPadId==mpSD->mMavId){
                    mTrackVehicleId=min(1,max(0,msg_swarm.mav_target[mpSD->mMavId]-4));
                }
                mMavVehicleRela=msg_swarm.mav_target[mpSD->mMavId+MAV_NUM];
                //cal mTrackYaw and mTrackCloudPitch
                mTrackYaw=(mMavVehicleRela+4)*CV_2PI/8.0;
                if(mTrackYaw>=CV_2PI){
                    mTrackYaw -= CV_2PI;
                }
               mTrackYaw=Wrap_pi(mTrackYaw);
               printf("mTrackYaw1:%f\n",mTrackYaw);
                mTrackCloudPitch=atan(mTrackHeight/mTrackDist);
            }
            else if(msg_swarm.swarm_state<START_SWARM_TRACK){
                mpSD->mTrackSend.state=eStateSwarm(msg_swarm.swarm_state);
                mpSD->mTrackSend.state_status=TRACK_INIT;
            }

            mpSD->SetVehiclePos(Vec6d(msg_swarm.vehicle0[0]/100.0,-msg_swarm.vehicle0[1]/100.0,-msg_swarm.vehicle0[2]/100.0,-msg_swarm.vehicle0[3],ts,1.0),
                                Vec6d(msg_swarm.vehicle1[0]/100.0,-msg_swarm.vehicle1[1]/100.0,-msg_swarm.vehicle1[2]/100.0,-msg_swarm.vehicle1[3],ts,1.0));
            for(int i=0;i<3;i++){
                mpSD->SetMavPos(Vec6d(msg_swarm.pos[i*5+0]/100.0,-msg_swarm.pos[i*5+1]/100.0,-msg_swarm.pos[i*5+2]/100.0,-msg_swarm.pos[i*5+3],ts,1.0),i);
            }
            printf("------------------------------------------------\n");
            {
                std::lock_guard<std::mutex> slock(mpSD->mMutexAllMavPos);
                for(int i=0;i<3;i++){
                    printf("mav%d(%f,%f,%f,%f,%f)\n",i,mpSD->mAllMavPos[i][0],mpSD->mAllMavPos[i][1],mpSD->mAllMavPos[i][2],mpSD->mAllMavPos[i][3],mpSD->mAllMavPos[i][4]);
                }
            }
            {
                std::lock_guard<std::mutex> slock(mpSD->mMutexVehPos);
                printf("vehicle0(%f,%f,%f,%f),   vehicle1(%f,%f,%f,%f)",mpSD->mVehicle[0][0],mpSD->mVehicle[0][1],mpSD->mVehicle[0][2],mpSD->mVehicle[0][3],mpSD->mVehicle[1][0],mpSD->mVehicle[1][1],mpSD->mVehicle[1][2],mpSD->mVehicle[1][3]);
            }
            printf("------------------------------------------------\n");
		}
		if (topic_check(&host_parmas_sub, 0) == 0) {
			topic_copy(TOPIC_ID(host_parmas), &msg_host);

            // do something
            mpSD->mHostParamsInited=1;
            mpSD->mHostParams=msg_host;
            // mTrackHeight=msg_host.track_height;
		}
        if (topic_check(&drone_id_sub, 0) == 0) {
            drone_id_t msg;
			topic_copy(TOPIC_ID(drone_id), &msg);
            printf("mav id %d\n",msg.id);//exit(0);
            if(msg.id>=1&&msg.id<=MAV_NUM){
                mpSD->mMavId = msg.id-1;
                mpSD->mMavIdInited=1;
            }
            else{
                // HGLOG_INFO("[algo_server] mav_id={},id error", msg.id);
            }
		}
        if (topic_check(&drone_attitude_sub, 0) == 0) {
            drone_attitude_t msg;
			topic_copy(TOPIC_ID(drone_attitude), &msg);
            // printf("rec h:%f, yaw:%f\n",msg.ground_distance,msg.yaw);
            mpSD->SetMavHeight(msg.ground_distance);
            mpSD->SetMavYaw(msg.yaw);
		}
        if (topic_check(&drone_pos_acc_sub, 0) == 0) {
            drone_pos_acc_t msg;
			topic_copy(TOPIC_ID(drone_pos_acc), &msg);
            mpSD->SetMavPos0(Point3f(msg.local_pos[0],msg.local_pos[1],msg.local_pos[2]));
		}
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

unsigned char im_swarm[IMAGE_WIDTH*IMAGE_HEIGHT*3/2]={0};
std::mutex mutex_rect;
int track_rect_x,track_rect_y,track_rect_w,track_rect_h;
void AlgorithmServer::TrackThread()
{
    int cnt=0;
    int ret = 0;
    HGMEDIA_BUFFER mb = NULL;
    int marker_num;
    hgprotocol_swarm_track_t swarm_track_data;
    float mav_height=1.0;
    int track_cnt=0;
    while(m_trackStatus)
    {
        cnt++;
        // printf("cnt %d \n",cnt);
        mb = MPI_SYS_GetMediaBuffer(HGID_VPSS, 1, -1);
        if (!mb) {
            HGLOG_ERROR("[algo_server] MPI_SYS_GetMediaBuffer get null buffer!");
            break;
        }

        // printf("Get Venc packet[%d]:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
        //     "timestamp:%lld\n",
        //     0, MPI_MB_GetPtr(mb), /*MPI_MB_GetFD(mb)*/0,
        //     MPI_MB_GetSize(mb), MPI_MB_GetModeID(mb),
        //     MPI_MB_GetChannelID(mb), MPI_MB_GetTimestamp(mb));

        void *ptr = MPI_MB_GetPtr(mb);
        memcpy(im_swarm,ptr,IMAGE_WIDTH*IMAGE_HEIGHT*3/2);//IMAGE_WIDTH*IMAGE_HEIGHT
        MPI_MB_ReleaseBuffer(mb);
        Mat im_nv12(IMAGE_HEIGHT+IMAGE_HEIGHT/2,IMAGE_WIDTH,CV_8UC1,im_swarm);
        // cv::cvtColor(im_nv12, mImBGR, cv::COLOR_YUV2BGR);
        cv::cvtColor(im_nv12, mImBGR, cv::COLOR_YUV2BGR_NV12);
        #if 0
        if(cnt>=200&&cnt<=2800&&cnt%30==0){
            int tmpFd = open("/sdcard/rgb888.yuv", O_RDWR | O_APPEND | O_CREAT, 0777);
            if (0 > tmpFd)
            {
                printf("open failed(%s)\n", strerror(errno));
                exit(1);
            }
            // write(tmpFd, (char *)v4l2_pdev->vd_info->frame_buffer, 640*480);
            // write(tmpFd, (char *)params.dst_virt, 640*480);
            // write(tmpFd, (unsigned char *)&(im_swarm[0]), IMAGE_HEIGHT*IMAGE_WIDTH*3);
            Mat im_rgb;
            cv::cvtColor(mImBGR, im_rgb, cv::COLOR_BGR2RGB);
            write(tmpFd, (unsigned char *)&(im_rgb.data[0]), IMAGE_HEIGHT*IMAGE_WIDTH*3);
            close(tmpFd);
        }
        #endif

        #if 0
            mTrackDist=0;
            mTrackCloudPitch=75.0/57.3;
            mpSD->mMavId=0;
            mpSD->mMavPadId=2;
            mTrackHeight=1.5;
            mTrackVehicleId=0;
            double ts=mpSD->GetTimeMs();
            LockTrackCloudPitch();
            mpSD->SetVehiclePos(Vec6d(0,0,0,0,ts,1.0),Vec6d(0,0,0,0,ts,1.0));
            if(0==GotoNavPoint()){
                break;
            }
            else{
                continue;
            }
        #endif

        #if 0
        mpDDigital->DetectDigital(mImBGR,mMarkers);
        // mTrackRect=boundingRect(mMarkers.at(0).points);
        printf("detect num %d\n",mMarkers.size());
        if(mMarkers.size()>0)
            cout<<mTrackRect<<endl;
        #endif

        #if 0
        if(cnt==200)
            CtlCloudPitchByPos(-30.0/57.03);
        // else if(cnt==600)
        //     CtlCloudPitchByPos(-50.0/57.03);
        // else if(cnt==1000)
        //     CtlCloudPitchByPos(50.0/57.03);

        #endif

        ret=mpSD->GetSwarmTrackStatus(swarm_track_data,0);
        // uint8_t& s_state=swarm_track_data.swarm_state;
        uint8_t& s_state=mpSD->mTrackSend.state;
        printf("s_state %d\n",s_state);
        // s_state=SWARM_TRACKING;//test
        #if TEST_SINGLE_MAV_TRACK
        s_state=SWARM_TRACKING;
        mTrackCloudPitch=75.0/57.3;
        mpSD->mMavId=0;
        mpSD->mMavPadId=2;
        mTrackHeight=2.5;
        #endif
        #if TEST_GOTO_NAV_POINT
        // s_state=GOTO_START_POINT;
        mTrackDist=0;
        mTrackCloudPitch=75.0/57.3;
        mpSD->mMavId=0;
        mpSD->mMavPadId=2;
        mTrackHeight=1.5;
        mpSD->SetVehiclePos(Vec6d(0,0,0,0,0,1.0),Vec6d(0,0,0,0,0,1.0));
        if(s_state==START_SWARM_TRACK){
            if(0==GotoNavPoint()){
                break;
            }
            else{
                continue;
            }
        }
        #endif
        // 如果是初始状态，就去到起飞点
        if(s_state==START_SWARM_TRACK)
        {
            std::lock_guard<std::mutex> slock(mpSD->mMutexSTD);
            s_state=GOTO_START_POINT;
            mTimeStart=mpSD->GetTimeMs();
            mTimeStartDelay=float(mpSD->mMavId)*5*1000;
        }
        // 如果是运行过程中输入去到起飞点状态，就去到起飞点
        if(s_state==GOTO_START_POINT){
            LockTrackCloudPitch();
            if(0==GotoNavPoint()){
                std::lock_guard<std::mutex> slock(mpSD->mMutexSTD);
                s_state=SWARM_TRACKING;
            }
            else{
                continue;
            }
        }  
        
        //detect
        if(0==ret && s_state==SWARM_TRACKING && TrackStatusCheck()==0)
        {
            printf("mTrackStatus %d\n",mTrackStatus);
            #if !TEST_TRACK_USE_CLOUD
            LockTrackCloudPitch();
            #endif
            mMarkers.clear();
            if(mTrackStatus!=EXCHANGE_TARGET)
            {
                mpDDigital->DetectDigital(mImBGR,mMarkers);
            }
            marker_num=mMarkers.size();
            if(marker_num>0)
            {
                if(mpSD->mMavId==mpSD->mMavPadId && mMarkers.at(0).id==1&&mTrackStatus!=EXCHANGE_TARGET)
                {
                    mTrackStatus=EXCHANGE_TARGET;
                    mTrackVehicleId=1-mTrackVehicleId;
                    /*如果有检测到标记物，并且当前无人机的 ID 与无人机板载 ID 相等，同时第一个检测到的标记物的 ID 为 1，
                    且当前跟踪状态不是 EXCHANGE_TARGET，则执行以下操作：
                    将跟踪状态 mTrackStatus 设置为 EXCHANGE_TARGET，表示要切换跟踪目标。
                    切换 mTrackVehicleId 的值，可能是用来表示跟踪的目标车辆的标识。*/
                }
                // printf("mMavId:%d, mMavPadId:%d, mTrackVehicleId:%d\n",mpSD->mMavId,mpSD->mMavPadId,mTrackVehicleId);
            }
            if(mTrackStatus==EXCHANGE_TARGET)
            {
                //pad mav go to target vehicle
                if(0==GotoTargetVehicle())
                {
                    mTrackStatus=TRACK_INIT;
                }
                else
                {
                    usleep(1000*30);
                    continue;
                }
            }
            // track
            if(marker_num>0)
            {
                if(mTrackStatus!=TRACK_RUNNING)
                {
                    mTimeLastTrack=mpSD->GetTimeMs();
                    mLastSpeed=0;
                }
                mTrackStatus=TRACK_RUNNING;
                mTrackRect=boundingRect(mMarkers.at(0).points);
                mpTrack->extinit(mTrackRect.x, mTrackRect.y, mTrackRect.width, mTrackRect.height, im_swarm, IMAGE_WIDTH, IMAGE_HEIGHT);
                // 根据检测到的第一个标记物的位置和大小，初始化跟踪矩形，并使用该矩形初始化跟踪器。
                mRectIsNew=1;
                track_cnt=0;
                // 将跟踪矩形的信息设置为新的，跟踪计数器归零。
            }
            else
            {
                track_cnt++;
                if(mTrackStatus==TRACK_RUNNING)
                {
                    int x=0, y=0, w=0, h=0;
                    mpTrack->extupdate(im_swarm, IMAGE_WIDTH, IMAGE_HEIGHT, x, y, w, h);
                    // 更新跟踪目标，输出跟踪框坐标位置
                    printf("x:%d, y:%d, w:%d,h:%d\n",x,y,w,h);
                    if(-1==mpSD->GetMavHeight(mav_height))
                    {
                        mRectIsNew=0;
                        mTrackStatus=TRACK_LOST;
                    }
                    double t_size=CAMERA_FOCAL_LEN_X*MARKER_SIZE/mav_height;
                    double max0=max(w,h);
                    if(x>7&&y>7&&x+w<IMAGE_WIDTH-7&&y+h<IMAGE_HEIGHT-7&&max0<t_size*2.5&&max0>t_size/1.5)
                    {
                        mRectIsNew=1;
                        mTrackRect.x=x;
                        mTrackRect.y=y;
                        mTrackRect.width=w;
                        mTrackRect.height=h;
                    }
                    else
                    {
                        mRectIsNew=0;
                        mTrackStatus=TRACK_LOST;
                    }
                }
            }
        }
        //send data to control mav
        if(mRectIsNew==1)
        {
            mRectIsNew=0; // 已处理新的跟踪矩形信息
            if(track_cnt<30)
            {
                CtlMavToTrack();
            }
            //draw rect in test mode,should define TRACK_DEBUG
            {
                std::lock_guard<std::mutex> slock(mutex_rect);
                track_rect_x=mTrackRect.x;
                track_rect_y=mTrackRect.y;
                track_rect_w=mTrackRect.width;
                track_rect_h=mTrackRect.height;
            }
            //send vehicle pos to app
            SendTrackResultToApp();
        }
    }
}

void AlgorithmServer::StateManager()
{
    // switch(mpSD->mSTD.swarm_state){
    //     case IDLE:
    //         break;
    //     case INITING:
    //         break;
    //     case INIT_DONE:
    //         break;
    //     case TAKEOFF:
    //         break;
    //     case TAKEOFF_DONE:
    //         break;
    //     case START_SWARM_TRACK:
        
    //         break;
    //     case GOTO_START_POINT:
    //         break;
    //     case START_DETECT_DIGITAL:
    //         break;
    //     case SWARM_TRACKING:
    //         break;
    //     default:
    //         break;
    // }
}

//pitch is rad,down is +
void AlgorithmServer::CtlCloudPitchByPos(float pitch)
{
    // float pitch0=pitch;
    float c0=30.0/57.3;
    if(pitch>c0){
        pitch=c0;
    }
    else if(pitch<-c0){
        pitch=-c0;
    }
    // MAV_CMD_PTZ
    mount_control_t msg = {0};
    msg.input_a = int((pitch/c0)*3000.0); 
    // cout<<"CtlCloudPitchByPos msg.input_a "<<msg.input_a<<endl;
    msg.component = 1;  // 0 speed control or 1 pos control
    topic_publish(TOPIC_ID(mount_control), &msg);  
}

void AlgorithmServer::CtlMavToTrack()
{
    float height = 1.0;
    float mav_yaw = 0.0;
    if (-1 == mpSD->GetMavHeight(height) || -1 == mpSD->GetMavYaw(mav_yaw)) // 获取无人机的高度和偏航角
    {
        // HGLOG_INFO("Get height and yaw failed,cancel current frame CtlMavToTrack!");
        printf("Get height and yaw failed,cancel current frame CtlMavToTrack!");
        return;
    }
    if (height > MAX_MAV_HEIGHT) // 限制无人机的最大高度
    {
        HGLOG_INFO("Height {} is bigger than MAX_MAV_HEIGHT 4.0!", height);
        height = MAX_MAV_HEIGHT;
    }
    printf("CtlMavToTrack h:%f, yaw:%f\n", height, mav_yaw);                               // 打印无人机的高度和偏航角
    Point2f pt(mTrackRect.x + mTrackRect.width / 2, mTrackRect.y + mTrackRect.height / 2); // 获取跟踪矩形的中心点
    Point2f im_cen(IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2);                                     // 获取图像的中心点
    float dx = atan((pt.x - im_cen.x) / CAMERA_FOCAL_LEN_X);                               // 计算跟踪矩形中心点与图像中心点的水平偏移角
    float dy = -atan((pt.y - im_cen.y) / CAMERA_FOCAL_LEN_Y);                              // 计算跟踪矩形中心点与图像中心点的垂直偏移角
#if TEST_TRACK_USE_CLOUD
    printf("compensate pitch is %f\n", dy);
    CtlCloudPitchByPos(dy); // 调用 CtlCloudPitchByPos(dy) 进行云台俯仰角的调整。
#endif
    // change coordinate to mav coor
    float x, y, z, yaw;
    if (mCloudPitch != 0 && mCloudPitch + dy != 0) // 计算在无人机坐标系下的 x、y 和 z 位移
    {
        x = height / tan(mCloudPitch) - height / tan(mCloudPitch + dy);
    }
    else
    {
        x = 0;
    }
    x = max(-TRACK_MAX_MOVE, min(TRACK_MAX_MOVE, x));
    y = height * tan(dx);
    y = max(-TRACK_MAX_MOVE, min(TRACK_MAX_MOVE, y)); // 限制 x 和 y 的最大值
    if (abs(mTrackHeight - height) > 0.4)
    {
        z = -(mTrackHeight - height); // 如果无人机的高度与目标高度差值超过 0.4，将 z 位移设置为高度差，否则置为 0，表示无人机不需要调整高度。
    }
    else
    {
        z = 0;
    }
    float x0 = cos(mTrackYaw - mav_yaw);
    float y0 = sin(mTrackYaw - mav_yaw);
    float t_angle;
    t_angle = atan2(y0, x0);       // 计算旋转角度 t_angle，用于调整无人机的偏航角。这里的计算是将无人机的当前偏航角与目标偏航角的差值。
    if (abs(t_angle) > 10.0 / 57.3) // 如果旋转角度的绝对值大于 5 度（以弧度为单位），将角度差作为目标偏航角，否则将偏航角置为 0,表示无人机不需要调整偏航角。
    {
        yaw = t_angle;
    }
    else
    {
        yaw = 0;
    }
#if TEST_SINGLE_MAV_TRACK
    yaw = 0;
#endif
    float len = min(TRACK_MAX_MOVE, sqrt(x * x + y * y));     // 计算位移的模长，即无人机应该飞行的距离，将其限制在 TRACK_MAX_MOVE 范围内。
    float speed_t = max(30.0f, len / TRACK_MAX_MOVE * 60.0f); // 计算无人机的速度，将其限制在 30-60 之间。
    uint16_t speed = mLastSpeed;
    if (speed_t - mLastSpeed > 20)
    {
        speed = mLastSpeed + 20;
    }
    else if (speed_t - mLastSpeed < -20)
    {
        speed = mLastSpeed - 20;
    }
    else
    {
        speed = speed_t;
    }
    printf("x:%f, y:%f, z:%f, yaw:%f,speed %d\n", x, y, z, yaw, speed); // 打印输出位移和角度信息，以及计算出的速度
    SendResultControlMav(20, x, y, z, yaw, speed);                      // 调用函数，将计算出的位移和角度信息发送给无人机。
    mLastSpeed = speed;                                                 // 记录最后的速度值。这个速度值会在下一次调用函数时使用。
}

void AlgorithmServer::LockTrackCloudPitch()
{
    if (abs(mCloudPitch - mTrackCloudPitch) > 0.5 / 57.3)
    {
        CtlCloudPitchByPos(mTrackCloudPitch - mCloudPitch); // 传入参数为目标俯仰角与当前俯仰角之差，以实现调整云台俯仰角的操作。
    }
}

//
int AlgorithmServer::TrackStatusCheck()
{
    #if TEST_TRACK_USE_CLOUD||TEST_SINGLE_MAV_TRACK
    return 0;
    #endif
    
    if(mpSD->mMavIdInited&&mpSD->mHostParamsInited){
        return 0;
    }
    else{
        return -1;
    }
}

int AlgorithmServer::MavChangeTrackTarget()
{
    mTrackStatus = EXCHANGE_TARGET;
    return 0; // TODO:code not done
}

int AlgorithmServer::GotoTargetVehicle()
{
    double ts = mpSD->GetTimeMs();
    Vec6d vehicle(0, 0, 0, 0, 0, 0);
    if (0 != mpSD->GetVehiclePos(vehicle, mTrackVehicleId)) // 获取目标车辆的位置信息
    {
        return -1;
    }
    printf("GotoTargetVehicle %f    %f,  mTrackVehicleId:%d,vehicle(%f,%f)\n", vehicle[4], ts, mTrackVehicleId, vehicle[0], vehicle[1]);
    if (vehicle[0] != 0 || vehicle[1] != 0) // ms  abs(vehicle[4]-ts)<1500&&
    {
        Point2f vp(vehicle[0], vehicle[1]); // 获取目标车辆的位置信息
        mMavTargetPos[0] = vp.x + mTrackDist * cos(mTrackYaw - CV_PI);
        mMavTargetPos[1] = vp.y + mTrackDist * sin(mTrackYaw - CV_PI);
        mMavTargetPos[2] = -(mTrackHeight + 0.0);
        mMavTargetPos[3] = mTrackYaw;
        mMavTargetPos[4] = ts;
        mMavTargetPos[5] = 1.0;
        printf("ab x:%.1f,y:%.1f,z:%.1f,yaw:%.1f\n", mMavTargetPos[0], mMavTargetPos[1], mMavTargetPos[2], mMavTargetPos[3]);
        SendResultControlMav(18, mMavTargetPos[0], mMavTargetPos[1], mMavTargetPos[2], mMavTargetPos[3], 80);

        Point3f pos;
        int ret = mpSD->GetMavPos(pos);
        if (0 == ret)
        {
            double dist = cv::norm(Point2d(mMavTargetPos[0] - mpSD->mMavPos[0], mMavTargetPos[1] - mpSD->mMavPos[1]));
            // 计算无人机与目标车辆的距离
            if (dist < 0.2)
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        // HGLOG_INFO("GetMavHeight failed!");
        return -1;
    }
}

//TODO:need test
//ret: 0 GotoNavPoint done,-1 on the way
int AlgorithmServer::GotoNavPoint() // 暂时没用
{
    double ts_c = mpSD->GetTimeMs();
    printf("ts_c:%f s,mTimeStart:%f s,mTimeStartDelay:%f s,  %f\n", ts_c / 1000, mTimeStart / 1000, mTimeStartDelay / 1000, (mTimeStart + mTimeStartDelay) / 1000);
    if (ts_c < mTimeStart + mTimeStartDelay)
    {
        return -1;
    }
    // go to target point
    int ret = GotoTargetVehicle();
    return ret;
}

void AlgorithmServer::SendTrackResultToApp()
{
    //TODO:
    // uint8_t sbuf[512] = {0};
    // hgprotocol_message_t msg;
    // memset(sbuf, 0, sizeof(sbuf));
    // topic_copy(TOPIC_ID(track_send), &(mpSD->mTrackSend));
    // hgprotocol_track_send_encode(0, 0, &msg, &(mpSD->mTrackSend));
    // int slen = hgprotocol_msg_to_send_buffer(sbuf, &msg);
    // sendMsgToAppT(m_tfd, sbuf, slen);
}

float AlgorithmServer::Wrap_pi(float bearing) // bearing表示待处理的角度值, 将一个角度限制在 -π 到 π 之间，即将角度值限制在半个周期的范围内。
{
        /* value is inf or NaN */
        if (!isfinite(bearing)) 
        {
            return bearing;
        }
        int c = 0;
        while (bearing >= CV_PI) 
        {
            bearing -= CV_2PI;

            if (c++ > 3) 
            {
                return NAN;
            }
        }
        c = 0;
        while (bearing < -CV_PI) 
        {
            bearing += CV_2PI;
            if (c++ > 3) 
            {
                return NAN;
            }
        }
        return bearing;
}

void AlgorithmServer::SendResultControlMav(uint8_t cmd, float x, float y, float z, float yaw, int16_t speed)
{
    flightctrl_cmd_t msg = {0};
    msg.cmd = cmd;
    msg.yaw = yaw * 57.3 * 10.0; // 这里的计算是将弧度转换为以 0.1 度为单位的角度。
    msg.pos_data[0] = (int32_t)(x * 100.0f);
    msg.pos_data[1] = (int32_t)(y * 100.0f);
    msg.pos_data[2] = (int32_t)(z * 100.0f); // 这里乘以 100.0f 是将单位从米转换为厘米。
    msg.reserve[0] = speed/256;
    msg.reserve[1] = speed %256;
    // int16_t *ptr = (int16_t *)msg.reserve;
    // ptr[0] = speed;
    topic_publish(TOPIC_ID(drone_ctrl_algo), &msg); // 发布飞控控制消息。
}

ShareData::ShareData()
{
    mpTimeAlign=new TimeAlign();
    mHostParamsInited=1;//not use
    mMavIdInited=0;
    mMavHeight=Point2d(1.0,0);
    mMavYaw=Point2d(0.0,0);
    mMavPadId=-1;
    memset(&mTrackSend,0,sizeof(mTrackSend));
}

ShareData::~ShareData()
{
  if(mpTimeAlign)
  {
    delete mpTimeAlign;
    mpTimeAlign=nullptr;
  }
}

void ShareData::SetSwarmTrackStatus(hgprotocol_swarm_track_t data)
{
    std::lock_guard<std::mutex> slock(mMutexSTD);
    mSTD=data;
}

//time_stamp:us
int ShareData::GetSwarmTrackStatus(hgprotocol_swarm_track_t& data,unsigned long long time_stamp)
{
    std::lock_guard<std::mutex> slock(mMutexSTD);
    data=mSTD;
    int ret=-1;
    if((time_stamp>=mSTD.time_stamp && time_stamp-mSTD.time_stamp<1000*1000) || 
        (time_stamp<mSTD.time_stamp && (mSTD.time_stamp-time_stamp)<1000*1000) 
        || time_stamp==0)
    {
        ret=0;//success
    }
    return ret;
}

//ret 0 success,-1 failed
int ShareData::GetMavHeight(float& h)
{
    #if ONLY_TEST_TRACKING
    return 0;
    #endif
    std::lock_guard<std::mutex> slock(mMutexMavHeight);
    double ts=GetTimeMs();
    if(abs(mMavHeight.y-ts)<500)//ms
    {
        h=mMavHeight.x;
        return 0;
    }
    else{
        HGLOG_INFO("GetMavHeight failed,time diff {}!",abs(mMavHeight.y-ts)/100.0);
        return -1;
    }
}

void ShareData::SetMavHeight(float h)
{
    std::lock_guard<std::mutex> slock(mMutexMavHeight);
    double ts=GetTimeMs();
    mMavHeight.x=h;
    mMavHeight.y=ts;
    // printf("SetMavHeight ts %f\n",ts);
}

//ret 0 success,-1 failed
int ShareData::GetMavYaw(float& yaw)
{
    std::lock_guard<std::mutex> slock(mMutexMavYaw);
    double ts=GetTimeMs();
    if(abs(mMavYaw.y-ts)<500)//ms
    {
        yaw=mMavYaw.x;
        return 0;
    }
    else{
        HGLOG_INFO("GetMavYaw failed,time diff {}!",abs(mMavYaw.y-ts)/100.0);
        return -1;
    }
}

void ShareData::SetMavYaw(float yaw)
{
    std::lock_guard<std::mutex> slock(mMutexMavYaw);
    double ts=GetTimeMs();
    mMavYaw.x=yaw;
    mMavYaw.y=ts;
}

//ret 0 success,-1 failed
int ShareData::GetMavPos(Point3f& pos)
{
    std::lock_guard<std::mutex> slock(mMutexMavPos);
    double ts=GetTimeMs();
    //TODO:test receive value is right
    if(abs(mMavPos[3]-ts)<500)//ms
    {
        pos.x=mMavPos[0];
        pos.y=mMavPos[1];
        pos.z=mMavPos[2];
        return 0;
    }
    else{
        HGLOG_INFO("GetMavPos failed!");
        return -1;
    }
}

void ShareData::SetMavPos0(Point3f pos)
{
    std::lock_guard<std::mutex> slock(mMutexMavPos);
    double ts=GetTimeMs();
    mMavPos[0]=pos.x;
    mMavPos[1]=pos.y;
    mMavPos[2]=pos.z;
    mMavPos[3]=ts;
    // printf("SetMavPos0 x:%.2f,y:%.2f,z:%.2f\n",mMavPos[0],mMavPos[1],mMavPos[2]);
}

double ShareData::GetTimeMs()
{
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec*1000.0+t.tv_usec/1000.0;
}

int ShareData::GetVehiclePos(Vec6d& vehicle,int id)
{
   std::lock_guard<std::mutex> slock(mMutexVehPos);
    double ts=GetTimeMs();
    if(id>=0&&id<2){
        vehicle=mVehicle[id];
    }
    else{
        vehicle=Vec6d(0,0,0,0,0,0);
        return -1;
    }
    // printf("vehicle[4]-ts %f\n",vehicle[4]-ts);
    if(abs(vehicle[4]-ts)<1500&&(vehicle[0]!=0||vehicle[1]!=0||vehicle[2]!=0))//ms
    {
        vehicle[5]=1.0;//quality
        return 0;
    }
    else{
        // HGLOG_INFO("GetVehiclePos Vehicle0 failed!");
        vehicle[5]=0.0;//quality
        printf("Get vehicle %d pos failed!",id+VEHICLE_START_NUM);
        return -1;
    }
}

void ShareData::SetVehiclePos(Vec6d vehicle0,Vec6d vehicle1)
{
    std::lock_guard<std::mutex> slock(mMutexVehPos);
    // printf("get vehicle ts %f\n",vehicle0[4]);
    mVehicle[0]=vehicle0;
    mVehicle[1]=vehicle1;
}

void ShareData::SetMavPos(Vec6d mav_pos,int mav_id)
{
    std::lock_guard<std::mutex> slock(mMutexAllMavPos);
    if(mav_id>=0&&mav_id<MAV_NUM){
        mAllMavPos[mav_id]=mav_pos;
    }
}

void ShareData::GetMavPos(Vec6d& mav_pos,int mav_id)
{
    std::lock_guard<std::mutex> slock(mMutexAllMavPos);
    double ts=GetTimeMs();
    if(mav_id>=0&&mav_id<MAV_NUM){
        mav_pos=mAllMavPos[mav_id];
        if(abs(mav_pos[4]-ts)<500&&(mav_pos[0]!=0||mav_pos[1]!=0||mav_pos[2]!=0))//ms
        {
            mav_pos[5]=1.0;//quality
        }
        else{
            // HGLOG_INFO("GetVehiclePos Vehicle0 failed!");
            mav_pos[5]=0.0;//quality
        }
    }
    else{
         mav_pos[5]=0.0;//quality
    }
}

