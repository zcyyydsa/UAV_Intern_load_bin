/**
 * @file hgwifi.h
 * @author gwl
 * @brief switching of wifi mode 
 * @version 1.0
 * @date 2023/6/7
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#ifndef HG_WIFI_H
#define HG_WIFI_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif 

#define SOFTAP_VERSION "1.0"
#define DBG true
#define WIFI_CHIP_TYPE_PATH     "/userdata/cfg/rtlchip"

#if DBG
#define DEBUG_INFO(M, ...) printf("DEBUG %d: " M, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_INFO(M, ...) do {} while (0)
#endif
#define DEBUG_ERR(M, ...) printf("DEBUG %d: " M, __LINE__, ##__VA_ARGS__)

const char SOFTAP_INTERFACE_STATIC_IP[] = "192.168.1.133";
const char DNSMASQ_CONF_DIR[] = "/userdata/bin/dnsmasq.conf";
// hostapd
const char HOSTAPD_CONF_DIR[] = "/userdata/bin/hostapd.conf";

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

#endif // !HG_WIFI_H
 