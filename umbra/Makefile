ARCH=x86_64

AS = $(ARCH)-elf-as
LD = $(ARCH)-elf-ld
CC = $(ARCH)-elf-gcc

ASFLAGS=-nostdlib -I./bootloader/arch/$(ARCH)/

bootloader/arch/$(ARCH)/boot.o: bootloader/arch/$(ARCH)/boot.S
	$(AS) $(ASFLAGS) -o $@ $

boot: bootloader/arch/$(ARCH)/boot.o
	$(LD) -N -T bootloader/arch/$(ARCH)/linker.ld -o $@ $^

clean:
	find . -name "*.o" | xargs rm -f
	rm -f boot

