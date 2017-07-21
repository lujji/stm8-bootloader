#include <stdint.h>
#include "stm8s.h"
#include "ram.h"
#include "ivt.h"
//#include "debug.h"

#define UART_DIV ((F_CPU + BAUDRATE / 2) / BAUDRATE)

static uint8_t CRC;
static uint8_t rx_buffer[RX_BUFFER_LEN];
static uint8_t ivt[128];
static uint8_t f_ram[128];
static void (*flash_write_block_ram)(uint16_t addr, const uint8_t *buf);

#pragma save
#pragma disable_warning 84
static uint8_t get_ram_section_length() {
    volatile uint16_t len;
    (void) len;
    __asm
        ldw x,#l_RAM_SEG
        ldw(0x01, sp), x
    __endasm;
    return len;
}
#pragma restore

/* prescaler = 32, timeout = 63.70ms */
inline void iwdg_init() {
    IWDG_KR = IWDG_KEY_ENABLE;
    IWDG_KR = IWDG_KEY_ACCESS;
    IWDG_PR = 2;
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

inline uint8_t crc8_update(uint8_t data, uint8_t crc) {
    crc ^= data;
    for (uint8_t i = 0; i < 8; i++)
        crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1;
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

static void serial_read_block(uint8_t *dest) {
    const uint8_t *end = dest + BLOCK_SIZE;
    serial_send_ack();
    while (dest < end) {
        uint8_t rx =  uart_read();
        *dest++ = rx;
        CRC = crc8_update(rx, CRC);
    }
}

inline void bootloader_exec() {
    uint8_t chunks, crc_rx;
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

#if !RELOCATE_IVT
    /* get application interrupt table */
    serial_read_block(ivt);
    serial_read_block(ivt + BLOCK_SIZE);
    chunks -= 2;
#endif

    /* unlock flash */
    FLASH_PUKR = FLASH_PUKR_KEY1;
    FLASH_PUKR = FLASH_PUKR_KEY2;
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_PUL)));

    /* get main firmware */
    for (uint8_t i = 0; i < chunks; i++) {
        serial_read_block(rx_buffer);
        flash_write_block_ram(addr, rx_buffer);
        addr += BLOCK_SIZE;
    }

    /* verify CRC */
    if (CRC != crc_rx) {
        serial_send_nack();
        for (;;);
    }

#if !RELOCATE_IVT
    /* copy application interrupt vector table */
    *(uint32_t *) ivt = *(uint32_t *) (0x8000);
    flash_write_block_ram(0x8000, ivt);
    flash_write_block_ram(0x8000 + BLOCK_SIZE, ivt + BLOCK_SIZE);
#endif

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

// size: 750 -> 744 -> 738 -> 729 -> 721 -> 658
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
