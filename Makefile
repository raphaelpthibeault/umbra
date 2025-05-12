all:
	@echo "make all is not currently supported."

build:
	make -C umbra boot.img start.img
	cp umbra/boot.img umbra.img
	cp umbra/start.img	start.img
	dd if=start.img of=umbra.img bs=1 seek=512 conv=notrunc > /dev/null 2>&1
	# pad so that the disk load code loads something (nothing, but it's something)
	printf '\000' | dd of=./umbra.img bs=1 seek=1474559 conv=notrunc

run: build
	qemu-system-x86_64 -drive format=raw,file=umbra.img

clean:
	make -C umbra clean
	rm -f umbra.img start.img

