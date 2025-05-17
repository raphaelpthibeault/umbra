ARCH=x86_64

AS= $(ARCH)-elf-as
LD= $(ARCH)-elf-ld
CC= $(ARCH)-elf-gcc
OBJCOPY= $(ARCH)-elf-objcopy

ASFLAGS=-c -nostdlib -m32 -I./bootloader/arch/$(ARCH)/
#  32-bt code even if on a 64-bit machine; the kernel will jump to long mode, the bootloader will not
# -I ?
CFLAGS=-c -ffreestanding -std=c23 -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -Wall -Wextra -O2 -g -I./bootloader/
LDFLAGS=-m elf_i386 

STAGE2_ASM_SOURCES= bootloader/arch/$(ARCH)/core.S
STAGE2_C_SOURCES= bootloader/arch/$(ARCH)/main.c bootloader/arch/$(ARCH)/vga.c

STAGE2_ASM_OBJS := $(STAGE2_ASM_SOURCES:.S=.o)
STAGE2_C_OBJS := $(STAGE2_C_SOURCES:.c=.o)
STAGE2_OBJS := $(STAGE2_ASM_OBJS) $(STAGE2_C_OBJS)


%.o:%.S
	$(CC) $(ASFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $^


boot.img: bootloader/arch/$(ARCH)/boot.o
	$(LD) $(LDFLAGS) -N -T bootloader/arch/$(ARCH)/boot.ld -o $@ $^

# start.img (the first sector of 512 bytes of the core.img, must be linked sepatately)
start.img: bootloader/arch/$(ARCH)/start.o
	$(LD) $(LDFLAGS) -N -T bootloader/arch/$(ARCH)/start.ld -o $@ $^

# stage2.img (core.img = start.img + stage2.img)
stage2.img: $(STAGE2_OBJS)
	$(LD) $(LDFLAGS) -N -T bootloader/arch/$(ARCH)/core.ld -o $@ $^

clean:
	find . -name "*.o" | xargs rm -f
	rm -f boot.img start.img stage2.img

