## STM8S
MCU     ?= stm8s003f3
FAMILY  ?= STM8S

## STM8L
# MCU     ?= stm8l051f3
# FAMILY  ?= STM8L

ARCH     = stm8

TARGET  ?= main.ihx

SRCS    := $(wildcard *.c)
ASRCS   := $(wildcard *.s)

OBJS     = $(SRCS:.c=.rel)
OBJS    += $(ASRCS:.s=.rel)

CC       = sdcc
LD       = sdld
AS       = sdasstm8
OBJCOPY  = sdobjcopy
ASFLAGS  = -plosgff
CFLAGS   = -m$(ARCH) -p$(MCU) -D$(FAMILY) -I.
CFLAGS  += --stack-auto --noinduction --use-non-free --noinvariant --opt-code-size
LDFLAGS  = -m$(ARCH) -l$(ARCH) --out-fmt-ihx
LDFLAGS += -Wl-bIVT=0x8000 -Wl-bGSINIT=0x8080

all: $(TARGET) size

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.rel: %.s
	$(AS) $(ASFLAGS) $<

%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(TARGET)
	stm8flash -c stlinkv2 -p $(MCU) -w $(TARGET)

clean:
	rm -f *.map *.rel *.ihx *.o *.sym *.lk *.lst *.rst *.cdb *.bin *.asm

size:
	@$(OBJCOPY) -I ihex --output-target=binary $(TARGET) $(TARGET).bin
	@echo "-----\nImage size:"
	@stat -L -c %s $(TARGET).bin

## @TODO: separate option-bytes for stm8s and stm8l!
# enable write-protection on first 10 pages
opt-set:
	@echo '0x00 0x09 0xf6 0x00 0xff 0x00 0xff 0x00 0xff 0x00 0xff' | xxd -r > opt.bin
	stm8flash -c stlinkv2 -p stm8s003f3 -s opt -w opt.bin

# reset option-bytes to factory defaults
opt-reset:
	@echo '0x00 0x00 0xff 0x00 0xff 0x00 0xff 0x00 0xff 0x00 0xff' | xxd -r > opt.bin
	stm8flash -c stlinkv2 -p stm8s003f3 -s opt -w opt.bin

dump-opt:
	stm8flash -c stlinkv2 -p $(MCU) -s opt -r dump_opt.bin

dump-flash:
	stm8flash -c stlinkv2 -p $(MCU) -s flash -r dump_flash.bin

erase:
	tr '\000' '\377' < /dev/zero | dd of=empty.bin bs=8192 count=1
	stm8flash -c stlinkv2 -p $(MCU) -s flash -w empty.bin

.PHONY: clean all flash size dump-opt dump-flash erase
