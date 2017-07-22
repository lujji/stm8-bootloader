#if RELOCATE_IVT
void trap() __trap __naked { __asm__("jp " BOOT_ADDR_S "+0x4" ); }
void isr0() __interrupt(0) __naked { __asm__("jp " BOOT_ADDR_S "+0x8"); }
void isr1() __interrupt(1) __naked { __asm__("jp " BOOT_ADDR_S "+0xc"); }
void isr2() __interrupt(2) __naked { __asm__("jp " BOOT_ADDR_S "+0x10"); }
void isr3() __interrupt(3) __naked { __asm__("jp " BOOT_ADDR_S "+0x14"); }
void isr4() __interrupt(4) __naked { __asm__("jp " BOOT_ADDR_S "+0x18"); }
void isr5() __interrupt(5) __naked { __asm__("jp " BOOT_ADDR_S "+0x1c"); }
void isr6() __interrupt(6) __naked { __asm__("jp " BOOT_ADDR_S "+0x20"); }
void isr7() __interrupt(7) __naked { __asm__("jp " BOOT_ADDR_S "+0x24"); }
void isr8() __interrupt(8) __naked { __asm__("jp " BOOT_ADDR_S "+0x28"); }
void isr9() __interrupt(9) __naked { __asm__("jp " BOOT_ADDR_S "+0x2c"); }
void isr10() __interrupt(10) __naked { __asm__("jp " BOOT_ADDR_S "+0x30"); }
void isr11() __interrupt(11) __naked { __asm__("jp " BOOT_ADDR_S "+0x34"); }
void isr12() __interrupt(12) __naked { __asm__("jp " BOOT_ADDR_S "+0x38"); }
void isr13() __interrupt(13) __naked { __asm__("jp " BOOT_ADDR_S "+0x3c"); }
void isr14() __interrupt(14) __naked { __asm__("jp " BOOT_ADDR_S "+0x40"); }
void isr15() __interrupt(15) __naked { __asm__("jp " BOOT_ADDR_S "+0x44"); }
void isr16() __interrupt(16) __naked { __asm__("jp " BOOT_ADDR_S "+0x48"); }
void isr17() __interrupt(17) __naked { __asm__("jp " BOOT_ADDR_S "+0x4c"); }
void isr18() __interrupt(18) __naked { __asm__("jp " BOOT_ADDR_S "+0x50"); }
void isr19() __interrupt(19) __naked { __asm__("jp " BOOT_ADDR_S "+0x54"); }
void isr20() __interrupt(20) __naked { __asm__("jp " BOOT_ADDR_S "+0x58"); }
void isr21() __interrupt(21) __naked { __asm__("jp " BOOT_ADDR_S "+0x5c"); }
void isr22() __interrupt(22) __naked { __asm__("jp " BOOT_ADDR_S "+0x60"); }
void isr23() __interrupt(23) __naked { __asm__("jp " BOOT_ADDR_S "+0x64"); }
void isr24() __interrupt(24) __naked { __asm__("jp " BOOT_ADDR_S "+0x68"); }
void isr25() __interrupt(25) __naked { __asm__("jp " BOOT_ADDR_S "+0x6c"); }
void isr26() __interrupt(26) __naked { __asm__("jp " BOOT_ADDR_S "+0x70"); }
void isr27() __interrupt(27) __naked { __asm__("jp " BOOT_ADDR_S "+0x74"); }
void isr28() __interrupt(28) __naked { __asm__("jp " BOOT_ADDR_S "+0x78"); }
void isr29() __interrupt(29) __naked { __asm__("jp " BOOT_ADDR_S "+0x7C"); }
#else
void isr29() __interrupt(29) __naked { ; }
#endif
