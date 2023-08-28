#include "device_server.h"

#include "log.h"
#include "hg_protocol_types.h"
#include <sys/select.h>
#include <sys/time.h>

#include "hg_utils.h"

TOPIC_DEFINE(mount_status, mount_status_t);
TOPIC_DEFINE(drone_pos_acc, drone_pos_acc_t);
TOPIC_DEFINE(drone_stats, hgprotocol_drone_stats_t);
TOPIC_DEFINE(drone_ack, hgprotocol_lack_t);
TOPIC_DEFINE(drone_id, drone_id_t)
TOPIC_DEFINE(drone_attitude, drone_attitude_t)


DeviceServer::DeviceServer()
    : m_topicStatus(false)
    , m_tofStatus(false)
    , m_rcStatus(false)
    , m_ptzStatus(false)
    , m_ptzFd(-1)
    , m_servoFd(-1)
    , m_rcFd(-1)
    , m_tofFd(-1)
    , m_droneId(-1)
{
}

DeviceServer::~DeviceServer()
{
    hgdf::flightctrl_destroy();
}

uint8_t buf[256];
static void* get(void* param, int bytes)
{
    return buf;
}
static void release(void* param, void* packet)
{
    // do nothing
}
static void onrecv(void* param, const void* packet, uint16_t bytes, int msgid)
{
    //printf("msg id = %d\n", msgid);
    switch(msgid)
    {
        case DRONE_ID_STATS:
        {
            drone_id_t msg_id = {0};
            hgprotocol_drone_stats_t *msg = (hgprotocol_drone_stats_t *)packet;
            msg_id.id = msg->id;
           // HGLOG_INFO("Drone msg_id.id: {}", msg_id.id);
            topic_publish(TOPIC_ID(drone_stats), msg);
            topic_publish(TOPIC_ID(drone_id), &msg_id);
            break;
        }
        case DRONE_ID_ACK:
        {
            drone_ack_t *msg = (drone_ack_t *)packet;
            hgprotocol_lack_t msg_ack = {0};
            msg_ack.cmd = msg->cmd;
            msg_ack.result = msg->result;
            msg_ack.id = msg->id;
           // HGLOG_INFO("Drone Ack Id: {}", msg->id);
           // HGLOG_INFO("Drone Ack Cmd: {}", msg->cmd);
           // HGLOG_INFO("Drone Ack Result: {}", msg->result);

            topic_publish(TOPIC_ID(drone_ack), &msg_ack);
            break;
        }
        case DRONE_ID_ACC:
        {
            drone_pos_acc_t *msg = (drone_pos_acc_t *)packet;
            topic_publish(TOPIC_ID(drone_pos_acc), msg);
            break;
        }
        case DRONE_ID_ATTITUDE:
        {
            drone_attitude_t *msg = (drone_attitude_t *)packet;
            topic_publish(TOPIC_ID(drone_attitude), msg);
            break;
        }
        default:
            break;
    }
}

int DeviceServer::init()
{
    int ret = HG_ERR;
    hdr = {get, release, onrecv};
    ret = hgdf::flightctrl_create();
    if (ret != HG_OK) {
        HGLOG_ERROR("[device_server] flight controller create failed.");
    } else {
        hgdf::flightctrl_register(&hdr);
    } 
    return ret;
}

void DeviceServer::start()
{
    m_topicStatus = true;
    m_topicSubThread = std::thread(std::bind(&DeviceServer::TopicSubThread, this));

    m_tofStatus = true;
    m_tofRecvThread = std::thread(std::bind(&DeviceServer::TofRecvThread, this));

    m_rcStatus = true;
    m_rcRecvThread = std::thread(std::bind(&DeviceServer::RcRecvThread, this));

    m_ptzStatus = true;
    m_ptzRecvThread = std::thread(std::bind(&DeviceServer::PtzRecvThread, this));
}

void DeviceServer::stop()
{
    m_topicStatus = false;
    m_topicSubThread.join();
    m_tofStatus = false;
    m_tofRecvThread.join();
    m_rcStatus = false;
    m_rcRecvThread.join();  
    m_ptzStatus = false;
    m_ptzRecvThread.join(); 
}

void DeviceServer::ptzSetPitch(int16_t pitch)
{
    uint8_t sbuf[128];
    int size = ptz_pack_pitch_to_buffer(pitch, sbuf);
    write(m_ptzFd, sbuf, size);
}

void DeviceServer::ptzSetAcc(float ax, float ay, float az)
{
    uint8_t sbuf[128];
    int size = ptz_pack_acc_to_buffer(ax, ay, az, sbuf);
    write(m_ptzFd, sbuf, size);
}

void DeviceServer::TopicSubThread()
{
	uint64_t mount_control_sub = 0;
    uint64_t drone_pos_acc_sub = 0;
    uint64_t drone_ctrl_sub = 0;
    uint64_t drone_id_sub = 0;
    uint64_t drone_ctrl_algo_sub = 0;

    uint8_t new_ptz_ctrl = 0;
    int32_t target_angle=0;
    mount_control_t mct;
	
	if (topic_subscribe_auto(TOPIC_ID(mount_control), &mount_control_sub, (char*)"mount_control", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(mount_control)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_pos_acc), &drone_pos_acc_sub, (char*)"acc", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(vehicle_attitude)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_ctrl), &drone_ctrl_sub, (char*)"dc", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(drone_ctrl)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_id), &drone_id_sub, (char*)"drone_id", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(drone_id)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_ctrl_algo), &drone_ctrl_algo_sub, (char*)"drone_ctrl_algo", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(drone_ctrl_algo)");
    }

	while(m_topicStatus) 
    {
        if (topic_check(&drone_id_sub, 0) == 0) {
            drone_id_t msg;
			topic_copy(TOPIC_ID(drone_id), &msg);
            m_droneId = msg.id;
            #ifdef D04_EDU
                std::call_once(wflag, [this](){wifiModeSwitch(WIFI_AP);});
            #endif 
		}
		if (topic_check(&mount_control_sub, 0) == 0) {
			topic_copy(TOPIC_ID(mount_control), &mct);
     
            new_ptz_ctrl = 1;
		}
        if (topic_check(&drone_pos_acc_sub, 0) == 0) {
            drone_pos_acc_t msg;
			topic_copy(TOPIC_ID(drone_pos_acc), &msg);

            ptzSetAcc(msg.acc[0], msg.acc[1], msg.acc[2]);
		}
        if (topic_check(&drone_ctrl_sub, 0) == 0) {
            drone_ctrl_t msg;
			topic_copy(TOPIC_ID(drone_ctrl), &msg);

            droneCtrl(&msg);
		}
        if (topic_check(&drone_ctrl_algo_sub, 0) == 0) {
            flightctrl_cmd_t fmsg;
			topic_copy(TOPIC_ID(drone_ctrl_algo), &fmsg);

            hgdf::flightctrl_command(&fmsg);
		}

        // contorl angle
        if(mct.component == 1) {
            if(new_ptz_ctrl){
                target_angle=m_ptzCurDegree+mct.input_a;
            }
            mct.input_a = std::min(3000,std::max(-3000,(m_ptzCurDegree - target_angle)*3));//控制灵敏度偏低
            if(abs(mct.input_a)<20)
                mct.component=0;
        }
       // printf("%d----%d  %d\n",mct.component,m_ptzCurDegree,mct.input_a);
        // ptz pitch speed control
        if (new_ptz_ctrl || mct.component == 1) {
            ptzSetPitch(mct.input_a);
            // printf("set----%d  %d\n");
        } else {
            ptzSetPitch(0);
        }

        new_ptz_ctrl = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

static uint64_t get_time_usec()
{
	static struct timeval _time_stamp;
	gettimeofday(&_time_stamp, NULL);
	return _time_stamp.tv_sec*1000000 + _time_stamp.tv_usec;
}

void DeviceServer::TofRecvThread()
{
    int ret = 0;
    fd_set fd_read;
    struct timeval timeout;
    uint8_t rbuf[10];
    tof_data_t tof_data = {0};

    m_tofFd = uart_open(TOF_PORT, TOF_BUNDRATE);
	if (m_tofFd == -1) {
		HGLOG_ERROR("[device_server] open tof_uart[/dev/ttysWK1] err");
	}

    while(m_tofStatus)
    {
        FD_ZERO(&fd_read);
        FD_SET(m_tofFd, &fd_read);
        timeout.tv_sec = 3;//0;
        timeout.tv_usec = 0;//200000; //200ms

        ret = select(m_tofFd+1, &fd_read, NULL, NULL, &timeout);
        if (ret < 0) {
            HGLOG_ERROR("[device_server] tof_uart[/dev/ttysWK1] select err...");
        } else if (ret == 0) {
            HGLOG_INFO("[device_server] tof_uart[/dev/ttysWK1] not connected...");           	
		} else {

			if ( FD_ISSET(m_tofFd, &fd_read) > 0 ) {
				int s_ret = read(m_tofFd, rbuf, 10);
	            //printf("[device_server]%d \n", s_ret);
				for (int i=0; i<s_ret; i++) {
                    int ret = tof_parse_char(rbuf[i], &tof_data);  
                    if(ret == 1) {
                        //HGLOG_INFO("[device_server] tof height={} stength={}", tof_data.height, tof_data.strength);
                        uint64_t timestamp = get_time_usec();
                        if (tof_data.strength > 100) { // tof height valid                           
                            hgdf::flightctrl_tof(tof_data.height, 1, timestamp); // cm
                        } else { // tof height invalid
                            hgdf::flightctrl_tof(tof_data.height, 0, timestamp);
                        }
                    }
				}                
			}
		}
        //std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }

    uart_close(m_tofFd);
}

void DeviceServer::RcRecvThread()
{
    int ret = 0;
    fd_set fd_read;
    struct timeval timeout;
    uint8_t rbuf[128];

    m_rcFd = uart_open(RC_PORT, RC_BUNDRATE);
	if (m_rcFd == -1) {
		HGLOG_ERROR("[device_server] open tof_uart[/dev/ttysWK3] err");
	}
    uart_set_opt(m_rcFd, RC_BUNDRATE, 8, 'N', 2);

    while(m_rcStatus)
    {
        FD_ZERO(&fd_read);
        FD_SET(m_rcFd, &fd_read);
        timeout.tv_sec = 3;//0;
        timeout.tv_usec = 0;//200000; //200ms

        ret = select(m_rcFd+1, &fd_read, NULL, NULL, &timeout);
        if (ret < 0) {
            HGLOG_ERROR("[device_server] rc_uart[/dev/ttysWK3] select err...");
        } else if (ret == 0) {
            HGLOG_INFO("[device_server] rc_uart[/dev/ttysWK3] not connected...");       	
		} else {

			if ( FD_ISSET(m_rcFd, &fd_read) > 0 ) {
				int s_ret = read(m_rcFd, rbuf, 128);
	            //printf("[device_server]%d \n", s_ret);
				for (int i=0; i<s_ret; i++) {
                    bool ret = ibus_decoder(rbuf[i]);  
                    if(ret == true) {
                        uint16_t rc_ch[8];
                        ibus_get_values(rc_ch, 8);
                        //HGLOG_INFO("[device_server] rc: {} {} {} {} {} {} {} {}",
                        //   rc_ch[0],rc_ch[1],rc_ch[2],rc_ch[3],rc_ch[4],rc_ch[5],rc_ch[6],rc_ch[7]); 
                        hgdf::flightctrl_rc(rc_ch);
                    }
				}                
			}
		}
    }

    uart_close(m_rcFd);
}

void DeviceServer::PtzRecvThread()
{
    int ret = 0;
    fd_set fd_read;
    struct timeval timeout;
    uint8_t rbuf[128];
    ptz_payload_rx_data_status_t ptz_data = {0};

    m_ptzFd = uart_open(PTZ_DEV, 460800);
	if (m_ptzFd == -1) {
		HGLOG_ERROR("[device_server] open ptz_uart[/dev/ttysWK0] err");
	}

    while(m_ptzStatus)
    {
        FD_ZERO(&fd_read);
        FD_SET(m_ptzFd, &fd_read);
        timeout.tv_sec = 3;//0;
        timeout.tv_usec = 0;//200000; //200ms

        ret = select(m_ptzFd+1, &fd_read, NULL, NULL, &timeout);
        if (ret < 0) {
            HGLOG_ERROR("[device_server] ptz_uart[/dev/ttysWK0] select err...");
        } else if (ret == 0) {
            HGLOG_INFO("[device_server] ptz_uart[/dev/ttysWK0] not connected...");           	
		} else {

			if ( FD_ISSET(m_ptzFd, &fd_read) > 0 ) {
				int s_ret = read(m_ptzFd, rbuf, 128);
	            //printf("%d \n", s_ret);
				for (int i=0; i<s_ret; i++) {
                    int ret = ptz_parse_char(rbuf[i], &ptz_data);  
                    if(ret == 1) {
                       // HGLOG_INFO("[device_server] pitch={}", ptz_data.pitch);
                        mount_status_t ms;
                        ms.pitch = ptz_data.pitch;
                        m_ptzCurDegree = ptz_data.pitch;
                        topic_publish(TOPIC_ID(mount_status), &ms);
                    }
				}
			}
		}
    }   

    uart_close(m_ptzFd);
}

int DeviceServer::wifiModeSwitch(int32_t status) {
    std::string ssid = "D04_TEST";

    if(m_droneId >= 0) {
     ssid = "D04_" + std::to_string(m_droneId);
    }
    switch_wifi_mode(ssid.c_str(), status);
    return 0; 
}

void DeviceServer::sendAck2Msg(uint8_t cmd, int type, uint8_t result)
{
    hgprotocol_lack_t msg = {0};
    msg.id = m_droneId;
    msg.cmd = cmd;
    msg.result = result;
    msg.param[0] = type;
    topic_publish(TOPIC_ID(drone_ack), &msg);
}

void DeviceServer::droneCtrl(drone_ctrl_t *msg)
{
    switch(msg->cmd) {
    case MAV_CMD_TAKEOFF: { // takeoff
        hgdf::flightctrl_takeoff(msg->param[0], msg->param[1]); //param0:temperature param1:height
    }break;
    case MAV_CMD_LAND: {  // land
        hgdf::flightctrl_land();
    }break;
    case MAV_CMD_UP: {  // up
        hgdf::flightctrl_up(msg->param[0]);
    }break;
    case MAV_CMD_DOWN: {  // down
        hgdf::flightctrl_down(msg->param[0]);
    }break;
    case MAV_CMD_LEFT: { //left
        hgdf::flightctrl_left(msg->param[0]);
    }break;
    case MAV_CMD_RIGHT: {  // right
        hgdf::flightctrl_right(msg->param[0]);
    }break;
    case MAV_CMD_FORWARD: {  // forward
        hgdf::flightctrl_forward(msg->param[0]);
    }break;
    case MAV_CMD_BACK: {  // back
        hgdf::flightctrl_back(msg->param[0]);
    }break;
    case MAV_CMD_CW: {  // cw
        hgdf::flightctrl_cw(msg->param[0]);
    }break;
    case MAV_CMD_CCW: {  // ccw
        hgdf::flightctrl_ccw(msg->param[0]);
    }break;
    case MAV_CMD_GO: {  // go to fix point
        hgdf::flightctrl_go(msg->param[0], msg->param[1], msg->param[2], msg->param[3], msg->param[4]);
    } break;
    case MAV_CMD_STOP: {  // hover
        hgdf::flightctrl_stop();      
    }break;
    case MAV_CMD_LOCK: {  // lock
        hgdf::flightctrl_lock();        
    }break;
    case MAV_CMD_UNLOCK: {  // unlock
        hgdf::flightctrl_unlock();
    }break;
    case MAV_CMD_SERVO: {  // servo

    }break;
    case MAV_CMD_WIFI_MODE: {  // set wifi mode
       // wifiModeSwitch(msg->param[0]);
    } break;
    case MAV_CMD_TIMESYNC: {
        int ret = CMD_ERR;
        // timesync linux
        ret = sys_set_time(msg->utc);
        if (ret != CMD_OK) {
            HGLOG_ERROR("[device_server] time sync err.");
            sendAck2Msg(MAV_CMD_TIMESYNC, 0, CMD_ERR);
        } else {
            HGLOG_INFO("[device_server] time sync ok.");
            sendAck2Msg(MAV_CMD_TIMESYNC, 0, CMD_OK);
        }

        // timesync flight
        flightctrl_cmd_t fmsg = {0};
        fmsg.cmd = MAV_CMD_TIMESYNC;
        uint64_t *ptr = (uint64_t *)fmsg.reserve;
        *ptr = msg->utc;
        hgdf::flightctrl_command(&fmsg);
    } break;
    default:
        break;
    }
}