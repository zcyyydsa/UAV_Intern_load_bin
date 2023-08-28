/**
 * @file hg_uart.h
 * @author gwl
 * @brief  hg_uart
 * @version 1.0
 * @date 2023/7/17
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#ifndef HG_UART_H
#define HG_UART_H

using namespace std;
class HgUart
{

public:
  HgUart();
  ~HgUart();

public:
  int uart_open(const char* dev, int speed);
  void uart_close(int fd);
  int uart_set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);
  int uart_send(int fd, const unsigned char* buf, int len);
  int uart_recv(int fd, char* buf, int len, int timeout_ms);
  
private:
  

private:
  
};

#endif // !HG_UART_H