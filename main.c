#include <stdint.h>
#include "config.h"
#include "stm8.h"
#include "ram.h"

static uint8_t CRC = 0x00;
static uint8_t ivt[128];
static uint8_t f_ram1[128];
static uint8_t f_ram2[128];
static uint8_t rx_buffer[BLOCK_SIZE];
static volatile uint8_t RAM_SEG_LEN;
static void (*flash_write_block)(uint16_t addr, const uint8_t *buf) =
        (void (*)(uint16_t, const uint8_t *)) f_ram1;
static void (*write_rop_byte)(uint16_t addr, const uint8_t opt) =
        (void (*)(uint16_t, const uint8_t)) f_ram1;

/**
 * Write RAM_SEG section length into RAM_SEG_LEN
 */
inline void get_ram_section_length() {
    __asm__("mov _RAM_SEG_LEN, #l_RAM_SEG");
}

/**
 * Bytes / word union
 */
typedef union {
    uint16_t word;
    uint8_t bytes[2];
} word_union;

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
    for (uint8_t i = 0; i < 8; ++i)
        crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1;
    return crc;
}

/**
 *
 */
inline uint8_t calculate_crc(uint8_t *bts, uint8_t len) {
    uint8_t crc = 0xff;
    for (uint8_t i = 0; i < len; ++i) {
        crc = crc8_update(bts[i], crc);
    }
    return crc;
}

/**
 * Send ACK response
 */
static void serial_send_ack() {
    uart_write(0x79);
}

/**
 * Send NACK response (CRC mismatch)
 */
inline void serial_send_nack() {
    uart_write(0x1f);
}

/**
 * Read BLOCK_SIZE bytes from UART
 *
 * @param dest destination buffer
 */
static uint8_t serial_read_block(uint8_t *dest) {
    for (uint8_t i = 0; i < BLOCK_SIZE; ++i) {
        uint8_t rx = uart_read();
        dest[i] = rx;
    }
    uint8_t crc = uart_read();
    uint8_t crc2 = calculate_crc(dest, BLOCK_SIZE);
    if (crc != crc2)
        return 1;
    return 0;
}


/**
 * Read byte from uart and acknowledge
 */
inline uint8_t uart_read_byte() {
    uint8_t bts;
    for (;;) {
        bts = uart_read();
        uint8_t crc = uart_read();
        if (crc != calculate_crc(&bts, 1)) {
            serial_send_nack();
            continue;
        } else {
            serial_send_ack();
            break;
        }
    }
    return bts;
}

/**
 * Perform firmware update
 */
inline void write_memory() {
    uint16_t addr = BOOT_ADDR;
    uint8_t chunks = uart_read_byte();

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
    for (uint8_t i = 0; i < chunks; ++i) {
        uint8_t state = serial_read_block(rx_buffer);
        /* stop if error */
        if (state) {
            serial_send_nack();
            //uart_write(state);
            for (;;) ;
        }
        flash_write_block(addr, rx_buffer);
        serial_send_ack();
        addr += BLOCK_SIZE;
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
}

/**
 * Read single byte from flash
 */
uint8_t read_flash(uint16_t address) {
    return *((volatile uint8_t*) address);
}

/**
 * Read memory command: return requested memory content from address
 */
inline uint8_t read_memory() {
    word_union address;
    address.bytes[0] = uart_read();
    address.bytes[1] = uart_read();
    uint8_t crc = uart_read();

    if (crc != calculate_crc(&address.bytes[0], 2)) {
        serial_send_nack();
        return 1;
    } else {
        serial_send_ack();
    }

    uint8_t size = uart_read();
    crc = uart_read();
    if (crc != calculate_crc(&size, 1)) {
        serial_send_nack();
        return 2;
    } else {
        serial_send_ack();
    }

    crc = 0xff;
    for (uint16_t i = 0; i <= (uint16_t)size; ++i) {
        uint8_t mem = read_flash(address.word + i);
        crc = crc8_update(mem, crc);
        uart_write(mem);
    }
    uart_write(crc);
    return 0;
}

/**
 *
 */
inline void set_rop_opt_byte() {
    // get option byte to set over UART
    uint8_t opt = uart_read();
    uint8_t crc = uart_read();
    if (crc != calculate_crc(&opt, 1)) {
        serial_send_nack();
        return;
    } else {
        serial_send_ack();
    }

    /* unlock flash */
    FLASH_PUKR = FLASH_PUKR_KEY1;
    FLASH_PUKR = FLASH_PUKR_KEY2;
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_PUL)));

    write_rop_byte(0x4800, opt);

    /* lock flash */
    FLASH_IAPSR &= ~(1 << FLASH_IAPSR_PUL);

    uint8_t check = read_flash(0x4800);
    if (opt != check) {
        serial_send_nack();
    } else {
        serial_send_ack();
    }
}

/**
 *
 */
inline void read_rop_opt_byte() {
    // get option byte to set over UART
    uint8_t length = uart_read();
    uint8_t crc = uart_read();
    if (crc != calculate_crc(&length, 1)) {
        serial_send_nack();
        return;
    } else {
        serial_send_ack();
    }

    crc = 0xff;
    for (uint16_t i = 0; i <= (uint16_t)length; ++i) {
        uint16_t address = 0x4800 + i;
        uint8_t mem = read_flash(address);
        crc = crc8_update(mem, crc);
        uart_write(mem);
    }
    uart_write(crc);
}

/**
 * Decide wich function to call
 */
inline void bootloader_enter() {
    /* wait for init command */
    for (;;) {
        if (uart_read_byte() == 0x7f)
            break;
    }

    /* wait for command */
    uint8_t cmd;
    for (;;) {
        cmd = uart_read();
        uint8_t crc = uart_read();
        if (crc != calculate_crc(&cmd, 1)) {
            serial_send_nack();
            continue;
        } else {
            break;
        }
    }

    if (cmd == 0x10) {
        serial_send_ack();
        write_memory();
    } else if (cmd == 0x11) {
        // check if ROP is set
        uint8_t rop = read_flash(0x4800);
        if (rop == 0xaa) {
            serial_send_nack();
        } else {
            serial_send_ack();
            read_memory();
        }
    } else if (cmd == 0x12) {
        serial_send_ack();
        set_rop_opt_byte();
    } else if (cmd == 0x13) {
        serial_send_ack();
        read_rop_opt_byte();
    }

    /* stop */
    for (;;) ;
}

/**
 * Copy ram_flash_write_block routine into RAM
 */
inline void ram_cpy() {
    get_ram_section_length();
    for (uint8_t i = 0; i < RAM_SEG_LEN; ++i)
        f_ram1[i] = ((uint8_t *) ram_flash_write_block)[i];
    for (uint8_t i = 0; i < RAM_SEG_LEN; ++i)
        f_ram2[i] = ((uint8_t *) ram_write_rop_byte)[i];
}

void bootloader_main() {
    BOOT_PIN_CR1 = 1 << BOOT_PIN;
    if (!(BOOT_PIN_IDR & (1 << BOOT_PIN))) {
        /* execute bootloader */
        CLK_CKDIVR = 0;
        ram_cpy();
        uart_init();
        bootloader_enter();
    } else {
        /* jump to application */
        BOOT_PIN_CR1 = 0x00;
        BOOT();
    }
}
