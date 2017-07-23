;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.6.8 #9951 (Linux)
;--------------------------------------------------------
	.module ramx
	.optsdcc -mstm8
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _ram_do_stuff
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area DATA
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area INITIALIZED
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area HOME
	.area GSINIT
	.area GSFINAL
	.area GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area HOME
	.area HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area CODE
;	ramx.c: 4: static void x() {
;	-----------------------------------------
;	 function x
;	-----------------------------------------
_x:
;	ramx.c: 5: __asm__(".area NEW");
	.area	NEW
	ret
;	ramx.c: 8: void ram_do_stuff(uint16_t addr, const uint8_t *buf) {
;	-----------------------------------------
;	 function ram_do_stuff
;	-----------------------------------------
_ram_do_stuff:
;	ramx.c: 10: FLASH_CR2 = 1 << FLASH_CR2_PRG;
	mov	0x505b+0, #0x01
;	ramx.c: 11: FLASH_NCR2 = (uint8_t) ~(1 << FLASH_NCR2_NPRG);
	mov	0x505c+0, #0xfe
;	ramx.c: 18: while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_EOP)));
00101$:
	ld	a, 0x505f
	bcp	a, #0x04
	jreq	00101$
	ret
	.area CODE
	.area INITIALIZER
	.area CABS (ABS)
