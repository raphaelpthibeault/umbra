
all:
	i686-elf-as boot.s -o boot.o
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu23 -ffreestanding -O2 -Wall -Wextra
	i686-elf-gcc -T linker.ld -o umbra.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
	mkdir -p isodir/boot/grub
	cp umbra.bin isodir/boot/umbra.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o umbra.iso isodir

run: all
	qemu-system-i386 -cdrom umbra.iso	

clean:
	rm -rf *.o umbra.bin isodir umbra.iso	
