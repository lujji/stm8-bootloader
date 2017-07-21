#include <stdint.h>
#include "stm8s.h"
#include "ram.h"
#include "debug.h"
//#include "isr.h"

#define UART_DIV ((F_CPU + BAUDRATE / 2) / BAUDRATE)

static uint8_t rx_buffer[RX_BUFFER_LEN];
static uint8_t ivt[128];
static uint8_t f_ram[128];
static void (*flash_write_block_ram)(uint16_t addr, const uint8_t *buf);

void dummy_isr() __interrupt(29) __naked { ; }

static uint8_t get_ram_section_length() {
    volatile uint16_t len;
    __asm
        ldw x,#l_RAM_SEG
        ldw(0x01, sp), x
    __endasm;
    return len;
}

inline void iwdg_init() {
    IWDG_KR = IWDG_KEY_ENABLE;
    IWDG_KR = IWDG_KEY_REFRESH;
}

inline void iwdg_refresh() {
    IWDG_KR = IWDG_KEY_REFRESH;
}

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
    iwdg_refresh();
    while (!(UART1_SR & (1 << UART1_SR_RXNE)));
    return UART1_DR;
}

uint8_t crc8_update(uint8_t data, uint8_t crc) {
    crc ^= data;
    for (uint8_t i = 0; i < 8; i++) {
        iwdg_refresh();
        crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1;
    }
    return crc;
}

uint8_t crc8(const uint8_t *src, uint8_t len, uint8_t crc) {
    const uint8_t *end = src + len;
    while (src < end)
        crc = crc8_update(*src++, crc);
    return crc;
}

inline uint8_t get_crc8(uint8_t chunks) {
    uint8_t crc = 0;
    crc = crc8(ivt, 128, crc);
    crc = crc8((uint8_t *) BOOT_ADDR, BLOCK_SIZE * chunks, crc);
    return crc;
}

static void serial_send_ack() {
    uart_write(0xAA);
    uart_write(0xBB);
}

inline void serial_send_nack() {
    uart_write(0xDE);
    uart_write(0xAD);
}

void serial_read_block(uint8_t *dest) {
    const uint8_t *end = dest + BLOCK_SIZE;
    serial_send_ack();
    while (dest < end)
        *dest++ = uart_read();
}

inline void bootloader_exec() {
    uint8_t i;
    uint8_t chunks, crc_rx, crc;
    uint8_t *ivt_ptr = ivt;
    uint16_t addr = BOOT_ADDR;

    /* enter bootloader */
    for (;;) {
        uint8_t rx = uart_read();
        if (rx != 0xDE) continue;
        rx = uart_read();
        if (rx != 0xAD) continue;
        rx = uart_read();
        if (rx != 0xBE) continue;
        rx = uart_read();
        if (rx != 0xEF) continue;
        chunks = uart_read();
        crc_rx = uart_read();
        rx = uart_read();
        if (crc_rx != rx)
            continue;
        break;
    }

    /* unlock flash */
    FLASH_PUKR = FLASH_PUKR_KEY1;
    FLASH_PUKR = FLASH_PUKR_KEY2;
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_PUL)));

    /* get application interrupt table */
    serial_read_block(ivt_ptr);
    serial_read_block(ivt_ptr + 64);

    /* get main firmware */
    for (i = 2; i < chunks; i++) {
        serial_read_block(rx_buffer);
        flash_write_block_ram(addr, rx_buffer);
        addr += BLOCK_SIZE;
    }

    /* verify CRC */
    crc = get_crc8(chunks - 2);
    if (crc != crc_rx) {
        serial_send_nack();
        for (;;);
    }

    /* copy application interrupt vector table */
    *(uint32_t *) ivt = *(uint32_t *) (0x8000);
    flash_write_block_ram(0x8000, ivt);
    flash_write_block_ram(0x8000 + 64, &ivt[64]);

    /* lock flash */
    FLASH_IAPSR &= ~(1 << FLASH_IAPSR_PUL);

    serial_send_ack();

    /* reboot */
    for (;;);
}

inline void ram_cpy() {
    uint8_t len = get_ram_section_length();
    for (uint8_t i = 0; i < len; i++)
        f_ram[i] = ((uint8_t *) &flash_write_block)[i];
    flash_write_block_ram = (void (*)(uint16_t, const uint8_t *)) &f_ram;
}

// size: 750 -> 744 -> 738 -> 729
void main() {
    BOOT_PIN_CR1 = 1 << BOOT_PIN;
    if (!(BOOT_PIN_IDR & (1 << BOOT_PIN))) {
        /* execute bootloader */
        ram_cpy();
        iwdg_init();
        uart_init();
        bootloader_exec();
    } else {
        /* jump to application */
        BOOT_PIN_CR1 = 0x00;
        BOOT();
    }
}
