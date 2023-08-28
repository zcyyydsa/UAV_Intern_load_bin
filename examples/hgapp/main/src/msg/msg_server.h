#ifndef __MSG_SERVER_H_
#define __MSG_SERVER_H_

#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <iostream>
#include "hg_flightctrl.h"
#include "hg_protocol.h"

class MsgServer
{
public:
  MsgServer();
  ~MsgServer();

  void start();
  void stop();

public:
   int sendAckToAppTest(int fd, uint16_t cmd);
   void msgPrintTest(hgprotocol_lcommand_t *msg);
   void msgPrintHeratTest(hgprotocol_drone_stats_t* lmsg);
   void msgPrintAckTest(hgprotocol_lack_t* ack_msg);

public:
   int sendMsgToAppT(int fd, uint8_t *send_data, int len);
   int readMsgFromAppT(int fd, uint8_t *recv_data, int len);
   void netDataCmdParse(hgprotocol_lcommand_t *cmd_msg);
   void handlerMsg(hgprotocol_message_t * msg);
  // int sendMsgToAppU(); // by udp

private:
  void TopicSubThread();
  void TcpRecvThread();
  void UdpRecvThread();

private:
  std::thread m_topicSubThread;
  std::thread m_tcpRecvThread;
  std::thread m_udpRecvThread;
  std::condition_variable m_tcpWait;
  std::mutex m_mutex,m_smutex;
  bool m_topicStatus; 
  bool m_udpRecvStatus;
  bool m_tcpRecvStatus;
  bool m_tcpThreadRun;
private:
  int m_ufd, m_tfd;
  int m_port;
  uint16_t m_cid;
  std::string m_ip;
};

#endif