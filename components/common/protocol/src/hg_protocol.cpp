
#include "hg_protocol.h"

#include <string.h>

#define _HGPROTOCOL_PAYLOAD(msg) ((const char *)(&((msg)->payload64[0])))
#define _HGPROTOCOL_PAYLOAD_NON_CONST(msg) ((char *)(&((msg)->payload64[0])))

// checksum is immediately after the payload bytes
#define hgprotocol_ck_a(msg) *((msg)->len + (uint8_t *)_HGPROTOCOL_PAYLOAD_NON_CONST(msg))
#define hgprotocol_ck_b(msg) *(((msg)->len+(uint16_t)1) + (uint8_t *)_HGPROTOCOL_PAYLOAD_NON_CONST(msg))

static inline void hgprotocol_array_memcpy(void *dest, const void *src, size_t n)
{
	if (src == NULL) {
		memset(dest, 0, n);
	} else {
		memcpy(dest, src, n);
	}
}

static inline void hgprotocol_crc_init(uint16_t* crcAccum)
{
    *crcAccum = 0xffff;//X25_INIT_CRC;
}

static inline void hgprotocol_crc_accumulate(uint8_t data, uint16_t *crcAccum)
{
    /*Accumulate one byte of data into the CRC*/
    uint8_t tmp;

    tmp = data ^ (uint8_t)(*crcAccum &0xff);
    tmp ^= (tmp<<4);
    *crcAccum = (*crcAccum>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);
}

static inline uint16_t hgprotocol_crc_calculate(const uint8_t* pBuffer, uint16_t length)
{
    uint16_t crcTmp;
    hgprotocol_crc_init(&crcTmp);
    while (length--) {
        hgprotocol_crc_accumulate(*pBuffer++, &crcTmp);
    }
    return crcTmp;
}

static inline void hgprotocol_crc_accumulate_buffer(uint16_t *crcAccum, const char *pBuffer, uint16_t length)
{
    const uint8_t *p = (const uint8_t *)pBuffer;
    while (length--) {
        hgprotocol_crc_accumulate(*p++, crcAccum);
    }
}

hgprotocol_message_t* hgprotocol_get_channel_buffer(uint8_t chan)
{
	static hgprotocol_message_t m_hgprotocol_buffer[HGPROTOCOL_COMM_NUM_BUFFERS];
	return &m_hgprotocol_buffer[chan];
}

hgprotocol_status_t* hgprotocol_get_channel_status(uint8_t chan)
{
	static hgprotocol_status_t m_hgprotocol_status[HGPROTOCOL_COMM_NUM_BUFFERS];
	return &m_hgprotocol_status[chan];
}

uint16_t hgprotocol_finalize_message(hgprotocol_message_t* msg, uint8_t system_id, uint8_t component_id, uint8_t chan, uint8_t length)
{
	// This is only used for the v2 protocol and we silence it here
	//(void)min_length;
	// This code part is the same for all messages;
	msg->magic = HGPROTOCOL_STX;
	msg->len = length;
	msg->sysid = system_id;
	msg->compid = component_id;
	// One sequence number per channel
	msg->seq = hgprotocol_get_channel_status(chan)->current_tx_seq;
    hgprotocol_get_channel_status(chan)->current_tx_seq = hgprotocol_get_channel_status(chan)->current_tx_seq+1;
	msg->checksum = hgprotocol_crc_calculate(((const uint8_t*)(msg)) + 3, HGPROTOCOL_CORE_HEADER_LEN);
	hgprotocol_crc_accumulate_buffer(&msg->checksum, _HGPROTOCOL_PAYLOAD(msg), msg->len);

	hgprotocol_ck_a(msg) = (uint8_t)(msg->checksum & 0xFF);
	hgprotocol_ck_b(msg) = (uint8_t)(msg->checksum >> 8);

	return length + HGPROTOCOL_NUM_NON_PAYLOAD_BYTES;
}

/**
 * @brief Pack a message to send it over a serial byte stream
 */
uint16_t hgprotocol_msg_to_send_buffer(uint8_t *buffer, const hgprotocol_message_t *msg)
{
	memcpy(buffer, (const uint8_t *)&msg->magic, HGPROTOCOL_NUM_HEADER_BYTES + (uint16_t)msg->len);

	uint8_t *ck = buffer + (HGPROTOCOL_NUM_HEADER_BYTES + (uint16_t)msg->len);

	ck[0] = (uint8_t)(msg->checksum & 0xFF);
	ck[1] = (uint8_t)(msg->checksum >> 8);

	return HGPROTOCOL_NUM_NON_PAYLOAD_BYTES + (uint16_t)msg->len;
}

void hgprotocol_start_checksum(hgprotocol_message_t* msg)
{
	hgprotocol_crc_init(&msg->checksum);
}

void hgprotocol_update_checksum(hgprotocol_message_t* msg, uint8_t c)
{
	hgprotocol_crc_accumulate(c, &msg->checksum);
}

uint8_t hgprotocol_frame_char_buffer(hgprotocol_message_t* rxmsg, 
                                                 hgprotocol_status_t* status,
                                                 uint8_t c, 
                                                 hgprotocol_message_t* r_message, 
                                                 hgprotocol_status_t* r_status)
{
    /*
	  default message crc function. You can override this per-system to
	  put this data in a different memory segment
	*/
// #if HGPROTOCOL_CRC_EXTRA
// #ifndef HGPROTOCOL_MESSAGE_CRC
// 	static const uint8_t hgprotocol_message_crcs[256][2] = HGPROTOCOL_MESSAGE_LENGTHS_CRCS;
// #define HGPROTOCOL_MESSAGE_CRC(msgid) hgprotocol_message_crcs[msgid][1]
// #endif
// #endif

	/* Enable this option to check the length of each message.
	   This allows invalid messages to be caught much sooner. Use if the transmission
	   medium is prone to missing (or extra) characters (e.g. a radio that fades in
	   and out). Only use if the channel will only contain messages types listed in
	   the headers.
	*/
// #ifdef HGPROTOCOL_CHECK_MESSAGE_LENGTH
// #ifndef HGPROTOCOL_MESSAGE_LENGTH
// 	static const uint8_t hgptotocol_message_lengths[256][2] = HGPROTOCOL_MESSAGE_LENGTHS_CRCS;
// #define HGPROTOCOL_MESSAGE_LENGTH(msgid) hgprotocol_message_lengths[msgid][0]
// #endif
// #endif

	int bufferIndex = 0;

	status->msg_received = HGPROTOCOL_FRAMING_INCOMPLETE;

	switch (status->parse_state)
	{
	case HGPROTOCOL_PARSE_STATE_UNINIT:
	case HGPROTOCOL_PARSE_STATE_IDLE:
		if (c == HGPROTOCOL_STX)
		{
			status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_STX;
			rxmsg->len = 0;
			rxmsg->magic = c;
			hgprotocol_start_checksum(rxmsg);
		}
		break;

	case HGPROTOCOL_PARSE_STATE_GOT_STX:
        if (status->msg_received 
            /* Support shorter buffers than the
            default maximum packet size */
            #if (HGPROTOCOL_MAX_PAYLOAD_LEN < 255)
                            || c > HGPROTOCOL_MAX_PAYLOAD_LEN
            #endif
                            )
		{
			status->buffer_overrun++;
			status->parse_error++;
			status->msg_received = 0;
			status->parse_state = HGPROTOCOL_PARSE_STATE_IDLE;
		}
		else
		{
			// NOT counting STX, LENGTH, SEQ, SYSID, COMPID, MSGID, CRC1 and CRC2
			rxmsg->len = c;
			status->packet_idx = 0;
			hgprotocol_update_checksum(rxmsg, c);
			status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_LENGTH;
		}
		break;

	case HGPROTOCOL_PARSE_STATE_GOT_LENGTH:
		rxmsg->seq = c;
		hgprotocol_update_checksum(rxmsg, c);
		status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_SEQ;
		break;

	case HGPROTOCOL_PARSE_STATE_GOT_SEQ:
		rxmsg->sysid = c;
		hgprotocol_update_checksum(rxmsg, c);
		status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_SYSID;
		break;

	case HGPROTOCOL_PARSE_STATE_GOT_SYSID:
		rxmsg->compid = c;
		hgprotocol_update_checksum(rxmsg, c);
		status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_COMPID;
		break;

	case HGPROTOCOL_PARSE_STATE_GOT_COMPID:
// #ifdef HGPROTOCOL_CHECK_MESSAGE_LENGTH
// 	        if (rxmsg->len != HGPROTOCOL_MESSAGE_LENGTH(c))
// 		{
// 			status->parse_error++;
// 			status->parse_state = HGPROTOCOL_PARSE_STATE_IDLE;
// 			break;
// 	    }
// #endif
		rxmsg->msgid = c;
		hgprotocol_update_checksum(rxmsg, c);
		if (rxmsg->len == 0)
		{
			status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_PAYLOAD;
		}
		else
		{
			status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_MSGID;
		}
		break;

	case HGPROTOCOL_PARSE_STATE_GOT_MSGID:
		_HGPROTOCOL_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx++] = (char)c;
		hgprotocol_update_checksum(rxmsg, c);
		if (status->packet_idx == rxmsg->len)
		{
			status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_PAYLOAD;
		}
		break;

	case HGPROTOCOL_PARSE_STATE_GOT_PAYLOAD:
// #if MAVLINK_CRC_EXTRA
// 		mavlink_update_checksum(rxmsg, MAVLINK_MESSAGE_CRC(rxmsg->msgid));
// #endif
		if (c != (rxmsg->checksum & 0xFF)) {
			status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_BAD_CRC1;
		} else {
			status->parse_state = HGPROTOCOL_PARSE_STATE_GOT_CRC1;
		}
        _HGPROTOCOL_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx] = (char)c;
		break;

	case HGPROTOCOL_PARSE_STATE_GOT_CRC1:
	case HGPROTOCOL_PARSE_STATE_GOT_BAD_CRC1:
		if (status->parse_state == HGPROTOCOL_PARSE_STATE_GOT_BAD_CRC1 || c != (rxmsg->checksum >> 8)) {
			// got a bad CRC message
			status->msg_received = HGPROTOCOL_FRAMING_BAD_CRC;
		} else {
			// Successfully got message
			status->msg_received = HGPROTOCOL_FRAMING_OK;
        }
        status->parse_state = HGPROTOCOL_PARSE_STATE_IDLE;
        _HGPROTOCOL_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx+1] = (char)c;
        memcpy(r_message, rxmsg, sizeof(hgprotocol_message_t));
		break;
	}

	bufferIndex++;
	// If a message has been sucessfully decoded, check index
	if (status->msg_received == HGPROTOCOL_FRAMING_OK)
	{
		//while(status->current_seq != rxmsg->seq)
		//{
		//	status->packet_rx_drop_count++;
		//               status->current_seq++;
		//}
		status->current_rx_seq = rxmsg->seq;
		// Initial condition: If no packet has been received so far, drop count is undefined
		if (status->packet_rx_success_count == 0) status->packet_rx_drop_count = 0;
		// Count this packet as received
		status->packet_rx_success_count++;
	}

	r_message->len = rxmsg->len; // Provide visibility on how far we are into current msg
	r_status->parse_state = status->parse_state;
	r_status->packet_idx = status->packet_idx;
	r_status->current_rx_seq = status->current_rx_seq+1;
	r_status->packet_rx_success_count = status->packet_rx_success_count;
	r_status->packet_rx_drop_count = status->parse_error;
	status->parse_error = 0;

	if (status->msg_received == HGPROTOCOL_FRAMING_BAD_CRC) {
		/*
		  the CRC came out wrong. We now need to overwrite the
		  msg CRC with the one on the wire so that if the
		  caller decides to forward the message anyway that
		  mavlink_msg_to_send_buffer() won't overwrite the
		  checksum
		 */
		r_message->checksum = _HGPROTOCOL_PAYLOAD(rxmsg)[status->packet_idx] | (_HGPROTOCOL_PAYLOAD(rxmsg)[status->packet_idx+1]<<8);
	}

	return status->msg_received;
}

uint8_t hgprotocol_parse_char(uint8_t chan, uint8_t c, hgprotocol_message_t* r_message, hgprotocol_status_t* r_status)
{
	return hgprotocol_frame_char_buffer(hgprotocol_get_channel_buffer(chan),
					 hgprotocol_get_channel_status(chan),
					 c,
					 r_message,
					 r_status);
}

///
/// hgprotocol msg encode and decode

// lcommand
uint16_t hgprotocol_lcommand_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lcommand_t* lmsg)
{
//    hgprotocol_lcommand_t packet;
//    packet.utc = lmsg->utc;
//    packet.start_id = lmsg->start_id;
//    packet.end_id = lmsg->end_id;
//    packet.cmd = lmsg->cmd;
//    packet.ack = lmsg->ack;
//    hgprotocol_array_memcpy(packet.param, lmsg->param, sizeof(int32_t)*8);
    memcpy(_HGPROTOCOL_PAYLOAD_NON_CONST(msg), lmsg, sizeof(hgprotocol_lcommand_t));

    msg->msgid = HGPROTOCOL_MSG_ID_LCOMMAND;

    return hgprotocol_finalize_message(msg, system_id, component_id, 0, sizeof(hgprotocol_lcommand_t));
}
void hgprotocol_lcommand_decode(const hgprotocol_message_t* msg, hgprotocol_lcommand_t* lmsg)
{
    int msg_size = sizeof(hgprotocol_lcommand_t);
    uint8_t len = msg->len < msg_size? msg->len : msg_size;
    memset(lmsg, 0, msg_size);
    memcpy(lmsg, _HGPROTOCOL_PAYLOAD(msg), len);    
}

// lack
uint16_t hgprotocol_lack_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lack_t* lmsg)
{
//    hgprotocol_lack_t packet;
//    packet.token = lmsg->token;
//    packet.id = lmsg->id;
//    packet.cmd = lmsg->cmd;
//    packet.result = lmsg->result;
//    packet.type = lmsg->type;
    memcpy(_HGPROTOCOL_PAYLOAD_NON_CONST(msg), lmsg, sizeof(hgprotocol_lack_t));

    msg->msgid = HGPROTOCOL_MSG_ID_LACK;

    return hgprotocol_finalize_message(msg, system_id, component_id, 0, sizeof(hgprotocol_lack_t));
}
void hgprotocol_lack_decode(const hgprotocol_message_t* msg, hgprotocol_lack_t* lmsg)
{
    int msg_size = sizeof(hgprotocol_lack_t);
    uint8_t len = msg->len < msg_size? msg->len : msg_size;
    memset(lmsg, 0, msg_size);
    memcpy(lmsg, _HGPROTOCOL_PAYLOAD(msg), len);    
}

uint16_t hgprotocol_lack_extend_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lack_extend_t* lmsg)
{
//    hgprotocol_lack_extend_t packet;
//    packet.token = lmsg->token;
//    packet.id = lmsg->id;
//    packet.cmd = lmsg->cmd;
//    packet.result = lmsg->result;
//    packet.type = lmsg->type;
//    hgprotocol_array_memcpy(packet.extend, lmsg->extend, sizeof(int8_t)*64);
    memcpy(_HGPROTOCOL_PAYLOAD_NON_CONST(msg), lmsg, sizeof(hgprotocol_lack_extend_t));

    msg->msgid = HGPROTOCOL_MSG_ID_LACK_EXTEND;

    return hgprotocol_finalize_message(msg, system_id, component_id, 0, sizeof(hgprotocol_lack_extend_t));
}
void hgprotocol_lack_extend_decode(const hgprotocol_message_t* msg, hgprotocol_lack_extend_t* lmsg)
{
    int msg_size = sizeof(hgprotocol_lack_extend_t);
    uint8_t len = msg->len < msg_size? msg->len : msg_size;
    memset(lmsg, 0, msg_size);
    memcpy(lmsg, _HGPROTOCOL_PAYLOAD(msg), len);  
}

uint16_t hgprotocol_lfileinfo_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_lfileinfo_t* lmsg)
{
//    hgprotocol_lfileinfo_t packet;
//    packet.file_size= lmsg->file_size;
//    packet.id= lmsg->id;
//    packet.file_type= lmsg->file_type;
//    hgprotocol_array_memcpy(packet.version, lmsg->version, sizeof(uint8_t)*3);
//    hgprotocol_array_memcpy(packet.md5, lmsg->md5, sizeof(uint8_t)*16);
//    hgprotocol_array_memcpy(packet.file_name, lmsg->file_name, sizeof(int8_t)*64);
    memcpy(_HGPROTOCOL_PAYLOAD_NON_CONST(msg), lmsg, sizeof(hgprotocol_lfileinfo_t));

    msg->msgid = HGPROTOCOL_MSG_ID_LFILEINFO;

    return hgprotocol_finalize_message(msg, system_id, component_id, 0, sizeof(hgprotocol_lfileinfo_t));
}
void hgprotocol_lfileinfo_decode(const hgprotocol_message_t* msg, hgprotocol_lfileinfo_t* lmsg)
{
    int msg_size = sizeof(hgprotocol_lfileinfo_t);
    uint8_t len = msg->len < msg_size? msg->len : msg_size;
    memset(lmsg, 0, msg_size);
    memcpy(lmsg, _HGPROTOCOL_PAYLOAD(msg), len);  
}

uint16_t hgprotocol_drone_stats_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_drone_stats_t* lmsg)
{
    memcpy(_HGPROTOCOL_PAYLOAD_NON_CONST(msg), lmsg, sizeof(hgprotocol_drone_stats_t));
    msg->msgid = HGPROTOCOL_MSG_ID_DRONE_STATS;

    return hgprotocol_finalize_message(msg, system_id, component_id, 0, sizeof(hgprotocol_drone_stats_t));
}

void hgprotocol_drone_stats_decode(const hgprotocol_message_t* msg, hgprotocol_drone_stats_t* lmsg)
{
    int msg_size = sizeof(hgprotocol_drone_stats_t);
    uint8_t len = msg->len < msg_size? msg->len : msg_size;
    memset(lmsg, 0, msg_size);
    memcpy(lmsg, _HGPROTOCOL_PAYLOAD(msg), len); 
}

uint16_t hgprotocol_swarm_track_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_swarm_track_t* lmsg)
{
    memcpy(_HGPROTOCOL_PAYLOAD_NON_CONST(msg), lmsg, sizeof(hgprotocol_swarm_track_t));
    msg->msgid = HGPROTOCOL_MSG_ID_SWARM_TRACK;

    return hgprotocol_finalize_message(msg, system_id, component_id, 0, sizeof(hgprotocol_swarm_track_t));
}

void hgprotocol_swarm_track_decode(const hgprotocol_message_t* msg, hgprotocol_swarm_track_t* lmsg)
{
    int msg_size = sizeof(hgprotocol_swarm_track_t);
    uint8_t len = msg->len < msg_size? msg->len : msg_size;
    memset(lmsg, 0, msg_size);
    memcpy(lmsg, _HGPROTOCOL_PAYLOAD(msg), len); 
}

uint16_t hgprotocol_host_parmas_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_host_parmas_t* lmsg)
{
    memcpy(_HGPROTOCOL_PAYLOAD_NON_CONST(msg), lmsg, sizeof(hgprotocol_host_parmas_t));
    msg->msgid = HGPROTOCOL_MSG_ID_HOST_PARMAS;

    return hgprotocol_finalize_message(msg, system_id, component_id, 0, sizeof(hgprotocol_host_parmas_t));
}

void hgprotocol_host_parmas_decode(const hgprotocol_message_t* msg, hgprotocol_host_parmas_t* lmsg)
{
    int msg_size = sizeof(hgprotocol_host_parmas_t);
    uint8_t len = msg->len < msg_size? msg->len : msg_size;
    memset(lmsg, 0, msg_size);
    memcpy(lmsg, _HGPROTOCOL_PAYLOAD(msg), len); 
}

uint16_t hgprotocol_track_send_encode(uint8_t system_id, uint8_t component_id, hgprotocol_message_t* msg, const hgprotocol_track_send_t* lmsg)
{
    memcpy(_HGPROTOCOL_PAYLOAD_NON_CONST(msg), lmsg, sizeof(hgprotocol_track_send_t));
    msg->msgid = HGPROTOCOL_MSG_ID_TRACK_SEND;

    return hgprotocol_finalize_message(msg, system_id, component_id, 0, sizeof(hgprotocol_track_send_t));
}

void hgprotocol_track_send_decode(const hgprotocol_message_t* msg, hgprotocol_track_send_t* lmsg)
{
    int msg_size = sizeof(hgprotocol_track_send_t);
    uint8_t len = msg->len < msg_size? msg->len : msg_size;
    memset(lmsg, 0, msg_size);
    memcpy(lmsg, _HGPROTOCOL_PAYLOAD(msg), len); 
}
