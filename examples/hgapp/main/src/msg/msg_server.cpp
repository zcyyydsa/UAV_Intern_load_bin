#include "msg_server.h"

#include "log.h"
#include "uorb_topic.h"

#define IP_FOUND  "IP_FOUND"
#define NET_PORT  8888

TOPIC_DEFINE(mount_control, mount_control_t);
TOPIC_DEFINE(drone_ctrl, drone_ctrl_t);
TOPIC_DEFINE(swarm_track, hgprotocol_swarm_track_t);
TOPIC_DEFINE(host_parmas, hgprotocol_host_parmas_t);
TOPIC_DEFINE(media_ctrl, media_ctrl_t);

MsgServer::MsgServer() : m_topicStatus(false), m_udpRecvStatus(false),
m_tcpRecvStatus(false),m_tcpThreadRun(true), m_ufd(0),m_tfd(0),m_port(0),m_cid(1) {
}

MsgServer::~MsgServer() {
  
}

void MsgServer::start() {
  m_topicStatus = true;
  m_topicSubThread = std::thread(std::bind(&MsgServer::TopicSubThread, this)); 
  m_udpRecvStatus = true;
  m_udpRecvThread = std::thread(std::bind(&MsgServer::UdpRecvThread, this));
    
  m_tcpRecvThread = std::thread(std::bind(&MsgServer::TcpRecvThread, this));
}

void MsgServer::stop() {
  m_topicStatus = false;
  m_topicSubThread.join();
  

  m_tcpRecvStatus = false;
  m_tcpThreadRun = false;
  m_tcpRecvThread.join();

  m_udpRecvStatus = false;
  m_udpRecvThread.join();
}

int MsgServer::sendAckToAppTest(int fd, uint16_t cmd) {
  hgprotocol_lack_t drone_ack = {0};
  hgprotocol_message_t  drone_ack_msg;
  uint8_t sbuf[512] = {0};
  drone_ack.id = 1;
  drone_ack.result = 0;
  drone_ack.cmd = cmd;
  drone_ack.type = 1;
  hgprotocol_lack_encode(0, 0, &drone_ack_msg, &drone_ack);
  int dlen = hgprotocol_msg_to_send_buffer(sbuf, &drone_ack_msg);
  int slen = send(fd, sbuf, dlen, MSG_NOSIGNAL);
  return 0;
}

int MsgServer::sendMsgToAppT(int fd, uint8_t *send_data, int len) {
  int slen = 0;
  if(m_tcpRecvStatus) {
    //HGLOG_INFO("SEND HEART DATA");
    std::lock_guard<std::mutex> slock(m_smutex);
    slen = send(fd, send_data, len, MSG_NOSIGNAL);
  }else {
    // HGLOG_INFO("Net is error when send data");
  }
  return slen;
}

int MsgServer::readMsgFromAppT(int fd, uint8_t *recv_data, int len) {
  int rlen = recv(fd, recv_data, len, 0);
  return rlen;
}

void MsgServer::msgPrintTest(hgprotocol_lcommand_t *msg) {
  HGLOG_INFO("start_id:{}", msg->start_id);
  HGLOG_INFO("end_id:{}", msg->end_id);
  HGLOG_INFO("cmd:{}", msg->cmd);
  HGLOG_INFO("ack:{}", msg->ack);
}

void MsgServer::msgPrintHeratTest(hgprotocol_drone_stats_t* lmsg) {
  printf("utc:%ld\n", lmsg->utc);
  printf("lat:%d\n", lmsg->lat);
  printf("lon:%d\n", lmsg->lon);
  printf("x:%02f\n", lmsg->x);
  printf("y:%02f\n", lmsg->y);
  printf("z:%02f\n", lmsg->z);
  printf("aux_token:%d\n", lmsg->aux_token);
  printf("time_token:%d\n", lmsg->time_token);
  printf("yaw:%d\n", lmsg->yaw);
  printf("id:%d\n", lmsg->id);
  printf("temperature_imu:%d\n", lmsg->temperature_imu);
  printf("version:%d\n", lmsg->version);
  printf("sensor_status:%d\n", lmsg->sensor_status);
  printf("temperature_battery:%d\n", lmsg->temperature_battery);
  printf("battery_cycles:%d\n", lmsg->battery_cycles);
  printf("battery:%d\n", lmsg->battery);
  printf("rtk_stats:%d\n", lmsg->rtk_stats);             
                         
  for(int i=0;i<16;i++) {
    printf("md5:%d\n", lmsg->md5[i]);  
  }  
   for(int j=0;j<8;j++) {
    printf("rk_version:%d\n", lmsg->rk_version[j]);  
  }       
  printf("type:%d\n",lmsg->type);           
  printf("formation_status:%d\n", lmsg->formation_status);
  printf("land_reason:%d\n", lmsg->land_reason);                 
  printf("imu_status0:%d\n", lmsg->imu_status[0]);
  printf("imu_status1:%d\n", lmsg->imu_status[1]);
  printf("mag_status0:%d\n", lmsg->mag_status[0]);
  printf("mag_status1:%d\n", lmsg->mag_status[1]);   
}

void MsgServer::msgPrintAckTest(hgprotocol_lack_t* ack_msg) {
  printf("ack_token:%d\n", ack_msg->token);
  printf("ack_id:%d\n", ack_msg->id);
  printf("ack_cmd:%d\n", ack_msg->cmd);
  printf("ack_result:%d\n", ack_msg->result);
  printf("ack_type:%d\n", ack_msg->type);
}

void MsgServer::netDataCmdParse(hgprotocol_lcommand_t *cmd_msg) {
  switch (cmd_msg->cmd) {
    case MAV_CMD_NET_DISCONNECT: {
      m_tcpRecvStatus = false;
    }break;
    case MAV_CMD_SETID: { // set id
      hgdf::flightctrl_set_id(cmd_msg->param[0]);
    }break;
    case MAV_CMD_PTZ: {
       mount_control_t msg = {0};
       msg.input_a = cmd_msg->param[0]; // 3000 or -3000
       msg.component = cmd_msg->param[1];  // 0 or 1
       HGLOG_INFO("input_a:{}", msg.input_a);
       HGLOG_INFO("component:{}", msg.component);
       
       topic_publish(TOPIC_ID(mount_control), &msg);  
    }break;
    case MAV_CMD_TAKEPHOTO: {
       media_ctrl_t msg = {0};
       msg.cmd = MAV_CMD_TAKEPHOTO;
       topic_publish(TOPIC_ID(media_ctrl), &msg); 
    }break;
    case MAV_CMD_RECORD: {
       media_ctrl_t msg = {0};
       msg.cmd = MAV_CMD_RECORD;
       msg.param[0] = cmd_msg->param[0];
       topic_publish(TOPIC_ID(media_ctrl), &msg); 
    }break;
    default:
     break;
  }
}

void MsgServer::handlerMsg(hgprotocol_message_t * msg) {
  switch(msg->msgid) {
    case HGPROTOCOL_MSG_ID_LCOMMAND: {
        hgprotocol_lcommand_t cmd_msg;
        drone_ctrl_t drone_ctrl_msg = {0};
        hgprotocol_lcommand_decode(msg, &cmd_msg);
        if(m_cid < cmd_msg.start_id || m_cid > cmd_msg.end_id) {
            HGLOG_INFO("Drone id is not in the range of command id,currend id is {}", m_cid);
            return;
        }
        netDataCmdParse(&cmd_msg);
        msgPrintTest(&cmd_msg); //for testing
        if ((MAV_CMD_DRONE_BASE<=cmd_msg.cmd && cmd_msg.cmd<MAV_CMD_MEDIA_BASE) ||
            (MAV_CMD_PERIPHERALS_BASE<=cmd_msg.cmd && cmd_msg.cmd<MAV_CMD_MSG_BASE)) {
            drone_ctrl_msg.utc = cmd_msg.utc;    
            drone_ctrl_msg.cmd = cmd_msg.cmd;
            memcpy(drone_ctrl_msg.param, cmd_msg.param, 20);
            topic_publish(TOPIC_ID(drone_ctrl), &drone_ctrl_msg);
        } else if (MAV_CMD_MEDIA_BASE<=cmd_msg.cmd && cmd_msg.cmd<MAV_CMD_PERIPHERALS_BASE) {
            media_ctrl_t media_ctrl_msg = {0};
            media_ctrl_msg.cmd = cmd_msg.cmd;
            memcpy(media_ctrl_msg.param, cmd_msg.param, 16);
            topic_publish(TOPIC_ID(media_ctrl), &media_ctrl_msg);
        }
        //sendAckToAppTest(m_tfd, cmd_msg.cmd);  //for testing
    }break;
    case HGPROTOCOL_MSG_ID_SWARM_TRACK: {
        hgprotocol_swarm_track_t track_msg;
        hgprotocol_swarm_track_decode(msg, &track_msg);
        topic_publish(TOPIC_ID(swarm_track), &track_msg);
    }break;
    case HGPROTOCOL_MSG_ID_HOST_PARMAS: {
        hgprotocol_host_parmas_t parmas_msg;
        hgprotocol_host_parmas_decode(msg, &parmas_msg);
        topic_publish(TOPIC_ID(host_parmas), &parmas_msg);
    }break;
    default:
        break;
    }
}

void MsgServer::TopicSubThread()
{
    uint64_t mount_status_sub = 0;
    uint64_t drone_stats_sub = 0;
    uint64_t drone_ack_sub = 0;
    uint64_t track_send_sub = 0;

    mount_status_t mount_status;
    //drone_stats_t drone_stats;
    // drone_ack_t drone_ack;
    hgprotocol_drone_stats_t drone_stats;
    hgprotocol_lack_t drone_ack, mount_ack;
    hgprotocol_track_send_t track_ack;
    hgprotocol_message_t mount_status_msg, drone_status_msg, drone_ack_msg, msg;
    uint8_t sbuf[512] = {0};

    if (topic_subscribe_auto(TOPIC_ID(mount_status), &mount_status_sub, (char*)"ptz", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(mount_status)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_stats), &drone_stats_sub, (char*)"dss", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(drone_stats)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_ack), &drone_ack_sub, (char*)"das", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(drone_ack)");
    }
    if (topic_subscribe_auto(TOPIC_ID(track_send), &track_send_sub, (char*)"track_send", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(track_send)");
    }

    while(m_topicStatus) {
        if (topic_check(&mount_status_sub, 0) == 0) {  //rk3399 send ptz ack to app 
            memset(sbuf, 0, sizeof(sbuf));
            topic_copy(TOPIC_ID(mount_status), &mount_status);
            mount_ack.cmd = MAV_CMD_PTZ;
            mount_ack.param[0] = (int32_t)mount_status.pitch;
            hgprotocol_lack_encode(0, 0, &mount_status_msg, &mount_ack);
            int slen = hgprotocol_msg_to_send_buffer(sbuf, &mount_status_msg);
            sendMsgToAppT(m_tfd, sbuf, slen);
        }

        if (topic_check(&drone_stats_sub, 0) == 0) { //drone heart from flight control
            memset(sbuf, 0, sizeof(sbuf));
            topic_copy(TOPIC_ID(drone_stats), &drone_stats);
            m_cid = drone_stats.id;
            hgprotocol_drone_stats_encode(0, 0, &drone_status_msg, &drone_stats);
            //msgPrintHeratTest(&drone_stats);  //heart print test
            int slen = hgprotocol_msg_to_send_buffer(sbuf, &drone_status_msg);
            sendMsgToAppT(m_tfd, sbuf, slen);
        }

        if (topic_check(&drone_ack_sub, 0) == 0) {  //drone ack from flight control
            memset(sbuf, 0, sizeof(sbuf));
            topic_copy(TOPIC_ID(drone_ack), &drone_ack);
            hgprotocol_lack_encode(0, 0, &drone_ack_msg, &drone_ack);
           // msgPrintAckTest(&drone_ack);
            int slen = hgprotocol_msg_to_send_buffer(sbuf, &drone_ack_msg);
            sendMsgToAppT(m_tfd, sbuf, slen);
        }

        if (topic_check(&track_send_sub, 0) == 0) {  //msg from algo_server
            memset(sbuf, 0, sizeof(sbuf));
            topic_copy(TOPIC_ID(track_send), &track_ack);
            hgprotocol_track_send_encode(0, 0, &msg, &track_ack);
            int slen = hgprotocol_msg_to_send_buffer(sbuf, &msg);
            sendMsgToAppT(m_tfd, sbuf, slen);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// void MsgServer::TestThread()
// {
//     while(m_testStatus) {
//         sleep(5);
//         media_ctrl_t media_ctrl_msg = {0};
//         // media_ctrl_msg.cmd = MAV_CMD_TAKEPHOTO;
//         media_ctrl_msg.cmd = MAV_CMD_FILELIST;
//         media_ctrl_msg.param[0] = 1;
//         topic_publish(TOPIC_ID(media_ctrl), &media_ctrl_msg);

//         sleep(2);

//         media_ctrl_msg.cmd = MAV_CMD_FILELIST;
//         media_ctrl_msg.param[0] = 0;
//         topic_publish(TOPIC_ID(media_ctrl), &media_ctrl_msg);

//         // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//         m_testStatus = false;
//     }
// }

void MsgServer::TcpRecvThread()
{
  while(m_tcpThreadRun) {
    m_tfd = 0;
    m_port = 0;
    HGLOG_INFO("TCP thread is waiting");
    std::unique_lock<std::mutex> lock(m_mutex);
    m_tcpWait.wait(lock, [this]{return m_tcpRecvStatus;});
    int ret, rlen;
    fd_set readfd;
    struct sockaddr_in addr;
    hgprotocol_message_t rmsg;
    hgprotocol_status_t rstat;
    uint8_t rbuf[512] = {0};
  
    m_tfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_tfd < 0) {
      HGLOG_ERROR("TCP socket create failed");
      m_tcpRecvStatus = false;
      continue;
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    addr.sin_port = htons(m_port);

    ret = connect(m_tfd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
      HGLOG_ERROR("TCP socket connect failed, please try it again");
      m_tcpRecvStatus = false;
      continue;
    }
    HGLOG_INFO("TCP thread is running");
	  while(m_tcpRecvStatus) {
      FD_ZERO(&readfd);
      FD_SET(m_tfd, &readfd);
      struct timeval tv = {5, 0};

      ret = select(m_tfd + 1, &readfd, NULL, NULL, &tv);
      if(ret > 0) { 
        if(FD_ISSET(m_tfd, &readfd)) {
          rlen = readMsgFromAppT(m_tfd, rbuf, sizeof(rbuf));
          for(int i=0;i<rlen;i++) {
            if(HGPROTOCOL_FRAMING_OK == hgprotocol_parse_char(0, rbuf[i], &rmsg, &rstat)) {
              handlerMsg(&rmsg);
            }
          }
        }
      }else if(0 == ret) { //timeout
        continue;
      }else {
        HGLOG_INFO("TCP network is error");
        m_tcpRecvStatus = false;
      }
    }
    close(m_tfd);
    m_tcpRecvStatus = false; 
  }
 HGLOG_INFO("TCP thread is quit");
}

void MsgServer::UdpRecvThread()
{
    int ret;
    fd_set readfd;
    char buf[1024];
    struct sockaddr_in bro_addr, srv_addr;

    m_ufd = socket(AF_INET, SOCK_DGRAM, 0);
    if(m_ufd < 0) {
      HGLOG_ERROR("UDP create socket failed");
      return;
    }
    
    int addr_len = sizeof(struct sockaddr_in);
    memset((void *)&(bro_addr), 0, addr_len);
    bro_addr.sin_family = AF_INET;
    bro_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bro_addr.sin_port = htons(NET_PORT);

    ret = bind(m_ufd, (struct sockaddr *)&bro_addr, addr_len);
    if(ret < 0) {
     HGLOG_ERROR("UDP bind failed");
      return;
    }
    HGLOG_INFO("UDP thread is running");

	while(m_udpRecvStatus) {
      FD_ZERO(&readfd);
      FD_SET(m_ufd, &readfd);
      ret = select(m_ufd + 1, &readfd, NULL, NULL, NULL);      
      if(ret > 0) {
        if(FD_ISSET(m_ufd, &readfd)) {
          recvfrom(m_ufd, buf, 1024, 0, (struct sockaddr *)&(srv_addr), (socklen_t *)&(addr_len));
          printf("udp recv:%s\n", buf);
          if(strstr(buf, IP_FOUND)) {
            m_ip = inet_ntoa(srv_addr.sin_addr);
            m_port = htons(srv_addr.sin_port);
            if((!m_ip.empty()) && (m_port == NET_PORT) && (false == m_tcpRecvStatus)) { 
              m_tcpRecvStatus = true;
              m_tcpWait.notify_one();
            }
          }else {
            HGLOG_WARN("found not ip or port");
          }
        }
      }else if(0 == ret) {
         continue;  //timeout
      }else {
        //error
      }
    }
}



/*

drone_ctrl_t msg = {0};
msg.cmd = GO_LEFT_CMD;
msg.param[0] = 10;
topic_publish(TOPIC_ID(drone_ctrl), &msg);

mount_control_t msg = {0};
msg.input_a = 3000; // 3000 or -3000
msg.component = 0;  // 0 or 1
topic_publish(TOPIC_ID(mount_control), &msg);
*/