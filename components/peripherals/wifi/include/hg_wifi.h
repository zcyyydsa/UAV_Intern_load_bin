/**
 * @file hg_wifi.h
 * @author gwl
 * @brief switching of wifi mode 
 * @version 1.0
 * @date 2023/7/13
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#ifndef HG_WIFI_H
#define HG_WIFI_H

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#define WIFI_AP  0x01
#define WIFI_STA 0x02

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif 

#define DBG true

#if DBG
#define DEBUG_INFO(M, ...) printf("DEBUG %d: " M, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_INFO(M, ...) do {} while (0)
#endif
#define DEBUG_ERR(M, ...) printf("DEBUG %d: " M, __LINE__, ##__VA_ARGS__)


typedef struct {
  const char *softapIP = "192.168.1.133";
  const char *dnsmasqConf = "/userdata/bin/dnsmasq.conf";
  const char *hostapdConf = "/userdata/bin/hostapd.conf";
  const char *chipTypePath = "/userdata/cfg/rtlchip";
  char wifi_type[32];
  char softap_name[32];
  uint8_t cWifiMode = WIFI_STA;
}WifiConfig;

extern WifiConfig wifiConfig;

/* run console cmd */
const bool console_run(const char *cmdline);

/* get dns or hostapd pid */
int get_pid(const char *Name);

/* get dns pid */
int get_dnsmasq_pid();

/* get hostapd pid */
int get_hostapd_pid();

/* stop hostapd that is rtl type chip */
int wifi_rtl_stop_hostapd();

/* create hostapd file */
int create_hostapd_file(const char* name, const char* password, char *softap_name);

/* create dns file */
bool creat_dnsmasq_file();

/* start to access wlan ap */
int wlan_access_point_start(const char* ssid, const char* password, char *softap_name);

/* ckeck wifi chip type */
int check_wifi_chip_type_string(char *type);

/* switch wifi mode */
int switch_wifi_mode(const char *apName, int32_t status);


#endif // !HG_WIFI_H
 