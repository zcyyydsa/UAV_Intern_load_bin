/**
 * @file test_wifi.cpp
 * @author gwl
 * @brief switching of wifi mode test
 * @version 1.0
 * @date 2023/6/7
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#include "hgwifi.h"

static char wifi_type[64];
char softap_name[64];

int main(int argc, char **argv) {
    const char *str_stop = "stop";
    const char *apName = "HG_SOFTAP_TEST";

    DEBUG_INFO("\nsoftap_version: %s\n", SOFTAP_VERSION);

    check_wifi_chip_type_string(wifi_type);
    DEBUG_INFO("\nwifi type: %s\n",wifi_type);

    if (!strncmp(wifi_type, "RTL", 3))
        strcpy(softap_name, "p2p0");
    else
        strcpy(softap_name, "wlan1");

    if (argc >= 2) {
        if (!strncmp(wifi_type, "RTL", 3)) {
            if (strcmp(argv[1], str_stop) == 0) {
                DEBUG_INFO("-stop softap-\n");
                wifi_rtl_stop_hostapd();
                system("ifconfig p2p0 down");
                return 0;
            }
        } else {
            if (strcmp(argv[1],str_stop) == 0) {
                DEBUG_INFO("-stop softap-\n");
                wifi_rtl_stop_hostapd();
                console_run("killall dnsmasq");
                console_run("ifconfig wlan1 down");
                return 0;
            }
        }
        apName = argv[1];
    }


    console_run("killall dnsmasq");
    console_run("killall hostapd");
    console_run("killall udhcpc");

    DEBUG_INFO("start softap with name: %s---", apName);
    if (!strncmp(wifi_type, "RTL", 3)) {
        printf("do rtl8188eu p2p0\n");
        console_run("ifconfig p2p0 down");
        console_run("rm -rf /userdata/bin/p2p0");
        wlan_access_point_start(apName, "123456789", softap_name);
    } else {
        printf("do not rtl8188eu p2p0\n");
        console_run("ifconfig wlan1 down");
        console_run("rm -rf /userdata/bin/wlan1");
        console_run("iw dev wlan1 del");
        console_run("ifconfig wlan0 up");
        if (!strncmp(wifi_type, "AP6181", 6))
            console_run("iw dev wlan0 interface add wlan1 type __ap");
        else
            console_run("iw phy0 interface add wlan1 type managed");
        wlan_access_point_start(apName, "123456789", softap_name);
    }
    return 0;
}
