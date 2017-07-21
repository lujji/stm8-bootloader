#include <stdint.h>
#include "stm8s.h"
#include "ram.h"

#define UART_DIV ((F_CPU + BAUDRATE / 2) / BAUDRATE)

#define DBG_PIN  4
inline void DBG_INIT() {
    PD_DDR |= 1 << DBG_PIN;
    PD_CR1 |= 1 << DBG_PIN;
}
#define SET() do { PD_ODR |= (1 << DBG_PIN);  } while (0);
#define CLR() do { PD_ODR &= ~(1 << DBG_PIN); } while (0);
#define TOG() do { PD_ODR ^= (1 << DBG_PIN);  } while (0);

static uint8_t rx_buffer[RX_BUFFER_LEN];
static uint8_t ivt[128];
static uint8_t f_ram[128];
static void (*flash_write_block_ram)(uint16_t addr, const uint8_t *buf);

void dummy_isr() __interrupt(29) __naked { ; }

static uint8_t get_section_length()
{
    volatile int len;
    __asm
        ldw x, #l_RAM_SEG
        ldw (0x01, sp), x
    __endasm;
    return len;
}

inline void iwdg_init() {
    /* enable IWDG */
    IWDG_KR = IWDG_KEY_ENABLE;

    /* enable write access */
    //IWDG_KR = IWDG_KEY_ACCESS;

    /* prescaler = 64 */
    //IWDG_PR = 4;

    /* timeout = 250ms */
    //IWDG_RLR = 0xFA;

    /* reload watchdog counter */
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

static void serial_send_ack() {
    uart_write(0xAA);
    uart_write(0xBB);
}

inline void bootloader_exec() {
    uint8_t chunks;
    uint16_t addr = BOOT_ADDR;
    uint8_t *ivt_ptr = ivt;

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
        break;
    }

    /* unlock flash */
    FLASH_PUKR = FLASH_PUKR_KEY1;
    FLASH_PUKR = FLASH_PUKR_KEY2;
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_PUL)));

    /* get application interrupt table */
    for (uint8_t i = 0; i < 2; i++) {
        serial_send_ack();
        for (uint8_t j = 0; j < BLOCK_SIZE; j++)
            *ivt_ptr++ = uart_read();
    }

    /* get main firmware */
    for (uint8_t i = 2; i < chunks; i++) {
        serial_send_ack();
        for (uint8_t j = 0; j < BLOCK_SIZE; j++)
            rx_buffer[j] = uart_read();
        flash_write_block_ram(addr, rx_buffer);
        addr += BLOCK_SIZE;
    }

    /* verify CRC */
    /// @TODO

    /* copy application interrupt vector table */
    for (uint8_t i = 4; i < 2 * BLOCK_SIZE; i++) {
        _MEM_(0x8000 + i) = ivt[i];
        iwdg_refresh();
        while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_EOP)));
    }

    /* lock flash */
    FLASH_IAPSR &= ~(1 << FLASH_IAPSR_PUL);

    /* reboot */
    for(;;);
}

inline void ram_cpy() {
    uint8_t len = get_section_length();
    for (uint8_t i = 0; i < len; i++)
        f_ram[i] = ((uint8_t *) &flash_write_block)[i];
    flash_write_block_ram = (void (*)(uint16_t, const uint8_t *)) &f_ram;
}

void main() {
    BOOT_PIN_CR1 = 1 << BOOT_PIN;
    DBG_INIT();
    if (!(BOOT_PIN_IDR & (1 << BOOT_PIN))) {
        /* execute bootloader */
        SET();
        ram_cpy();
        iwdg_init();
        uart_init();
        bootloader_exec();
    } else {
        /* jump to application */
        BOOT_PIN_CR1 = 0x00;
        __asm__("jp 0x8400");
    }
}
