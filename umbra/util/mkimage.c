#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

#define DISK_SECTOR_SIZE 0x200		/* 512 bytes */
#define DISK_SECTOR_BITS 9
#define CORE_X86_ADDRESS 0x8000
#define CORE_X86_SEGMENT 0x800		/* real mode uses logical addresses 
																	 * A:B = (A * 0x10) + B */
#define UMBRA_SIZE_MAX 18446744073709551615UL

struct umbra_boot_blocklist {
	uint64_t start;
	uint16_t length;
	uint16_t segment;
} __attribute__((packed));


void
__attribute__((noreturn))
umbra_exit(int rc)
{
  exit(rc < 0 ? 1 : rc);
}

void
umbra_error(const char *fmt, ...)
{
	va_list ap;
	fprintf(stderr, "error: ");
	va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fprintf (stderr, ".\n");
	umbra_exit(1);
}

void
umbra_info(const char *fmt, ...)
{
	va_list ap;
	fprintf(stderr, "info: ");
	va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fprintf (stderr, ".\n");
}

void *
xmalloc(size_t size)
{
	void *p;

	p = malloc(size);
	if (p == NULL) {
		umbra_error("%s", "out of memory");
	}

	return p;
}

inline size_t
strlen(const char *s)
{
	const char *p = s;
	while (*p) {
		++p;
	}

	return p - s;
}

char *
get_path(const char *dir, const char *fname)
{
	char *path;
	
	path = xmalloc(strlen(dir) + 1 + strlen(fname) + 1);
	sprintf(path, "%s%s", dir, fname);

	return path;
}

size_t
umbra_get_image_size(const char *path)
{
	FILE *f;
	size_t ret;
	off_t sz;

	f = fopen(path, "rb");
	if (!f) {
		umbra_error("cannot open %s:%s", path, strerror(errno));
	}

	fseeko(f, 0, SEEK_END);

	sz = ftello(f);
	if (sz < 0) {
		umbra_error("cannot open %s:%s", path, strerror(errno));
	}
  if (sz > (off_t)(UMBRA_SIZE_MAX >> 1)) {
		umbra_error("file is too large  %s:%s", path, strerror(errno));
	}

	ret = (size_t)sz;
	fclose(f);
	return ret;
}

char *
umbra_read_image(const char *path)
{
	char *img;
	FILE *f;
	size_t size;

	umbra_info("reading '%s'", path);	

	size = umbra_get_image_size(path);
	img = xmalloc(size);

	f = fopen(path, "rb");
	if (!f) {
		umbra_error("cannot open %s:%s", path, strerror(errno));
	}

	if (fread(img, 1, size, f) != size) {
		umbra_error("cannot open %s:%s", path, strerror(errno));
	}

	fclose(f);

	return img;
}

void
umbra_write_image(const char *img, size_t size, FILE *out, const char *outname, const char *imgname)
{
	umbra_info("writing '%s' into '%s'", imgname, outname);	

	if (fwrite(img, 1, size, out) != size) {
		if (!outname) {
			umbra_error("cannot write to stdout: %s", outname, strerror(errno));
		} else {
			umbra_error("cannot write to %s:%s", outname, strerror(errno));
		}
	}
}

void
umbra_mkimage(const char *dir, const char *boot_file, const char *core_file)
{
	char *boot_img, *boot_path, *core_img, *core_path; // kernel_img, kernel_path
	size_t boot_size, core_size;
	uint16_t core_sectors;
	struct umbra_boot_blocklist *first_block;

	umbra_info("setting up umbra.img...");
	
	boot_path = get_path(dir, boot_file);
	boot_size = umbra_get_image_size(boot_path);
	if (boot_size != DISK_SECTOR_SIZE) {
		umbra_error("Boot sector '%s' is not %u", boot_size, DISK_SECTOR_SIZE);
	}

	boot_img = umbra_read_image(boot_path);
	free(boot_path);

	core_path = get_path(dir, core_file);
	core_size = umbra_get_image_size(core_path);
	if (core_size < DISK_SECTOR_SIZE) {
		umbra_error("Core '%s' is too small", core_path);
	}
	if (core_size > 0xFFFF * DISK_SECTOR_SIZE) {
		umbra_error("Core '%s' is too big", core_path);
	}
	
	core_img = umbra_read_image(core_path);
	free(core_path);

	core_sectors = ((core_size + DISK_SECTOR_SIZE -1) >> DISK_SECTOR_BITS);

	/* 
	 * use grub's idea of blocklists
	 * must have the blocklists allocated in start.S
	 * point first_block to the first blocklist
	 * */

	first_block = (struct umbra_boot_blocklist *)(core_img 
																								+ DISK_SECTOR_SIZE
																								- sizeof(*first_block));

	/* fill blocklist length */
	/* starting from 2 * DISK_SECTOR_SIZE, how many sectors are there to load in
	 * core and kernel? kernel is TODO */
	// I'm running an intel processor and current target is x86,
	//	both are little endian so no byte-swapping before write needed
	//	x86_64 is little endian, in general
	// TODO: host64_to_target16 where we need to define host and target machines
	
	if (first_block->length != 0) {
		umbra_error("first_block length is nonzero!");
	}

	first_block->length = (core_sectors - 1); // + kernel_sectors;
	
	// here I assume all the things are correct, so write to file
	{
		FILE *umbra;	
		const char umbra_img_name[] = "umbra.img";
		char *umbra_path;

		umbra_path = get_path(dir, umbra_img_name);
		
		umbra = fopen(umbra_path, "ab");

		if (!umbra) {
			umbra_error("cannot open %s:%s", umbra_path, strerror(errno));
		}

		umbra_write_image(boot_img, boot_size, umbra, umbra_img_name, boot_file);
		umbra_write_image(core_img, core_size, umbra, umbra_img_name, core_file);
		// umbra_write_image(core_img, core_size, umbra, umbra_img_name);
		
		/* 
		 * QUESTION:
		 * I obviously have to let the bootloader do the actual loading, but where
		 * do I put the kernel sector size? Do I put it in the first blocklist?
		 * I would prefer the sectors to load be separate, but idk
		 * */

		fclose(umbra);

		// sanity check
		size_t umbra_size_expected = boot_size + core_size; // + kernel_size;
		size_t umbra_size = umbra_get_image_size(umbra_path);
		if (umbra_size != umbra_size_expected) {
			umbra_error("umbra_size != umbra_size_expected: '%lu' != '%lu'",
					umbra_size, umbra_size_expected);
		} else {
			umbra_info("bootloader size: '%lu'", umbra_size);
		}

		free(umbra_path);
	}

cleanup:
	free(boot_img);
	free(core_img);
	// free(kernel_img);
}

int main(int argc, char *argv[])
{
	char *dir, *boot_file, *core_file;
	/* argv[1] full path to directory containing boot image, core image and kernel image
	 * argv[2] boot file name
	 * argv[3] core file name
	 * TODO argv[4] kernel file name
	 * */

	if (argc != 4) {
		umbra_error("Usage: ./mkimage . boot.img core.img");
	}

	umbra_info("mkimage called with: %s, %s, %s", argv[1], argv[2], argv[3]);

	dir = argv[1];
	boot_file = argv[2];
	core_file = argv[3];

	// accept current working directory
	if (strcmp(dir, ".") == 0) {
		dir = "";
	}

	umbra_mkimage(dir, boot_file, core_file);

	return 0;
}

