ARCH=x86_64

AS = $(ARCH)-elf-as
LD = $(ARCH)-elf-ld
CC = $(ARCH)-elf-gcc

ASFLAGS=-nostdlib -I./bootloader/arch/$(ARCH)/

# stage1 target (the boot sector of 512 bytes)
bootloader/arch/$(ARCH)/boot.o: bootloader/arch/$(ARCH)/boot.S
	$(AS) $(ASFLAGS) -o $@ $^

boot.img: bootloader/arch/$(ARCH)/boot.o
	$(LD) -N -T bootloader/arch/$(ARCH)/boot.ld -o $@ $^

# TO BE DONE AT A LATER DATE WHEN I HAVE THE CORE BOOTLOADER CODE:
#stage2 target (the start sector concatenated with the rest of the bootloader)
# start of stage2 (1 sector of 512 bytes)
# which should be concatenated with the core of the bootloader

# start.img
bootloader/arch/$(ARCH)/start.o: bootloader/arch/$(ARCH)/start.S
	$(AS) $(ASFLAGS) -o $@ $^

start.img: bootloader/arch/$(ARCH)/start.o
	$(LD) -N -T bootloader/arch/$(ARCH)/start.ld -o $@ $^


# stage2.img
bootloader/arch/$(ARCH)/asm.o: bootloader/arch/$(ARCH)/asm.S
	$(AS) $(ASFLAGS) -o $@ $^

stage2.img: bootloader/arch/$(ARCH)/asm.o
	$(LD) -N -T bootloader/arch/$(ARCH)/core.ld -o $@ $^

clean:
	find . -name "*.o" | xargs rm -f
	rm -f boot.img start.img stage2.img

