ARCH=x86_64

boot: ASFLAGS=-nostdlib -I./bootloader/arch/$(ARCH)/


boot: bootloader/arch/$(ARCH)/boot.o
	$(LD) -N -T bootloader/arch/$(ARCH)/linker.ld -o $@ $^

clean:
	find . -name "*.o" | xargs rm -f
	rm -f boot

