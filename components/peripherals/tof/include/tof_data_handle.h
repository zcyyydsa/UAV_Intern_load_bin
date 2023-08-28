/**
 * @file tof_data_queue.h
 * @author gwl
 * @brief  tof_data_queue
 * @version 1.0
 * @date 2023/7/17
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */
#ifndef TOF_DATA_HANDLE_H
#define TOF_DATA_HANDLE_H
 
#include <queue>
#include <mutex>
#include <thread>
#include <iostream>
#include <list>
#include <condition_variable>


template<typename T>
class TofDataHandle {
public:
    TofDataHandle(int size) : m_maxSize(size) {}
    //TofDataQueue(){}
    
    void pushData(T& val) {
      std::unique_lock<std::mutex> lock(m_mutex);
      notEmpty.wait(lock, [this]{return !(tofQueue.size() == m_maxSize);});
      tofQueue.emplace_back(val);
      notEmpty.notify_one();
    }
 
    T getData() {
      std::unique_lock<std::mutex> lock(m_mutex);
      notEmpty.wait(lock, [this]{return !tofQueue.empty();});
      T val = tofQueue.front();
      tofQueue.pop_front(); 
      notEmpty.notify_one();
      return val;
    }
    
    bool isFull() {
      return tofQueue.size() == m_maxSize;
    }

    bool isEmpty() {
      std::lock_guard<std::mutex> lock(m_mutex);
      return tofQueue.empty();
    }

    int size() {
      std::lock_guard<std::mutex> lock(m_mutex);
      return tofQueue.size();
    }

private:
    std::list<T> tofQueue;
    int m_maxSize;
    std::mutex m_mutex;
    std::condition_variable notEmpty;
};
 
#endif //TOF_DATA_HANDLE_H
