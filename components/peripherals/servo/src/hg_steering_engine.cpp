/**
 * @file hg_steering_engine.cpp
 * @author gwl
 * @brief  hg_steering_engine
 * @version 1.0
 * @date 2023/7/17
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#include "hg_steering_engine.h"

HgUart seUart;
HgSteeringEngine hgSteeringEngine;
static void *steering_cmd_listen(HgSteeringEngine *steering);

static void *steering_cmd_listen(HgSteeringEngine *steering) {
  printf("steering engine data listening thread run\n");
  pthread_detach(pthread_self());
  int ret, rlen = 0;
  
  while (steering->isRunning) {
    if(hg_steering_engine_read(steering)) {
      hg_steering_engine_parse(steering);
    }
  }
   return NULL;
}

int hg_steering_engine_init() {
  hgSteeringEngine.fd = seUart.uart_open(STEERING_PORT, STEERING_BUNDRATE);
  sleep(2);
  hg_steering_engine_freq(0);
  while(1){
     sleep(3);
     hg_clkwise_rota_angle(0x32);
     sleep(3);
     hg_clkwise_rota_angle(0x14);
  }
  
  return 1;
}

int hg_steering_engine_write(HgSteeringEngine *wStreeingData) {
  return hg_streeing_send(wStreeingData->fd, &(wStreeingData->seData), sizeof(wStreeingData->seData));
}

int hg_steering_engine_read(HgSteeringEngine *rStreeingData) {
  int ret = 0;
  memset(rStreeingData->rdata, 0, sizeof(rStreeingData->rdata));
  ret = seUart.uart_recv(rStreeingData->fd, rStreeingData->rdata, sizeof(rStreeingData->rdata), 0);
//  printf("ret:%d rdata1:%02x rdata2:%02x\n", ret, rStreeingData->rdata[0], rStreeingData->rdata[1]);
 
  return ret;
}

int hg_steering_engine_parse(HgSteeringEngine *parse) {
   int len = sizeof(parse->rdata);
   for(int i=0;i<len;++i) {
     printf("steering read:%02x\n", parse->rdata[i]);
   }
   return 1;
}

int hg_steering_engint_print_test(StreeingEngineData *printTest) {
   printf("head:%02x\n", printTest->head);
   printf("addr:%02x\n", printTest->addr);
   printf("cmd:%02x\n", printTest->cmd);
   printf("data0:%02x\n", printTest->data[0]);
   printf("data1:%02x\n", printTest->data[1]);
   printf("data2:%02x\n", printTest->data[2]);
   printf("data3:%02x\n", printTest->data[3]);
   printf("crc0:%02x\n", printTest->crc[0]);
   printf("crc1:%02x\n", printTest->crc[1]);
   printf("-----------------------------------\n");

   return 1;
}
/*int hg_clkwise_rota_angle(HgSteeringEngine *rotaAngle) {
  
  return 1;
}*/

int hg_clkwise_rota_angle(uint8_t dutyCycle) {
  memset(&(hgSteeringEngine.seData), 0, sizeof(hgSteeringEngine.seData));
  hgSteeringEngine.seData.head = 0xa5;
  hgSteeringEngine.seData.addr = 0x3d;
  hgSteeringEngine.seData.cmd = 0x03;
  hgSteeringEngine.seData.data[0] = 0;
  hgSteeringEngine.seData.data[1] = 0;
  hgSteeringEngine.seData.data[2] = 0;
  hgSteeringEngine.seData.data[3] = dutyCycle;
  hg_streeing_engine_crc(&(hgSteeringEngine.seData), sizeof(hgSteeringEngine.seData));
  //hg_steering_engint_print_test(&(hgSteeringEngine.seData));
  hg_steering_engine_write(&hgSteeringEngine);

  return 1;
}

int hg_steering_engine_freq(uint32_t freq) {
  memset(&(hgSteeringEngine.seData), 0, sizeof(hgSteeringEngine.seData));
  hgSteeringEngine.seData.head = 0xa5;
  hgSteeringEngine.seData.addr = 0x3d;
  hgSteeringEngine.seData.cmd = 0x01;
  hgSteeringEngine.seData.data[0] = 0;
  hgSteeringEngine.seData.data[1] = 0;
  hgSteeringEngine.seData.data[2] = 0x01;
  hgSteeringEngine.seData.data[3] = 0x90;
  hg_streeing_engine_crc(&(hgSteeringEngine.seData), sizeof(hgSteeringEngine.seData));
 // hg_steering_engint_print_test(&(hgSteeringEngine.seData));
  hg_steering_engine_write(&hgSteeringEngine);

  return 1;
}

int hg_steering_engine_uninit(HgSteeringEngine *steeringEngine) {
  steeringEngine->isRunning = false;
  memset(&(steeringEngine->seData), 0, sizeof(steeringEngine->seData));
  memset(steeringEngine->rdata, 0, sizeof(steeringEngine->rdata));
  seUart.uart_close(steeringEngine->fd);
  return 1;
}

uint16_t hg_streeing_engine_crc(StreeingEngineData *dataCrc, uint8_t length) {
  uint8_t i, j;
  uint16_t t;
  uint16_t crc = 0xffff;
  uint8_t buf[7] = {0};
  uint8_t *data = buf;
  
  memcpy(buf, dataCrc, length - 2);
  
  for(j=length-2;j>0;j--) {
    crc = (crc ^ (((uint16_t) *data) << 8));
    for(i=8;i>0;i--) {
      t = crc << 1;
      if(crc & 0x8000) {
        t = t ^ 0x1021;
      }
      crc = t;
    }
    data++;
  }
   dataCrc->crc[0] = ((crc >> 8) & 0xff);
   dataCrc->crc[1] = (crc & 0xff);

   return crc;
}