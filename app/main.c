#include <stdint.h>
#include "../stm8s.h"

#define LED_PIN     4

volatile uint16_t ctr = 0;

void dummy_isr() __interrupt(29) __naked { ; }

void blink() {
    PD_ODR ^= (1 << LED_PIN);
}
void timer_isr() __interrupt(TIM4_ISR) {
    if (++ctr >= 64) {
        blink();
        ctr = 0;
    }
    TIM4_SR &= ~(1 << TIM4_SR_UIF);
}

inline void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ((F_CPU / 18 / 1000UL) * ms); i++) {
        __asm__("nop");
    }
}

inline void timer_config() {
    /* Prescaler = 128 */
    TIM4_PSCR = 0b00000111;

    /* Frequency = F_CLK / (2 * prescaler * (1 + ARR))
     *           = 2 MHz / (2 * 128 * (1 + 77)) = 100 Hz */
    TIM4_ARR = 77;

    TIM4_IER |= (1 << TIM4_IER_UIE); // Enable Update Interrupt
    TIM4_CR1 |= (1 << TIM4_CR1_CEN); // Enable TIM4
}

void main() {
    PD_DDR |= (1 << LED_PIN);
    PD_CR1 |= (1 << LED_PIN);

    enable_interrupts();
    timer_config();

    while (1) {
//         PD_ODR ^= (1 << LED_PIN);
//         delay_ms(250);
    }
}
