#ifndef RAM_H
#define RAM_H

#include <stdint.h>

void ram_flash_write_block(uint16_t addr, const uint8_t *buf);
void ram_write_rop_byte(uint16_t addr, const uint8_t opt);

#endif /* RAM_H */
