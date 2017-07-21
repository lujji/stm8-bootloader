print ('void dummy_trap() __trap __naked { __asm__("jp " BOOT_ADDR_S "+0x4" ); }')
for i in xrange(29):
	print ('void dummy_isr%d() __interrupt(%d) __naked { __asm__("jp " BOOT_ADDR_S "+%s"); }' % (i, i, hex(i * 4 + 8)))
