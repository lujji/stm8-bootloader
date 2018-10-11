#include <stdint.h>
#include "config.h"
#include "stm8.h"
#include "ram.h"

static uint8_t CRC;
static uint8_t ivt[128];
static uint8_t f_ram[128];
static uint8_t rx_buffer[BLOCK_SIZE];
static volatile uint8_t RAM_SEG_LEN;
static void (*flash_write_block)(uint16_t addr, const uint8_t *buf) =
        (void (*)(uint16_t, const uint8_t *)) f_ram;

/**
 * Write RAM_SEG section length into RAM_SEG_LEN
 */
inline void get_ram_section_length() {
    __asm__("mov _RAM_SEG_LEN, #l_RAM_SEG");
}

/**
 * Initialize watchdog:
 * prescaler = 32, timeout = 63.70ms
 */
inline void iwdg_init() {
    IWDG_KR = IWDG_KEY_ENABLE;
    IWDG_KR = IWDG_KEY_ACCESS;
    IWDG_PR = 2;
    IWDG_KR = IWDG_KEY_REFRESH;
}

/**
 * Kick the dog
 */
inline void iwdg_refresh() {
    IWDG_KR = IWDG_KEY_REFRESH;
}

/**
 * Initialize UART1 in 8N1 mode
 */
inline void uart_init() {
    /* enable UART clock (STM8L only)*/
    UART_CLK_ENABLE();
    /* madness.. */
    UART_BRR2 = ((UART_DIV >> 8) & 0xF0) + (UART_DIV & 0x0F);
    UART_BRR1 = UART_DIV >> 4;
    /* enable transmitter and receiver */
    UART_CR2 = (1 << UART_CR2_TEN) | (1 << UART_CR2_REN);
}

/**
 * Write byte into UART
 */
static void uart_write(uint8_t data) {
    UART_DR = data;
    while (!(UART_SR & (1 << UART_SR_TC)));
}

/**
 * Read byte from UART and reset watchdog
 */
static uint8_t uart_read() {
    iwdg_refresh();
    while (!(UART_SR & (1 << UART_SR_RXNE)));
    return UART_DR;
}

/**
 * Calculate CRC-8-CCIT.
 * Polynomial: x^8 + x^2 + x + 1 (0x07)
 *
 * @param data input byte
 * @param crc initial CRC
 * @return CRC value
 */
inline uint8_t crc8_update(uint8_t data, uint8_t crc) {
    crc ^= data;
    for (uint8_t i = 0; i < 8; i++)
        crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1;
    return crc;
}

/**
 * Send ACK response
 */
static void serial_send_ack() {
    uart_write(0xAA);
    uart_write(0xBB);
}

/**
 * Send NACK response (CRC mismatch)
 */
inline void serial_send_nack() {
    uart_write(0xDE);
    uart_write(0xAD);
}

/**
 * Read BLOCK_SIZE bytes from UART
 *
 * @param dest destination buffer
 */
static void serial_read_block(uint8_t *dest) {
    serial_send_ack();
    for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
        uint8_t rx = uart_read();
        dest[i] = rx;
        CRC = crc8_update(rx, CRC);
    }
}

/**
 * Enter bootloader and perform firmware update
 */
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
    chunks--;
    #if BLOCK_SIZE == 64
    chunks--;
    serial_read_block(ivt + BLOCK_SIZE);
    #endif
#endif

    /* unlock flash */
    FLASH_PUKR = FLASH_PUKR_KEY1;
    FLASH_PUKR = FLASH_PUKR_KEY2;
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_PUL)));

    /* get main firmware */
    for (uint8_t i = 0; i < chunks; i++) {
        serial_read_block(rx_buffer);
        flash_write_block(addr, rx_buffer);
        addr += BLOCK_SIZE;
    }

    /* verify CRC */
    if (CRC != crc_rx) {
        serial_send_nack();
        for (;;) ;
    }

#if !RELOCATE_IVT
    /* overwrite vector table preserving the reset interrupt */
    *(uint32_t *) ivt = *(uint32_t *) (0x8000);
    flash_write_block(0x8000, ivt);
    #if BLOCK_SIZE == 64
    flash_write_block(0x8000 + BLOCK_SIZE, ivt + BLOCK_SIZE);
    #endif
#endif

    /* lock flash */
    FLASH_IAPSR &= ~(1 << FLASH_IAPSR_PUL);

    serial_send_ack();

    /* reboot */
    for (;;) ;
}

/**
 * Copy ram_flash_write_block routine into RAM
 */
inline void ram_cpy() {
    get_ram_section_length();
    for (uint8_t i = 0; i < RAM_SEG_LEN; i++)
        f_ram[i] = ((uint8_t *) ram_flash_write_block)[i];
}

void bootloader_main() {
    BOOT_PIN_CR1 = 1 << BOOT_PIN;
    if (!(BOOT_PIN_IDR & (1 << BOOT_PIN))) {
        /* execute bootloader */
        CLK_CKDIVR = 0;
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
