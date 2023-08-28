#ifndef __HG_RC_H_
#define __HG_RC_H_

#include <stdbool.h>
#include <stdint.h>

#define RC_PORT "/dev/ttysWK3"
#define RC_BUNDRATE 115200
#define IBUS_FRAME_SIZE		32
#define IBUS_INPUT_CHANNELS	15
	
bool ibus_decoder(uint8_t ch);
void ibus_get_values(uint16_t *value, uint16_t num_value);
bool ibus_ifdrop(void);
bool ibus_iffail(void);

#endif
