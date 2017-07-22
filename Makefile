export PATH := $(PATH):$(HOME)/local/sdcc/bin

MCU  = stm8s003f3
ARCH = stm8

#F_CPU   ?= 2000000
F_CPU   ?= 16000000
TARGET  ?= main.ihx

SRCS    := $(wildcard *.c)
ASRCS	:= ivt.asm

OBJS     = $(SRCS:.c=.rel)
OBJS	+= $(ASRCS:.asm=.rel)

CC       = sdcc
LD       = sdld
AS       = sdasstm8
OBJCOPY  = sdobjcopy
CFLAGS   = -m$(ARCH) -p$(MCU)
CFLAGS  += -DF_CPU=$(F_CPU)UL -I.
CFLAGS  += --stack-auto --noinduction --use-non-free --noinvariant
LDFLAGS  = -m$(ARCH) -l$(ARCH) --out-fmt-ihx
LDFLAGS += -Wl-bIVT=0x8000 -Wl-bGSINIT=0x8080
#-Wl-bCODE=0x8080
#-Wl-bGSINIT=0x8080

all: $(TARGET) size

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.rel: %.asm
	$(AS) -plosgff $(ASRCS)

%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(TARGET)
	stm8flash -c stlinkv2 -p $(MCU) -w $(TARGET)

clean:
	rm -f *.map *.rel *.ihx *.o *.sym *.lk *.lst *.rst *.cdb *.bin $(SRCS:.c=.asm)

size:
	@$(OBJCOPY) -I ihex --output-target=binary $(TARGET) $(TARGET).bin
	@echo "-----\nImage size:"
	@stat -L -c %s $(TARGET).bin

opt-dump:
	stm8flash -c stlinkv2 -p stm8s003f3 -s opt -r dump_opt.bin

mem-dump:
	stm8flash -c stlinkv2 -p stm8s003f3 -s flash -r dump_flash.bin

erase:
	tr '\000' '\377' < /dev/zero | dd of=empty.bin bs=8192 count=1
	stm8flash -c stlinkv2 -p stm8s003f3 -s flash -w empty.bin

.PHONY: clean all program
