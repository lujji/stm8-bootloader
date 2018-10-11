#include <stdint.h>
#include "../stm8.h"

#define LED_PIN  4

void dummy_isr() __interrupt(29) __naked { ; }

void tim4_isr() __interrupt(TIM4_ISR) {
    static uint16_t ctr = 0;
    if (++ctr >= 64) {
        PD_ODR ^= (1 << LED_PIN);
        ctr = 0;
    }
    TIM4_SR &= ~(1 << TIM4_SR_UIF);
}

static void timer_config() {
#if STM8L
    CLK_PCKENR1 |= 1 << 2;
#endif
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
        /* interrupts do the job */
    }
}
