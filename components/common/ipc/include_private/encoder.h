/**
 * @file encoder.h
 * @author gwl
 * @brief 
 * @version 1.0
 * @date 2023/6/14
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#ifndef ENCODER_H
#define ENCODER_H

#include "stream.h"


#define META_ENCODER(T, args) \
  inline Encoder& operator|(Encoder& s, const T& t) { return s | args; }

class Encoder {
 public:
  Encoder();
  void print();
  const std::vector<char>& getData();
  void clear();

  template <class T, size_t N>
  Encoder& operator|(const T (&v)[N]) {
    *this | static_cast<uint32_t>(N);
    for (size_t i = 0; i < N; ++i) *this | v[i];
    return *this;
  }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, int>::type = 0>
  Encoder& operator|(T v) {
    stream_.write((const char*)&v, sizeof(T));
    return *this;
  }

  template <typename T, typename std::enable_if<is_container<T>::value, int>::type = 0>
  Encoder& operator|(const T& v) {
   *this | static_cast<uint32_t>(v.size());
    for (typename T::const_iterator it = v.begin(); it != v.end(); ++it) *this | *it;
    return *this;
  }

  Encoder& operator|(const std::string& v);
  
  template <class T1, class T2>
  Encoder& operator|(const std::pair<T1, T2>& v) {
    *this | v.first | v.second;
    return *this;
  }

 private:
  Stream stream_;
};

#endif // !ENCODER_H
