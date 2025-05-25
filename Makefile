all:
	@echo "make all is not currently supported."

build: clean 
	make -C umbra/bootloader bootloader
	cp umbra/bootloader/bootloader.hdd umbra.hdd

run: build
	qemu-system-x86_64 -drive format=raw,file=umbra.hdd -no-reboot

clean:
	make -C umbra/bootloader clean
	rm -f umbra.hdd
