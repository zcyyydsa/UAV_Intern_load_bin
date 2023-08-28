/**
 * @file hg_steering_engine.h
 * @author gwl
 * @brief  hg_steering_engine
 * @version 1.0
 * @date 2023/7/17
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#ifndef HG_STEERING_ENGINE_H
#define HG_STEERING_ENGINE_H

#include <stdio.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <pthread.h>
#include "hg_uart.h"
#define STEERING_PORT     "/dev/ttysWK2"
#define STEERING_BUNDRATE 115200

typedef struct {
  uint8_t head = 0xa5;
  uint8_t addr = 0x3d;
  uint8_t cmd;
  uint8_t data[4] = {0};
  uint8_t crc[2] = {0}; //crc[0]:crc_H, crc[1]:crc_L
}StreeingEngineData;

typedef struct {
  int fd;
  bool isRunning = false;
  char rdata[3] = {0};
  StreeingEngineData seData;
}HgSteeringEngine;
 
int hg_steering_engine_init();
int hg_steering_engine_write(HgSteeringEngine *wStreeingData);
int hg_steering_engine_read(HgSteeringEngine *rStreeingData);
int hg_steering_engine_parse(HgSteeringEngine *parse);
int hg_steering_engint_print_test(StreeingEngineData *printTest);

//int hg_clkwise_rota_angle(HgSteeringEngine *rotaAngle);
int hg_clkwise_rota_angle(uint8_t dutyCycle);
int hg_steering_engine_freq(uint32_t freq);
int hg_steering_engine_uninit(HgSteeringEngine *steeringEngine);
uint16_t hg_streeing_engine_crc(StreeingEngineData *dataCrc, uint8_t length);

extern HgSteeringEngine hgSteeringEngine;
extern HgUart seUart;

template <typename T>
inline int hg_streeing_send(int cfd, T *uart_data, int len) {
  int ret = 0;
  uint8_t sbuf[len] = {0};
  printf("cfd:%d\n", cfd);
  memcpy(sbuf, uart_data, len);
 // ret = write(cfd, sbuf, len);
  ret = seUart.uart_send(cfd, sbuf, len); 
  printf("ret:%d len:%d\n", ret, len);
  for(int i=0;i<len;++i) {
    printf("sbuf:%02x\n", sbuf[i]);
  }
  printf("\n-----------------------------------\n");
  return ret;
  //return seUart.uart_send(cfd, sbuf, len);   
}

#endif // !HG_STEERING_ENGINE_H