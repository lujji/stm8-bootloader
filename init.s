.module INIT
.macro jump addr
    jp 0x8280 + addr
    .ds 1
.endm

.area IVT
int init ; reset
jump 0x4 ; trap
jump 0x8 ; int0
jump 0xc ; int1
jump 0x10 ; int2
jump 0x14 ; int3
jump 0x18 ; int4
jump 0x1c ; int5
jump 0x20 ; int6
jump 0x24 ; int7
jump 0x28 ; int8
jump 0x2c ; int9
jump 0x30 ; int10
jump 0x34 ; int11
jump 0x38 ; int12
jump 0x3c ; int13
jump 0x40 ; int14
jump 0x44 ; int15
jump 0x48 ; int16
jump 0x4c ; int17
jump 0x50 ; int18
jump 0x54 ; int19
jump 0x58 ; int20
jump 0x5c ; int21
jump 0x60 ; int22
jump 0x64 ; int23
jump 0x68 ; int24
jump 0x6c ; int25
jump 0x70 ; int26
jump 0x74 ; int27
jump 0x78 ; int28
jump 0x7c ; int29

.area SSEG
.area GSINIT
init:
    ldw x, #l_DATA
    jreq    00002$
00001$:
    clr (s_DATA - 1, x)
    decw x
    jrne    00001$
00002$:
    ldw x, #l_INITIALIZER
    jreq    00004$
00003$:
    ld  a, (s_INITIALIZER - 1, x)
    ld  (s_INITIALIZED - 1, x), a
    decw    x
    jrne    00003$
00004$:
    jp  _bootloader_main
