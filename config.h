#ifndef CONFIG_H
#define CONFIG_H

#include "stm8.h"

/* UART configuration */
#define BAUDRATE            115200

#if defined(STM8L)
#define UART_SR             USART1_SR
#define UART_CLK_ENABLE()   do { CLK_PCKENR1 |= (1 << 5); } while(0)
#else
#define UART_SR             UART1_SR
#define UART_CLK_ENABLE()   { ; }
#endif

/* application address */
#define BOOT_ADDR           0x8280

/*
 * 0 = overwrite vector table during update,
 * 1 = use default jump-table
 */
#define RELOCATE_IVT        1

/* flash block size */
#define BLOCK_SIZE          64

/* entry jumper */
#define BOOT_PIN            3
#define BOOT_PIN_IDR        PD_IDR
#define BOOT_PIN_CR1        PD_CR1

/* internal RC oscillator, CKDIVR = 0 */
#define F_CPU               16000000UL

/* not configured by user */
#define STR(x)              #x
#define STRX(x)             STR(x)
#define BOOT()              __asm__("jp " STRX(BOOT_ADDR))
#define UART_DIV            ((F_CPU + BAUDRATE / 2) / BAUDRATE)
#define UART_SR_TXE         7
#define UART_SR_TC          6
#define UART_SR_RXNE        5
#define UART_DR             *(&UART_SR + 0x01)
#define UART_BRR1           *(&UART_SR + 0x02)
#define UART_BRR2           *(&UART_SR + 0x03)
#define UART_CR1            *(&UART_SR + 0x04)
#define UART_CR2            *(&UART_SR + 0x05)
#define UART_CR2_TEN        3
#define UART_CR2_REN        2

#endif /* CONFIG_H */
