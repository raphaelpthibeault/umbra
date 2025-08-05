ARCH=x86_64

CC= $(ARCH)-elf-gcc

KERNEL_ASFLAGS 	= -c -nostdlib -I./arch/$(ARCH)/ -I./
KERNEL_CFLAGS  	= -c -ffreestanding -std=c23 -fno-pie -fno-stack-protector -nostdlib -Wall -Wextra -ffreestanding -fno-strict-aliasing -O2 -g -I./
KERNEL_LD_FLAGS	= -nostdlib

ifeq ($(ARCH),x86_64)
    override KERNEL_CFLAGS += \
        -m64 \
        -march=x86-64 \
        -mno-80387 \
        -mno-mmx \
        -mno-sse \
        -mno-sse2 \
        -mno-red-zone \
        -mcmodel=kernel
    override KERNEL_LD_FLAGS += \
        -m64
    override KERNEL_ASFLAGS += \
        -m64
endif

KERNEL_ASM_SOURCES= arch/$(ARCH)/bootstrap.S arch/$(ARCH)/isr.S
KERNEL_C_SOURCES	= arch/$(ARCH)/main.c drivers/serial.c arch/$(ARCH)/idt.c arch/$(ARCH)/isr.c  arch/$(ARCH)/mmu.c arch/$(ARCH)/paging.c

KERNEL_ASM_OBJS := $(KERNEL_ASM_SOURCES:.S=.S.o)
KERNEL_C_OBJS := $(KERNEL_C_SOURCES:.c=.c.o)
KERNEL_OBJS := $(KERNEL_ASM_OBJS) $(KERNEL_C_OBJS)

%.S.o: %.S
	$(CC) $(KERNEL_ASFLAGS) -o $@ $^

%.c.o: %.c
	$(CC) $(KERNEL_CFLAGS) -o $@ $^

kernel: $(KERNEL_OBJS)
	$(CC) $(KERNEL_LD_FLAGS) -N -T arch/$(ARCH)/link.ld -o $@ $^ #-Wl,-Map,umbra.map

clean:
	find . -name "*.o" | xargs rm -f
	rm -f kernel
