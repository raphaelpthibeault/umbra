ARCH=x86_64

LD= $(ARCH)-elf-ld
CC= $(ARCH)-elf-gcc
OBJCOPY= $(ARCH)-elf-objcopy

BOOT_ASFLAGS=-c -nostdlib -m32 -I./arch/$(ARCH)/
BOOT_CFLAGS=-c -ffreestanding -std=c23 -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -Wall -Wextra -O2 -g -I./ 
BOOT_LDFLAGS=-m elf_i386 

STAGE2_ASM_SOURCES= arch/$(ARCH)/core.S arch/$(ARCH)/asm.S arch/$(ARCH)/int.S arch/$(ARCH)/isr_vector.S
STAGE2_C_SOURCES= arch/$(ARCH)/main.c drivers/vga.c arch/$(ARCH)/e820.c mm/pmm.c lib/misc.c arch/$(ARCH)/idt.c \
									  arch/$(ARCH)/exceptions.c drivers/disk.c

STAGE3_ASM_SOURCES= arch/$(ARCH)/boot_menu.S
STAGE3_C_SOURCES= common/boot_menu.c


STAGE2_ASM_OBJS := $(STAGE2_ASM_SOURCES:.S=.o)
STAGE2_C_OBJS := $(STAGE2_C_SOURCES:.c=.o)
STAGE2_OBJS := $(STAGE2_ASM_OBJS) $(STAGE2_C_OBJS)

STAGE3_ASM_OBJS := $(STAGE3_ASM_SOURCES:.S=.o)
STAGE3_C_OBJS := $(STAGE3_C_SOURCES:.c=.o)
STAGE3_OBJS := $(STAGE3_ASM_OBJS) $(STAGE3_C_OBJS)

%.o: %.S
	$(CC) $(BOOT_ASFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(BOOT_CFLAGS) -o $@ $^


boot.hdd: arch/$(ARCH)/boot.o
	$(LD) $(BOOT_LDFLAGS) -N -T arch/$(ARCH)/boot.ld -o $@ $^

# start.hdd (the first sector of 512 bytes of the core.hdd, must be linked sepatately)
start.hdd: arch/$(ARCH)/start.o
	$(LD) $(BOOT_LDFLAGS) -N -T arch/$(ARCH)/start.ld -o $@ $^

# stage2.hdd (core.hdd = start.hdd + stage2.hdd)
stage2.hdd: $(STAGE2_OBJS)
	$(LD) $(BOOT_LDFLAGS) -N -T arch/$(ARCH)/core.ld -o $@ $^

# the boot menu
stage3.sys: $(STAGE3_OBJS)
	$(LD) $(BOOT_LDFLAGS) -N -T arch/$(ARCH)/stage3.ld -o $@ $^

mkimage: host/mkimage.c
	# use host compiler not cross-compiler
	gcc -o $@ $^

bootloader: mkimage boot.hdd start.hdd stage2.hdd stage3.sys
	# prepend start.hdd to core.hdd
	cat start.hdd stage2.hdd > core.hdd

clean:
	find . -name "*.o" | xargs rm -f
	rm -f mkimage boot.hdd start.hdd stage2.hdd core.hdd bootloader.hdd stage3.sys

