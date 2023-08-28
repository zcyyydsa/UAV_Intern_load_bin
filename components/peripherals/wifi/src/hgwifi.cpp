/**
 * @file hgwifi.cpp
 * @author gwl
 * @brief switching of wifi mode 
 * @version 1.0
 * @date 2023/6/7
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#include "hgwifi.h"

const bool console_run(const char *cmdline) {
    DEBUG_INFO("cmdline = %s\n", cmdline);
    int ret;
    ret = system(cmdline);
    if (ret < 0) {
        DEBUG_ERR("Running cmdline failed: %s\n", cmdline);
        return false;
    }
    return true;
}


int get_pid(const char *Name) {
    int len;
    char name[20] = {0};
    len = strlen(Name);
    strncpy(name,Name,len);
    name[len] ='\0';
    char cmdresult[256] = {0};
    char cmd[20] = {0};
    FILE *pFile = NULL;
    int  pid = 0;

    sprintf(cmd, "pidof %s", name);
    pFile = popen(cmd, "r");
    if (pFile != NULL) {
        while (fgets(cmdresult, sizeof(cmdresult), pFile)) {
            pid = atoi(cmdresult);
            DEBUG_INFO("--- %s pid = %d ---\n", name, pid);
            break;
        }
    }
    pclose(pFile);
    return pid;
}

int get_dnsmasq_pid() {
    int ret;
    ret = get_pid("dnsmasq");
    return ret;
}

int get_hostapd_pid() {
    int ret;
    ret = get_pid("hostapd");
    return ret;
}


int wifi_rtl_stop_hostapd() {
    int pid;
    char *cmd = NULL;

    pid = get_hostapd_pid();

    if (pid!=0) {
        asprintf(&cmd, "kill %d", pid);
        console_run(cmd);
        console_run("killall hostapd");
        free(cmd);
    }
    return 0;
}

int create_hostapd_file(const char* name, const char* password, char *softap_name) {
    FILE* fp;
    char cmdline[256] = {0};

    fp = fopen(HOSTAPD_CONF_DIR, "wt+");

    if (fp != 0) {
        sprintf(cmdline, "interface=%s\n", softap_name);
        fputs(cmdline, fp);
        fputs("ctrl_interface=/var/run/hostapd\n", fp);
        fputs("driver=nl80211\n", fp);
        fputs("ssid=", fp);
        fputs(name, fp);
        fputs("\n", fp);
        fputs("channel=6\n", fp);
        fputs("hw_mode=g\n", fp);
        fputs("ieee80211n=1\n", fp);
        fputs("ignore_broadcast_ssid=0\n", fp);
#if 0
        fputs("auth_algs=1\n", fp);
        fputs("wpa=3\n", fp);
        fputs("wpa_passphrase=", fp);
        fputs(password, fp);
        fputs("\n", fp);
        fputs("wpa_key_mgmt=WPA-PSK\n", fp);
        fputs("wpa_pairwise=TKIP\n", fp);
        fputs("rsn_pairwise=CCMP", fp);
#endif
        fclose(fp);
        return 0;
    }
    return -1;
}

bool creat_dnsmasq_file() {
    FILE* fp;
    fp = fopen(DNSMASQ_CONF_DIR, "wt+");
    if (fp != 0) {
        fputs("user=root\n", fp);
        fputs("listen-address=", fp);
        fputs(SOFTAP_INTERFACE_STATIC_IP, fp);
        fputs("\n", fp);
        fputs("dhcp-range=192.168.1.50,192.168.1.150\n", fp);
        fputs("server=/google/8.8.8.8\n", fp);
        fclose(fp);
        return true;
    }
    DEBUG_ERR("---open dnsmasq configuarion file failed!!---");
    return true;
}

int wlan_access_point_start(const char* ssid, const char* password, char *softap_name) {
    char cmdline[256] = {0};
    create_hostapd_file(ssid, password, softap_name);

    console_run("killall dnsmasq");
    sprintf(cmdline, "ifconfig %s up", softap_name);
    console_run(cmdline);
    sprintf(cmdline, "ifconfig %s 192.168.1.133 netmask 255.255.255.0", softap_name);
    console_run(cmdline);
    //sprintf(cmdline, "route add default gw 192.168.1.133 %s", softap_name);
    //console_run(cmdline);
    creat_dnsmasq_file();
    int dnsmasq_pid = get_dnsmasq_pid();
    if (dnsmasq_pid != 0) {
        memset(cmdline, 0, sizeof(cmdline));
        sprintf(cmdline, "kill %d", dnsmasq_pid);
        console_run(cmdline);
    }
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline, "dnsmasq -C %s --interface=%s", DNSMASQ_CONF_DIR, softap_name);
    console_run(cmdline);

    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline, "hostapd %s &", HOSTAPD_CONF_DIR);
    console_run(cmdline);
    return 1;
}

int check_wifi_chip_type_string(char *type)
{
    int wififd, ret = 0;
    char buf[64];

    wififd = open(WIFI_CHIP_TYPE_PATH, O_RDONLY);
    if (wififd < 0 ) {
        DEBUG_ERR("Can't open %s, errno = %d", WIFI_CHIP_TYPE_PATH, errno);
        ret = -1;
        goto fail_exit;
    }
    memset(buf, 0, 64);

    if (0 == read(wififd, buf, 32)) {
        DEBUG_ERR("read %s failed", WIFI_CHIP_TYPE_PATH);
        close(wififd);
        ret = -1;
        goto fail_exit;
    }
    close(wififd);

    strcpy(type, buf);
    DEBUG_INFO("%s: %s", __FUNCTION__, type);

fail_exit:
    return ret;
}

