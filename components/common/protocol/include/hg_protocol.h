#ifndef __HG_PROTOCOL_H_
#define __HG_PROTOCOL_H_

#define AP_NAME      "HG_SOFTAP_TEST"
#include "hg_protocol_types.h"


uint8_t hgprotocol_parse_char(uint8_t chan, uint8_t c, hgprotocol_message_t* r_message, hgprotocol_status_t* r_status);
uint16_t hgprotocol_msg_to_send_buffer(uint8_t *buffer, const hgprotocol_message_t *msg);

uint16_t hgprotocol_lcommand_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lcommand_t* lmsg);
void hgprotocol_lcommand_decode(const hgprotocol_message_t* msg, hgprotocol_lcommand_t* lmsg);

uint16_t hgprotocol_lack_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lack_t* lmsg);
void hgprotocol_lack_decode(const hgprotocol_message_t* msg, hgprotocol_lack_t* lmsg);

uint16_t hgprotocol_lack_extend_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lack_extend_t* lmsg);
void hgprotocol_lack_extend_decode(const hgprotocol_message_t* msg, hgprotocol_lack_extend_t* lmsg);

uint16_t hgprotocol_lfileinfo_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lfileinfo_t* lmsg);
void hgprotocol_lfileinfo_decode(const hgprotocol_message_t* msg, hgprotocol_lfileinfo_t* lmsg);

uint16_t hgprotocol_drone_stats_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_drone_stats_t* lmsg);
void hgprotocol_drone_stats_decode(const hgprotocol_message_t* msg, hgprotocol_drone_stats_t* lmsg);

uint16_t hgprotocol_swarm_track_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_swarm_track_t* lmsg);
void hgprotocol_swarm_track_decode(const hgprotocol_message_t* msg, hgprotocol_swarm_track_t* lmsg);

uint16_t hgprotocol_host_parmas_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_host_parmas_t* lmsg);
void hgprotocol_host_parmas_decode(const hgprotocol_message_t* msg, hgprotocol_host_parmas_t* lmsg);

uint16_t hgprotocol_track_send_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_track_send_t* lmsg);
void hgprotocol_track_send_decode(const hgprotocol_message_t* msg, hgprotocol_track_send_t* lmsg);

#endif
