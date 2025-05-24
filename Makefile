all:
	@echo "make all is not currently supported."

build: clean 
	make -C umbra/bootloader bootloader
	cp umbra/bootloader/umbra.img umbra.img

run: build
	qemu-system-x86_64 -drive format=raw,file=umbra.img -no-reboot

clean:
	make -C umbra/bootloader clean
	rm -f umbra.img
