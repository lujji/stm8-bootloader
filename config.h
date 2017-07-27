#ifndef CONFIG_H
#define CONFIG_H

#include "stm8s.h"

#define BAUDRATE        115200

/* application address */
#define BOOT_ADDR       0x8280

/*
 * 0 = overwrite vector table during update,
 * 1 = use default jump-table
 */
#define RELOCATE_IVT    1

/* flash block size */
#define BLOCK_SIZE      64

/* entry jumper */
#define BOOT_PIN        3
#define BOOT_PIN_IDR    PD_IDR
#define BOOT_PIN_CR1    PD_CR1

#define STR(x)          #x
#define STRX(x)         STR(x)
#define BOOT()          __asm__("jp " STRX(BOOT_ADDR))

#endif /* CONFIG_H */
