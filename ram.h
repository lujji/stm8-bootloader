#ifndef RAM_H
#define RAM_H

#include <stdint.h>

#define BLOCK_SIZE  64

void flash_write_block(uint16_t addr, const uint8_t *buf, uint16_t len);

#endif /* RAM_H */
