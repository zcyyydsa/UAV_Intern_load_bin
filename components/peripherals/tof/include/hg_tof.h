#ifndef __HG_TOF_H_
#define __HG_TOF_H_

#include <stdint.h>

#define TOF_PORT     "/dev/ttysWK1"
#define TOF_BUNDRATE 115200

typedef struct {
  int height;
  int strength;
} tof_data_t;

int tof_parse_char(uint8_t b, tof_data_t *data);

#endif // !HG_TOF_H