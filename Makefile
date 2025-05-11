all:
	@echo "make all is not currently supported."

image:
	make -C umbra boot
	cp umbra/boot umbra.img
	#printf '\125\252' | dd of=./umbra.img bs=1 seek=510 conv=notrunc

run: image
	qemu-system-x86_64 -drive format=raw,file=umbra.img

clean:
	make -C umbra clean
	rm -f umbra.img

