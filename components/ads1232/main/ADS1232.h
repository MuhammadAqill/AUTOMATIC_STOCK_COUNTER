#ifndef ADS1232_H
#define ADS1232_H

#include <stdint.h>

int32_t ads1232_read_raw(void);
float ads1232_read_grams_with_offset(int32_t zero_offset);

#endif // ADS1232_H
