
#include <signal.h>

#include "log.h"
#include "uorb_topic.h"
#include "media_server.h"
#include "device_server.h"
#include "algorithm_server.h"
#include "msg_server.h"

bool mrunning = true;

static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    mrunning = false;
    exit(0);
}

int topic_advertise_management(void);
void topic_unadvertise_management(void);

int main(int argc, char **argv)
{
    int ret = -1;
    signal(SIGINT, sigterm_handler);

    log_init();
    ret = topic_advertise_management();
    if (ret != 0) {
       HGLOG_ERROR("[main] check extboard.");
       exit(0);
    }
    HGLOG_INFO("-------------main----------------");
    
    MsgServer msgServer;
    msgServer.start();

    MediaServer mediaServer;
    mediaServer.init();
    mediaServer.start();

    DeviceServer deviceServer;
    deviceServer.init();
    deviceServer.start();

    AlgorithmServer algoServer;
    algoServer.start();
    
    while (mrunning) {
        sleep(3);
    }
    msgServer.stop();
    mediaServer.stop();
    deviceServer.stop();
    algoServer.stop();

    topic_unadvertise_management();
    log_deinit();

    return ret;
}

int topic_advertise_management(void) {
    int ret = -1;
    ret = topic_init();
    if (0 != ret)
        return -1;

    ret = topic_advertise(TOPIC_ID(mount_control));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(mount_status));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(drone_pos_acc));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(drone_stats));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(drone_ack));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(drone_ctrl));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(swarm_track));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(host_parmas));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(track_send));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(media_ctrl));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(drone_id));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(drone_attitude));if (ret != 0)return ret;
    ret = topic_advertise(TOPIC_ID(drone_ctrl_algo));if (ret != 0)return ret;

    return ret;
}

void topic_unadvertise_management(void) {
    topic_unadvertise(TOPIC_ID(mount_control));
    topic_unadvertise(TOPIC_ID(mount_status));
    topic_unadvertise(TOPIC_ID(drone_pos_acc));
    topic_unadvertise(TOPIC_ID(drone_stats));
    topic_unadvertise(TOPIC_ID(drone_ack));
    topic_unadvertise(TOPIC_ID(drone_ctrl));
    topic_unadvertise(TOPIC_ID(swarm_track));
    topic_unadvertise(TOPIC_ID(host_parmas));
    topic_unadvertise(TOPIC_ID(track_send));
    topic_unadvertise(TOPIC_ID(media_ctrl));
    topic_unadvertise(TOPIC_ID(drone_id));
    topic_unadvertise(TOPIC_ID(drone_attitude));
    topic_unadvertise(TOPIC_ID(drone_ctrl_algo));
}

