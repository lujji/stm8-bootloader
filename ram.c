#include "ram.h"
#include "stm8.h"
#include "config.h"

#pragma codeseg RAM_SEG
void ram_flash_write_block(uint16_t addr, const uint8_t *buf) {
    /* enable block programming */
    FLASH_CR2 = 1 << FLASH_CR2_PRG;
#if !(STM8L)
    FLASH_NCR2 = (uint8_t) ~(1 << FLASH_NCR2_NPRG);
#endif

    /* write data from buffer */
    for (uint8_t i = 0; i < BLOCK_SIZE; i++)
        _MEM_(addr + i) = buf[i];

    /* wait for operation to complete */
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_EOP)));
}
