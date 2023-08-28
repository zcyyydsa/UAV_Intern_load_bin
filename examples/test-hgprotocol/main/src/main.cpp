#include <stdio.h>

#include "hg_protocol.h"

void handle_msg(hgprotocol_message_t* msg)
{
    printf("msgid: %d\n", msg->msgid);
    switch(msg->msgid)
    {
        case HGPROTOCOL_MSG_ID_LCOMMAND:
        {
            hgprotocol_lcommand_t cmd_msg;
            hgprotocol_lcommand_decode(msg, &cmd_msg);
            printf("start_id: %d\n", cmd_msg.start_id);
            break;
        }
        case HGPROTOCOL_MSG_ID_LACK:
        {
            hgprotocol_lack_t ack;
            hgprotocol_lack_decode(msg, &ack);
            printf("type=%d id=%d res=%d cmd=%d\n", ack.type, ack.id, ack.result, ack.cmd);
            break;
        }
        case HGPROTOCOL_MSG_ID_LACK_EXTEND:
        {
            hgprotocol_lack_extend_t ack;
            hgprotocol_lack_extend_decode(msg, &ack);
            printf("extend=%s\n", ack.extend);
            break;
        }
        case HGPROTOCOL_MSG_ID_LFILEINFO:
        {
            hgprotocol_lfileinfo_t file;
            hgprotocol_lfileinfo_decode(msg, &file);
            printf("size=%d id=%d type=%d\n", file.file_size, file.id, file.file_type);
            printf("ver=%s md5=%s name=%s\n", file.version, file.md5, file.file_name);
            break;   
        }
        case HGPROTOCOL_MSG_ID_DRONE_STATS:
        {
            hgprotocol_drone_stats_t dst;
            hgprotocol_drone_stats_decode(msg, &dst);
            printf("id=%d type=%d x=%f\n", dst.id, dst.type, dst.x);
            break;
        }
        default:
            break;
    }
}

int main()
{
    uint8_t sbuf[512] = {0};
    hgprotocol_message_t msg;

// msg send test    
    hgprotocol_lcommand_t cmd_msg = {0};
    cmd_msg.start_id = 100;
    hgprotocol_lcommand_encode(0, 0, &msg, &cmd_msg);
    int size = hgprotocol_msg_to_send_buffer(sbuf, &msg);
    
    hgprotocol_lack_t ack = {0};
    ack.type = 0;
    ack.id = 88;
    ack.result = 1;
    ack.cmd = 12;
    hgprotocol_lack_encode(0, 0, &msg, &ack);
    int size2 = hgprotocol_msg_to_send_buffer(sbuf+size, &msg);

    hgprotocol_lack_extend_t ack2 = {0};
    sprintf((char*)ack2.extend, "/sdcard/VID_20230101.mp4");
    hgprotocol_lack_extend_encode(0, 0, &msg, &ack2);
    int size3 = hgprotocol_msg_to_send_buffer(sbuf+size+size2, &msg);
    
    hgprotocol_lfileinfo_t dance_file = {0};
    dance_file.file_size =11;
    dance_file.id =1;
    dance_file.file_type = 0;
    sprintf((char*)dance_file.version, "13");
    sprintf((char*)dance_file.md5, "jflsjlsd");
    sprintf((char*)dance_file.file_name, "/sdcard/dancefile1.json");
    hgprotocol_lfileinfo_encode(0, 0, &msg, &dance_file);
    int size4 = hgprotocol_msg_to_send_buffer(sbuf+size+size2+size3, &msg);

    hgprotocol_drone_stats_t dst = {0};
    dst.id = 33;
    dst.type = 9;
    dst.x = 3;
    hgprotocol_drone_stats_encode(0, 0, &msg, &dst);
    int size5 = hgprotocol_msg_to_send_buffer(sbuf+size+size2+size3+size4, &msg);


// msg rec test
    int total_size = size + size2 + size3 + size4 + size5;  
    hgprotocol_message_t r_msg;
    hgprotocol_status_t r_status;
    for (int i=0; i<total_size; i++) {
        if (HGPROTOCOL_FRAMING_OK == hgprotocol_parse_char(0, sbuf[i], &r_msg, &r_status))
        {
            handle_msg(&r_msg);
        }
    }
    
    return 0;
}
