all:
	@echo "make all is not currently supported."

image:
	make -C umbra boot
	cp umbra/boot umbra.img
	# pad so that the disk load code loads something (nothing, but it's something)
	printf '\000' | dd of=./umbra.img bs=1 seek=1474559 conv=notrunc

run: image
	qemu-system-x86_64 -drive format=raw,file=umbra.img

clean:
	make -C umbra clean
	rm -f umbra.img

