#ifndef RAM_H
#define RAM_H

#include <stdint.h>

void ram_flash_write_block(uint16_t addr, const uint8_t *buf);

#endif /* RAM_H */
