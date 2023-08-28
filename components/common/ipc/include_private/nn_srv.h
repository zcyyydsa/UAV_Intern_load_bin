/**
 * @file nn_srv.h
 * @author gwl
 * @brief 
 * @version 1.0
 * @date 2023/6/14
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#ifndef NN_SRV_H
#define NN_SRV_H

#include <chrono>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include "socket.h"

#define META_SERIALIZE(T, args) \
  META_ENCODER(T, args)         \
  META_DECODER(T, args)

struct TestDataDemo {
  uint8_t data1;
  uint8_t data2;  
  uint8_t data3;
};
META_SERIALIZE(TestDataDemo, t.data1 | t.data2 | t.data3)

int nn_server_init(socket *sck);


#endif // !NN_SRV_H
