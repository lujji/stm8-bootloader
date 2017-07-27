export PATH := $(PATH):$(HOME)/local/sdcc/bin

MCU  = stm8s003f3
ARCH = stm8

F_CPU   ?= 2000000
TARGET  ?= main.ihx

SRCS    := $(wildcard *.c $(LIBDIR)/*.c)
OBJS     = $(SRCS:.c=.rel)

CC       = sdcc
LD       = sdld
OBJCOPY  = sdobjcopy
CFLAGS   = -m$(ARCH) -p$(MCU)
CFLAGS  += -DF_CPU=$(F_CPU)UL -I. -I$(LIBDIR)
CFLAGS  += --stack-auto --noinduction --use-non-free
LDFLAGS  = -m$(ARCH) -l$(ARCH) --out-fmt-ihx
#LDFLAGS += --code-loc 0x8260
LDFLAGS += --code-loc 0x8280

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	@$(OBJCOPY) -I ihex --output-target=binary $(TARGET) firmware.bin

%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.map *.asm *.rel *.ihx *.o *.sym *.lk *.lst *.rst *.cdb *.bin

.PHONY: clean all program
