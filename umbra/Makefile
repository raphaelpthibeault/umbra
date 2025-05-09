ARCH=x86_64

diskboot: ASFLAGS=-nostdlib -I./bootloader/arch/$(ARCH)/

## MBR
diskboot: bootloader/arch/$(ARCH)/diskboot.o
	$(LD) -N -T bootloader/arch/$(ARCH)/linker.ld -o $@ $^

clean:
	find . -name "*.o" | xargs rm -f
	rm -f diskboot

