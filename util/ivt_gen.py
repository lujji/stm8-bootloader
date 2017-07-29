BOOT_ADDR = 0x8400

prologue = (
    '.module IVT',
    '.macro jump addr',
    '    jp %s + addr' % hex(BOOT_ADDR),
    '    .ds 1',
    '.endm',
    '',
    '.area IVT',
    'int init ; reset',
    'jump 0x4 ; trap',)

epilogue = (
    '',
    '.area GSINIT',
    'init:',
    '    ldw x, #l_DATA',
    '    jreq    00002$',
    '00001$:',
    '    clr (s_DATA - 1, x)',
    '    decw x',
    '    jrne    00001$',
    '00002$:',
    '    ldw x, #l_INITIALIZER',
    '    jreq    00004$',
    '00003$:',
    '    ld  a, (s_INITIALIZER - 1, x)',
    '    ld  (s_INITIALIZED - 1, x), a',
    '    decw    x',
    '    jrne    00003$',
    '00004$:',
    '    jp  _bootloader_main')

for i in prologue: print(i)
for i in range(30):
    print('jump %s ; int%d' % (hex(i * 4 + 8), i))
for i in epilogue: print(i)
