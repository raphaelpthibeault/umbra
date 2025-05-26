all:
	@echo "make all is not currently supported."

build: clean 
	make -C umbra/bootloader bootloader
	# make empty image
	dd if=/dev/zero bs=1M count=0 seek=64 of=umbra.hdd
	# format with sgdisk to create an mbr with "inactive" 1st partition
	PATH=$$PATH:/usr/sbin:/sbin sgdisk umbra.hdd -n 1:2048 -t 1:ef00 -m 1
	# install bootloader on raw image
	./umbra/bootloader/mkimage umbra.hdd

run: build
	qemu-system-x86_64 -drive format=raw,file=umbra.hdd -no-reboot

clean:
	make -C umbra/bootloader clean
	rm -f umbra.hdd
