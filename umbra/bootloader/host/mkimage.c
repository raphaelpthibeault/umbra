#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdnoreturn.h>
#include <stdbool.h>

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

noreturn void 
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

uint8_t *
umbra_read_image(const char *path)
{
	uint8_t *img;
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
		umbra_error("cannot read %s:%s", path, strerror(errno));
	}

	fclose(f);

	return img;
}

static inline int
set_pos(FILE *stream, uint64_t pos) 
{
	if (sizeof(long) >= 8) {
		return fseek(stream, (long)pos, SEEK_SET);
	} 
	return -1;
}

/* write buffer to out at location loc */
void
umbra_write_out(uint8_t *_buf, FILE *out, size_t loc, size_t count)
{
	uint8_t *buf = _buf;	
	
	if (set_pos(out, loc) != 0) {
		umbra_error("set_pos(): %s", strerror(errno));
	}

	if (fwrite(buf, 1, count, out) != count) {
		umbra_error("fwrite(): %s", strerror(errno));
	}
}

/* read to buffer from out at location loc */
void
umbra_read_in(uint8_t *_buf, FILE *out, size_t loc, size_t count)
{
	uint8_t *buf = _buf;

	if (set_pos(out, loc) != 0) {
		umbra_error("set_pos(): %s", strerror(errno));
	}

	if (fread(buf, 1, count, out) != count) {
		umbra_error("fread(): %s", strerror(errno));
	}
}

void
umbra_mkimage(const char *out_file)
{
	char *boot_path, *core_path;
	size_t boot_size, core_size;
	uint16_t core_sectors;
	struct umbra_boot_blocklist *first_block;
	uint8_t *boot_img, *core_img;

	char boot_file[] = "boot.hdd";
	char core_file[] = "core.hdd";
	char dir[] = "umbra/bootloader/"; // calling from root direcotry, so bootloader directory as working directory TODO ? have some global bin var that umbra/Makefile writes to ?

	umbra_info("setting up '%s'...", out_file);

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

	/* load via core size, (i.e. just load stage2) since stage3 and kernel will be in file system */
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
	/* starting from 2 * DISK_SECTOR_SIZE, how many sectors are there to load in stage2 (core.hdd) */
	// I'm running an intel processor and current target is x86,
	//	both are little endian so no byte-swapping before write needed
	//	x86_64 is little endian, in general
	// TODO: host64_to_target16 where we need to define host and target machines
	
	if (first_block->length != 0) {
		umbra_error("first_block length is nonzero!");
	}

	first_block->length = (core_sectors - 1);

	/* 
	 * here I would put the MBR, but since we're not using that I don't think I care
	 * I assume the original raw image will have it from some tool like sgdisk
	 * */

	// check bios magic	(little endian)
	uint16_t magic = (boot_img[511] << 8) + boot_img[510];
	if (magic != 0xaa55) {
		umbra_error("bios magic not 0xaa55");
	}

	FILE *out_img;

	/* calling mkimage from root, so assume out_file is the raw umbra.img that is also in root */
	out_img = fopen(out_file, "r+b");
	
	if (!out_img) {
		umbra_error("cannot open %s:%s", out_file, strerror(errno));
	}

	/* check MBR partitions, assume out_img has MBR partitions from some tool like sgdisk */
	bool any_active = false;

	uint8_t hint8;
	umbra_read_in(&hint8, out_img, 0x1be + 0x04, sizeof(uint8_t));
	if (hint8 != 0x00 && hint8 != 0x80) {
		// force MBR, this is likely a EFI partition, 0xef
		hint8 &= 0x80;
		umbra_write_out(&hint8, out_img, 0x1be + 0x04, sizeof(uint8_t));
	}
	any_active = any_active || (hint8 & 0x80) != 0;

	umbra_read_in(&hint8, out_img, 0x1ce + 0x04, sizeof(uint8_t));
	if (hint8 != 0x00 && hint8 != 0x80) {
		hint8 &= 0x80;
		umbra_write_out(&hint8, out_img, 0x1ce + 0x04, sizeof(uint8_t));
	}
	any_active = any_active || (hint8 & 0x80) != 0;

	umbra_read_in(&hint8, out_img, 0x1de + 0x04, sizeof(uint8_t));
	if (hint8 != 0x00 && hint8 != 0x80) {
		hint8 &= 0x80;
		umbra_write_out(&hint8, out_img, 0x1de + 0x04, sizeof(uint8_t));
	}
	any_active = any_active || (hint8 & 0x80) != 0;

	umbra_read_in(&hint8, out_img, 0x1ee + 0x04, sizeof(uint8_t));
	if (hint8 != 0x00 && hint8 != 0x80) {
		hint8 &= 0x80;
		umbra_write_out(&hint8, out_img, 0x1ee + 0x04, sizeof(uint8_t));
	}
	any_active = any_active || (hint8 & 0x80) != 0;

	if (!any_active) {
		umbra_error("No active partitions found!");
	}

	// keep MBR, going along with the assumption that out_img has MBR partitions (from above)
	uint8_t original_mbr[70];
	umbra_read_in(original_mbr, out_img, 440, 70);

	umbra_info("writing boot image to '%s'", out_file);
	umbra_write_out(boot_img, out_img, 0, boot_size);
	umbra_info("writing core image to '%s'", out_file);
	umbra_write_out(core_img, out_img, 512, core_size);
	umbra_info("writing mbr to '%s'", out_file);
	umbra_write_out(original_mbr, out_img, 440, 70);

	fclose(out_img);

	// sanity check
	size_t size_expected = 0x4000000; // size of raw img file defined in umbra/bootloader/Makefile
	size_t size = umbra_get_image_size(out_file);
	if (size != size_expected) {
		umbra_error("size != size_expected: '%lu' != '%lu'",
				size, size_expected);
	}

	// stage3 at 0xf000 so don't be bigger than that
	if (boot_size + core_size >= 0xf000) {
		umbra_error("boot_size + core_size: '%lu' is >= '%lu'",
				boot_size+core_size, 0xf000);
	}

cleanup:
	free(boot_img);
	free(core_img);
}

int 
main(int argc, char *argv[])
{
	char *out_file;
	/* mkimage should just know where boot and core images are 
	 * argv[1] -> raw (zeroed-out) image file
	 * */

	if (argc != 2) {
		umbra_error("Usage: ./mkimage <path-to-raw-image-file>");
	}

	umbra_info("mkimage called with: %s", argv[1]);
	out_file = argv[1];

	umbra_mkimage(out_file);

	return 0;
}

