/**
 * @file stream.h
 * @author gwl
 * @brief 
 * @version 1.0
 * @date 2023/6/14
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */
#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>
#include <cstdint>
#include <string>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>
#include <assert.h>
#include "nn.h"
#include "pair.h"
#include "functional"


template <typename T>
struct is_container : public std::false_type {};
template <typename T>
struct is_container<std::vector<T>> : public std::true_type {};
template <typename T>
struct is_container<std::deque<T>> : public std::true_type {};
template <typename T>
struct is_container<std::list<T>> : public std::true_type {};
template <typename T>
struct is_container<std::set<T>> : public std::true_type {};
template <typename T>
struct is_container<std::multiset<T>> : public std::true_type {};
template <typename T1, typename T2>
struct is_container<std::map<T1, T2>> : public std::true_type {};
template <typename T1, typename T2>
struct is_container<std::multimap<T1, T2>> : public std::true_type {};



class Stream {
 public:
  Stream();
  operator bool() const;
  void clear();
  bool empty(); 
  void write(const char* buffer, int size);
  void read(char* buffer, int size);
  const std::vector<char>& getData();

 private:
  bool ok_;
  int index_;
  std::vector<char> data_;
};


#define nn_slow(x) __builtin_expect((x), 0)




#endif  //!STREAM_H
