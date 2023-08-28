/**
 * @file socket.h
 * @author gwl
 * @brief 
 * @version 1.0
 * @date 2023/6/14
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */
#include "stream.h"
#include "encoder.h"
#include "decoder.h"

inline uint32_t hash(const char *str) {
  size_t hash = static_cast<size_t>(0xc70f6907UL);  // seed

  while (*str) hash = (hash * 131) + (*str++);

  return ((hash & 0xFFFF) | 0xAA550000);
}

class socket {
 public:
  socket(int domain, int protocol);

  //  Prevent making copies of the socket by accident. 
  socket(const socket &) = delete;
  void operator=(const socket &) = delete;
   ~socket();
  int setsockopt(int level, int option, const void *optval, size_t optvallen);
  int getsockopt(int level, int option, void *optval, size_t *optvallen);
  int bind(const char *addr);
  int connect(const char *addr);
  void shutdown(int how);
  int send(const void *buf, size_t len, int flags);

  template <typename T>
  int send(uint32_t topic, const T &t, int flags = 0) {
    encoder_.clear();
    encoder_ | topic | const_cast<T&>(t);
    auto str = encoder_.getData();

    return send(str.data(), str.size(), flags);
  }

  template <typename T>
  int send(const std::string &topic, const T &t, int flags = 0) {
     send(hash(topic.c_str()), t, flags);
     return 0;
  }

  void connection();
  int close();

  void startAsync();
  void recvOnce();

  template <typename T>
  void reg(const char *topic, std::function<void(T &)> cb) {
   handler_map_.emplace(hash(topic), new Data<T>(cb));
  }

  template <typename T>
  void reg(uint32_t topic, std::function<void(T &)> cb) {
    handler_map_.emplace(topic, new Data<T>(cb));
  }

 private:
  class IData {
   public:
    virtual void callback(Decoder &) = 0;
  };

  template <typename T>
  class Data : public IData {
   public:
    Data(std::function<void(T &)> cb) : handler_(cb){};

    virtual void callback(Decoder &decoder) override {
      decoder | data_;
      handler_(data_);
    }

   private:
    T data_;
    std::function<void(T &)> handler_;
  };

  std::unordered_map<uint32_t, IData *> handler_map_;

 private:
  int sock_, how_;
  bool is_running_;
  Encoder encoder_;
  Decoder decoder_;
  std::thread async_thread_;
  std::mutex send_mutex_;
};

inline void term() { nn_term(); }

