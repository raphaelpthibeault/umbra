all: build

build: clean umbra.cfg 
	make -C umbra/bootloader bootloader
	make -C umbra/kernel kernel
	# make empty image
	dd if=/dev/zero bs=1M count=0 seek=64 of=umbra.hdd
	# format with sgdisk to create an mbr with "inactive" 1st partition
	PATH=$$PATH:/usr/sbin:/sbin sgdisk umbra.hdd -n 1:2048 -t 1:ef00 -m 1
	# install bootloader on raw image
	./umbra/bootloader/mkimage umbra.hdd
	# format raw image past bootloader (of size 1M) as FAT32
	mformat -i umbra.hdd@@1M
	# add relevant directories 
	mmd -i umbra.hdd@@1M ::/boot ::/boot/bootloader
	# copy files to relevant directories
	mcopy -i umbra.hdd@@1M umbra.cfg ::/boot/bootloader
	mcopy -i umbra.hdd@@1M umbra/kernel/kernel ::/boot

run: build
	qemu-system-x86_64 -drive format=raw,file=umbra.hdd -no-reboot -M q35 -m 2G -serial mon:stdio

clean:
	make -C umbra/bootloader clean
	make -C umbra/kernel clean
	rm -f umbra.hdd
