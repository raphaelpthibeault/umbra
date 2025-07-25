ARCH=x86_64

CC= $(ARCH)-elf-gcc

BOOT_ASFLAGS=-c -nostdlib -m32 -I./arch/$(ARCH)/
BOOT_CFLAGS=-c -ffreestanding -std=c23 -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -Wall -Wextra -ffreestanding -fno-strict-aliasing -O2 -g -I./ 
BOOT_LDFLAGS=-m32 -nostdlib -Wl,-m,elf_i386

STAGE2_ASM_SOURCES= arch/$(ARCH)/core.S arch/$(ARCH)/asm.S arch/$(ARCH)/int.S arch/$(ARCH)/isr_vector.S \
	arch/$(ARCH)/keypress.S arch/$(ARCH)/multiboot_reloc.S arch/$(ARCH)/multiboot_spinup.S arch/$(ARCH)/spinup.S 

STAGE2_C_SOURCES= arch/$(ARCH)/main.c drivers/vga.c arch/$(ARCH)/e820.c mm/pmm.c lib/misc.c arch/$(ARCH)/idt.c \
 arch/$(ARCH)/exceptions.c drivers/disk.c lib/partition.c lib/arithmetic64.c fs/fat32.c fs/file.c common/boot_menu.c \
 common/terminal.c drivers/vbe.c lib/framebuffer.c drivers/edid.c drivers/serial.c arch/$(ARCH)/kbd.c common/config.c \
 common/boot.c common/multiboot2.c common/uri.c common/acpi.c lib/elf.c common/relocation.c common/panic.c

STAGE2_ASM_OBJS := $(STAGE2_ASM_SOURCES:.S=.S.o)
STAGE2_C_OBJS := $(STAGE2_C_SOURCES:.c=.c.o)
STAGE2_OBJS := $(STAGE2_ASM_OBJS) $(STAGE2_C_OBJS)

%.S.o: %.S
	$(CC) $(BOOT_ASFLAGS) -o $@ $^

%.c.o: %.c
	$(CC) $(BOOT_CFLAGS) -o $@ $^


boot.hdd: arch/$(ARCH)/boot.S.o
	$(CC) $(BOOT_LDFLAGS) -N -T arch/$(ARCH)/boot.ld -o $@ $^

# start.hdd (the first sector of 512 bytes of the core.hdd, must be linked sepatately)
start.hdd: arch/$(ARCH)/start.S.o
	$(CC) $(BOOT_LDFLAGS) -N -T arch/$(ARCH)/start.ld -o $@ $^

# stage2.hdd (core.hdd = start.hdd + stage2.hdd)
stage2.hdd: $(STAGE2_OBJS)
	$(CC) $(BOOT_LDFLAGS) -N -T arch/$(ARCH)/core.ld -o $@ $^ #-Wl,-Map,umbra.map

mkimage: host/mkimage.c
	# use host compiler not cross-compiler
	gcc -o $@ $^

bootloader: mkimage boot.hdd start.hdd stage2.hdd 
	# prepend start.hdd to core.hdd
	cat start.hdd stage2.hdd > core.hdd

clean:
	find . -name "*.o" | xargs rm -f
	rm -f mkimage boot.hdd start.hdd stage2.hdd core.hdd bootloader.hdd 

