#include "hg_rc.h"
#include "stdio.h"

typedef enum
{
    IBUS_FRAME_START = 0,
	IBUS_FRAME_CRC1,
	IBUS_FRAME_CRC2,
    IBUS_FRAME_END,
	IBUS_FRAME_IDLE,
} ibus_status;

static uint8_t  decoder_status = IBUS_FRAME_IDLE;
static bool ibus_failsafe;
static bool ibus_frame_drop;
static uint32_t ch_num = 0;

static uint8_t ibus_buffer[IBUS_FRAME_SIZE] = {0};
static uint16_t ibus_values[IBUS_INPUT_CHANNELS] = {0};


bool ibus_decoder(uint8_t ch)
{
    bool new_frame = false;
    static uint16_t checksum = 0;
    static uint16_t rx_checksum = 0xffff;
    switch(decoder_status)
    {
        case IBUS_FRAME_IDLE:
			if(ch == 0x20)
			{
				decoder_status = IBUS_FRAME_START;
				ch_num = 0;
				ibus_buffer[ch_num++] = ch;  
			}
        break;
        
        case IBUS_FRAME_START:
			if(ch_num < IBUS_FRAME_SIZE-1)
			{
				ibus_buffer[ch_num++] = ch;
				if(ch_num == IBUS_FRAME_SIZE-2)
					decoder_status = IBUS_FRAME_CRC1;
			}
			break;
		case IBUS_FRAME_CRC1:
			checksum = ch;
			rx_checksum = 0xffff;
			for(int i=0; i<IBUS_FRAME_SIZE-2; i++)
			{
				rx_checksum -= ibus_buffer[i];
			}
			decoder_status = IBUS_FRAME_CRC2;
			break;
		
		case IBUS_FRAME_CRC2:
			checksum |= ((uint16_t)ch<<8);
			if(checksum == rx_checksum)
			{
				for(int i=0; i<14; i++)
				{
					ibus_values[i] = ((uint16_t)ibus_buffer[2*i+3]<<8) + ibus_buffer[2*i+2];
				}
				new_frame = true;
				ibus_frame_drop = false;
				for(int i=10; i<14; i++)
				{
					ibus_values[i] = 1000;
				}
			}
			else
			{
				new_frame = false;
				ibus_frame_drop = true;
			}
			decoder_status = IBUS_FRAME_IDLE;
			break;
    } 
    return  new_frame;
}

void ibus_get_values(uint16_t *value,uint16_t num_value)
{
    uint16_t i;
    for(i = 0; i < num_value; i++)
        value[i] = ibus_values[i];
}

bool ibus_ifdrop(void)
{
    return ibus_frame_drop;
}

bool ibus_iffail(void)
{
    return ibus_failsafe;
}
