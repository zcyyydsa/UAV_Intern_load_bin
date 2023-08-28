/**
 * @file decoder.h
 * @author gwl
 * @brief 
 * @version 1.0
 * @date 2023/6/14
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */
#ifndef DECODER_H
#define DECODER_H

#include "stream.h"

#define META_DECODER(T, args) \
  inline Decoder& operator|(Decoder& s, T& t) { return s | args; }
  
class Decoder {
 public:
  Decoder();

  void setData(const char* buf, size_t size);
  void setData(const std::string& str);
  void clear();
  bool hasData();

  template <typename T>
  T get() {
   T t;
   *this | t;
   return t;
  }
 
  template <class T, size_t N>
  Decoder& operator|(T (&v)[N]) {
    if (get<uint32_t>() != N) throw std::runtime_error("data vector len error");
    for (size_t i = 0; i < N; ++i) *this | v[i];
    return *this;
  }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, int>::type = 0>
  Decoder& operator|(T& v) {
    stream_.read((char*)&v, sizeof(T));
    if (!stream_) throw std::runtime_error("arithmetic data failed");
    return *this;
  }

  template <typename T, typename std::enable_if<is_container<T>::value, int>::type = 0>
  Decoder& operator|(T& v) {
    v.clear();
    auto len = get<uint32_t>();
    for (uint32_t i = 0; i < len; ++i) {
      typename T::value_type value;
      *this | value;
      v.insert(v.end(), value);
    }
    return *this;
  }

  Decoder& operator|(std::string& v);

  template <class T1, class T2>
  Decoder& operator|(std::pair<T1, T2>& v) {
    *this | v.first | v.second;
    return *this;
  }

 private:
  Stream stream_;
};

#endif  //!DECODER_H


