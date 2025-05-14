all:
	@echo "make all is not currently supported."

mkimage: umbra/util/mkimage.c
	# use host compiler not cross-compiler
	gcc -o $@ $^

build: clean mkimage
	make -C umbra boot.img start.img stage2.img
	cp umbra/boot.img boot.img
	cp umbra/start.img start.img
	cp umbra/stage2.img stage2.img
	# prepend start.img to core.img
	cat start.img stage2.img > core.img
	printf '\000' | dd of=./core.img bs=1 seek=10000 conv=notrunc
	./mkimage . boot.img core.img

run: build
	qemu-system-x86_64 -drive format=raw,file=umbra.img

clean:
	make -C umbra clean
	rm -f mkimage umbra.img boot.img core.img start.img stage2.img

