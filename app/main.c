#include <stdint.h>
#include "../stm8s.h"

#define LED_PIN     4

inline void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ((F_CPU / 18 / 1000UL) * ms); i++) {
        __asm__("nop");
    }
}

void main() {
    PD_DDR |= (1 << LED_PIN);
    PD_CR1 |= (1 << LED_PIN);

    while (1) {
        PD_ODR ^= (1 << LED_PIN);
        delay_ms(250);
    }
}
