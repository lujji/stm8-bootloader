#include <stdint.h>
#include <stm8s.h>
#include "ram.h"

#define BAUDRATE        115200
#define UART_DIV        ((F_CPU + BAUDRATE / 2) / BAUDRATE)

#define ADDR            (0x8000 + 0x400)
#define CHUNK_SIZE      64
#define RX_BUFFER_LEN   (128 + 16)
#define LED_PIN         4
//#define BOOT(addr)      ((boot*) addr)()
#define BOOT()          __asm__("jp 0x8400")

typedef void boot();

static uint8_t RX_BUFFER[RX_BUFFER_LEN];
static uint8_t f_ram[128];
static uint8_t chunks;
static void (*flash_write_block_RAM)(uint16_t addr, const uint8_t *buf);

inline void uart_init() {
    /* madness.. */
    UART1_BRR2 = ((UART_DIV >> 8) & 0xF0) + (UART_DIV & 0x0F);
    UART1_BRR1 = UART_DIV >> 4;
    /* enable transmitter and receiver */
    UART1_CR2 = (1 << UART1_CR2_TEN) | (1 << UART1_CR2_REN);
}

static void uart_write(uint8_t data) {
    UART1_DR = data;
    while (!(UART1_SR & (1 << UART1_SR_TC)));
}

static uint8_t uart_read() {
    while (!(UART1_SR & (1 << UART1_SR_RXNE)));
    return UART1_DR;
}

static void serial_read_buf(uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        while (!(UART1_SR & (1 << UART1_SR_RXNE)));
        RX_BUFFER[i] = uart_read();
    }
}

inline void serial_send_ack() {
    uart_write(0xAA);
    uart_write(0xBB);
}

static int get_section_length()
{
    volatile int length = 0;
    __asm
        ldw x, #l_RAM_SEG
        ldw (0x01, sp), x
    __endasm;
    return length;
}

inline void bootloader_enter() {
    uint8_t rx;
    for (;;) {
        rx = uart_read();
        if (rx != 0xDE) continue;
        rx = uart_read();
        if (rx != 0xAD) continue;
        rx = uart_read();
        if (rx != 0xBE) continue;
        rx = uart_read();
        if (rx != 0xEF) continue;
        chunks = uart_read();
        return;
    }
}

inline void bootloader_exec() {
    uint16_t addr = ADDR;
    uart_init();

    bootloader_enter();

    /* unlock flash */
    FLASH_PUKR = FLASH_PUKR_KEY1;
    FLASH_PUKR = FLASH_PUKR_KEY2;
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_PUL)));

    for (uint8_t i = 0; i < chunks; i++) {
        serial_send_ack();
        serial_read_buf(CHUNK_SIZE);
        flash_write_block_RAM(addr, RX_BUFFER);
        addr += CHUNK_SIZE;
    }

    /* copy application IVT */
    for (uint8_t i = 4; i < 128; i++) {
        _MEM_(0x8000 + i) = _MEM_(ADDR + i);
        while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_EOP)));
    }

    /* lock flash */
    FLASH_IAPSR &= ~(1 << FLASH_IAPSR_PUL);
}

inline void ram_cpy() {
    uint8_t len = get_section_length();
    for (uint8_t i = 0; i < len; i++)
        f_ram[i] = ((uint8_t *) &flash_write_block)[i];
    flash_write_block_RAM = (void (*)(uint16_t, const uint8_t *)) &f_ram;
}

// void timer_isr() __interrupt(TIM4_ISR) {
//     __asm__("jp 0x9064");
// }

void dummy_isr() __interrupt(29) __naked { ; }

void main() {
    ram_cpy();

    PD_DDR = (1 << LED_PIN);
    PD_CR1 = (1 << LED_PIN);

    bootloader_exec();

    PD_ODR = (1 << LED_PIN);

    /* jump to application */
    __asm__("jp 0x8400");
}
