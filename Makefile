export PATH := $(PATH):$(HOME)/local/sdcc/bin

MCU  = stm8s003f3
ARCH = stm8

F_CPU   ?= 2000000
TARGET  ?= main.ihx

#LIBDIR   = ../../stm8

SRCS    := $(wildcard *.c $(LIBDIR)/*.c)
#SRCS	:= $(filter-out ram.c, $(SRCS))
OBJS     = $(SRCS:.c=.rel)

CC       = sdcc
LD       = sdld
OBJCOPY  = sdobjcopy
CFLAGS   = -m$(ARCH) -p$(MCU) --std-sdcc11
CFLAGS  += -DF_CPU=$(F_CPU)UL -I.
CFLAGS  += --stack-auto --noinduction --use-non-free --opt-code-size
LDFLAGS  = -m$(ARCH) -l$(ARCH) --out-fmt-ihx

all: $(TARGET) size

# $(TARGET): $(OBJS)
# 	$(CC) $(CFLAGS) $(INCLUDE) --codeseg RAM_SEG -c ram.c -o ram.rel
# 	$(CC) $(LDFLAGS) $(OBJS) ram.rel -o $@

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.rel: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

flash: $(TARGET)
	stm8flash -c stlinkv2 -p $(MCU) -w $(TARGET)

clean:
	rm -f *.map *.asm *.rel *.ihx *.o *.sym *.lk *.lst *.rst *.cdb *.bin

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
