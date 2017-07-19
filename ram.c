#include "ram.h"
#include <stm8s.h>

#pragma codeseg RAM_SEG
void flash_write_block(uint16_t addr, const uint8_t *buf, uint16_t len) {
    const uint8_t *end = buf + len;
    volatile uint8_t *mem = (volatile uint8_t *)(addr);

//     /* unlock flash */
//     FLASH_PUKR = FLASH_PUKR_KEY1;
//     FLASH_PUKR = FLASH_PUKR_KEY2;
//     while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_PUL)));

    /* enable block programming */
    FLASH_CR2 = 1 << FLASH_CR2_PRG;
    FLASH_NCR2 = (uint8_t) ~(1 << FLASH_NCR2_NPRG);

    /* write data from buffer */
    while (buf < end)
        *mem++ = *buf++;

    /* wait for operation to complete */
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_EOP)));
}
