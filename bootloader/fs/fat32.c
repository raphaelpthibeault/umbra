#include "fat32.h"
#include "file.h"
#include <types.h>
#include <lib/partition.h>
#include <lib/misc.h>
#include <drivers/vga.h>
#include <mm/pmm.h>
#include <drivers/serial.h>

struct fat32_filehandle {
	struct fat32_context ctx;
	uint32_t first_cluster;
	uint32_t size_bytes;
	uint32_t size_sectors;
	uint32_t *cluster_chain;
	size_t chain_len;
};

static void fat32_read(struct filehandle *fh, void *buf, uint64_t loc, uint64_t count);
static void fat32_close(struct filehandle *fh); 

static void
fat32_lfncopy(char *dest, const void *src, uint32_t size)
{
	for (uint32_t i = 0; i < size; ++i) {
		/* deliberately ignoring high bytes */
		*(((uint8_t *)dest) + i) = *(((uint8_t *) src) + (i * 2));
	}	
}

static bool
fat32_fn_to_8_3(char *dest, const char *src) {
	int i = 0, j = 0;
	bool ext = false;

	for (int k = 0; k < 8+3; ++k) {
		dest[k] = ' ';	
	}
	
	while (src[i]) {
		if (src[i] == '.') {
			if (ext) { /* double file extension, not allowed */
				return false;
			}
			ext = true;
			j = 8;
			++i;
			continue;
		}	
		if (j >= 8+3 || (j >= 8 && !ext)) { /* file name too long, not allowed */
			return false;	
		}
		dest[j++] = toupper(src[i++]);
	}

	return true;
}

static int
fat32_mount(struct fat32_context *ctx, struct partition *part) 
{
	size_t valid_len;
	ctx->part = part;

	struct fat32_bpb bpb;
	partition_read(part, 0, sizeof(struct fat32_bpb), &bpb);

	/* FAT12/16 */
	if (strncmp(((void *)&bpb) + 0x36, "FAT", 3) == 0) {
		goto valid_signature;	
	}
	/* FAT32 */
	if (strncmp(((void *)&bpb) + 0x52, "FAT", 3) == 0) {
		putstr("FAT32\n", COLOR_GRY, COLOR_BLK);
		goto valid_signature;	
	}
	/* FAT32 with 64-bit sectors */
	if (strncmp(((void *)&bpb) + 0x03, "FAT32", 5) == 0) {
		putstr("FAT32 with 64-bit sectors\n", COLOR_GRY, COLOR_BLK);
		goto valid_signature;	
	}

	return 1;

valid_signature:
	valid_len = 8;	
	const uint8_t valid_sectors_per_cluster[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

	for (size_t i = 0; i < valid_len; ++i) {
		if (bpb.sectors_per_cluster == valid_sectors_per_cluster[i]) {
			goto valid_sectors_per_cluster;
		}
	}

	return 1;

valid_sectors_per_cluster:
	valid_len = 4;
	const uint16_t valid_bytes_per_sector[] = { 512, 1024, 2048, 4096 };

	for (size_t i = 0; i < valid_len; ++i) {
		if (bpb.bytes_per_sector == valid_bytes_per_sector[i]) {
			goto valid_bytes_per_sector;
		}
	}

	return 1;

valid_bytes_per_sector:
	/* the fat is valid, now to determine the type 
	 * reference: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
	 * The official Microsoft FAT Specification, Section 3: Boot Sector and BPB - 3.5 Determination of FAT type when mounting the volume
	 * */
	size_t root_dir_sectors, fat_sz, total_sectors, data_sectors, clusters_count;

	root_dir_sectors = ((bpb.root_entries_count * 32) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;
	
	fat_sz = bpb.sectors_per_fat_16 != 0 ? bpb.sectors_per_fat_16 : bpb.sectors_per_fat_32;
	total_sectors = bpb.sectors_count_16 != 0 ? bpb.sectors_count_16 : bpb.sectors_count_32;
	data_sectors = total_sectors - (bpb.reserved_sectors + (bpb.fats_count * fat_sz) + root_dir_sectors);

	clusters_count = data_sectors / bpb.sectors_per_cluster;	

	/* determine FAT type */
	if (clusters_count < 4085) { // FAT12
		ctx->type = 12;
	} else if (clusters_count < 65525) { // FAT16
		ctx->type = 16;
	} else { // FAT32
		ctx->type = 32;	
	}

	ctx->bytes_per_sector = bpb.bytes_per_sector;
	ctx->sectors_per_cluster = bpb.sectors_per_cluster;
	ctx->reserved_sectors = bpb.reserved_sectors;
	ctx->number_of_fats = bpb.fats_count;
	ctx->hidden_sectors = bpb.hidden_sectors_count;
	ctx->sectors_per_fat = ctx->type == 32 ? bpb.sectors_per_fat_32 : bpb.sectors_per_fat_16;
	ctx->root_directory_cluster = bpb.root_directory_cluster;
	ctx->fat_start_lba = bpb.reserved_sectors;
	ctx->root_entries = bpb.root_entries_count;
	ctx->root_start = ctx->reserved_sectors + ctx->number_of_fats * ctx->sectors_per_fat;
	ctx->root_size = DIV_ROUNDUP(ctx->root_entries * sizeof(struct fat32_directory_entry), ctx->bytes_per_sector);

	switch (ctx->type) {
		case 12:
		case 16:
			ctx->data_start_lba = ctx->root_start + ctx->root_size;
			break;
		case 32:
			ctx->data_start_lba = ctx->root_start;
			break;
		default:
			__builtin_unreachable();
	}

	/* get the label */
	struct fat32_directory_entry _curr_dir;
	struct fat32_directory_entry *curr_dir;

	switch (ctx->type) {
		case 12:
		case 16:
			curr_dir = NULL;
			break;
		case 32:
			_curr_dir.cluster_num_low = ctx->root_directory_cluster & 0xFFFF;
			_curr_dir.cluster_num_high = ctx->root_directory_cluster >> 16;
			curr_dir = &_curr_dir;
			break;
		default:
			__builtin_unreachable();
	}

	char *volume_label;
	if (fat32_open_in(ctx, curr_dir, (struct fat32_directory_entry *)&volume_label, NULL) == 0) {
		ctx->label = volume_label;
	} else {
		ctx->label = NULL;
	}

	return 0;
}

static int
read_cluster_from_map(struct fat32_context *ctx, uint32_t cluster, uint32_t *out)
{
	switch (ctx->type) {
		case 12:
			*out = 0;
			uint16_t tmp = 0;
			partition_read(ctx->part, ctx->fat_start_lba * ctx->bytes_per_sector + (cluster + cluster / 2), sizeof(uint16_t), &tmp);
			if (cluster % 2 == 0) {
				*out = tmp & 0xfff;
			} else {
				*out = tmp >> 4;
			}
			break;
		case 16:
			*out = 0;
			partition_read(ctx->part, ctx->fat_start_lba * ctx->bytes_per_sector + (cluster * sizeof(uint16_t)), sizeof(uint16_t), out);
			break;
		case 32:
			partition_read(ctx->part, ctx->fat_start_lba * ctx->bytes_per_sector + (cluster * sizeof(uint32_t)), sizeof(uint32_t), out);
			*out &= 0x0fffffff;	
			break;
		default:
			__builtin_unreachable();
	}	

	return 0;
}

static uint32_t *
cache_cluster_chain(struct fat32_context *ctx, uint32_t initial_cluster, size_t *_chain_len)
{
	uint32_t cluster_lim = (ctx->type == 12 ? 0xfef : 0) | (ctx->type == 16 ? 0xffef : 0) | (ctx->type == 32 ? 0xfffffef : 0);
	if (initial_cluster < 0x2 || initial_cluster > cluster_lim) {
		return NULL;
	}

	uint32_t cluster = initial_cluster;
	size_t chain_len;
	for (chain_len = 1; ; ++chain_len) {
		read_cluster_from_map(ctx, cluster, &cluster);	
		if (cluster < 0x2 || cluster > cluster_lim) {
			break;
		}
	}

	uint32_t *cluster_chain = ext_mem_alloc(chain_len * sizeof(uint32_t));
	cluster = initial_cluster;

	for (size_t i = 0; i < chain_len; ++i) {
		cluster_chain[i] = cluster;
		read_cluster_from_map(ctx, cluster, &cluster);
	}
	
	*_chain_len = chain_len;
	return cluster_chain;
}

static bool
read_cluster_chain(struct fat32_context *ctx, uint32_t *cluster_chain, void *buf, uint64_t loc, uint64_t count)
{
	size_t block_size = ctx->sectors_per_cluster * ctx->bytes_per_sector; 	
	uint64_t progress = 0;

	while (progress < count) {
		uint64_t block = (loc + progress) / block_size;
		uint64_t chunk = count - progress;
		uint64_t offset = (loc + progress) % block_size;
		if (chunk > block_size - offset) {
			chunk = block_size - offset;
		}

		uint64_t base = ((uint64_t)ctx->data_start_lba + (cluster_chain[block] - 2) * ctx->sectors_per_cluster) * ctx->bytes_per_sector;
		partition_read(ctx->part, base + offset, chunk, buf + progress);
		progress += chunk;
	}

	return true;
}

int
fat32_open_in(struct fat32_context *ctx, struct fat32_directory_entry *dir, struct fat32_directory_entry *file, const char *name) 
{
	char curr_lfn[FAT32_LFN_MAX_FILENAME_LENGTH] = {0};
	struct fat32_directory_entry *dir_entries;
	size_t dir_chain_len;
	size_t block_size = ctx->sectors_per_cluster * ctx->bytes_per_sector; 	

	if (dir != NULL) {
		uint32_t curr_cluster_num = dir->cluster_num_low;
		if (ctx->type == 32) {
			curr_cluster_num |= (uint32_t)dir->cluster_num_high << 16;
		}

		uint32_t *dir_cluster_chain = cache_cluster_chain(ctx, curr_cluster_num, &dir_chain_len);

		if (dir_cluster_chain == NULL) {
			return -1;
		}

		dir_entries = ext_mem_alloc(dir_chain_len * block_size);
		
		read_cluster_chain(ctx, dir_cluster_chain, dir_entries, 0, dir_chain_len * block_size);

		memmap_free(dir_cluster_chain, dir_chain_len * sizeof(uint32_t));
	} else {
		dir_chain_len = DIV_ROUNDUP(ctx->root_entries * sizeof(struct fat32_directory_entry), block_size);
		dir_entries = ext_mem_alloc(dir_chain_len * block_size);

		partition_read(ctx->part, ctx->root_start * ctx->bytes_per_sector, ctx->root_entries * sizeof(struct fat32_directory_entry), dir_entries);
	}

	int ret;

	for (size_t i = 0; i < ((dir_chain_len * block_size)/sizeof(struct fat32_directory_entry)); ++i) {
		if (dir_entries[i].file_name_and_ext[0] == 0x00) { /* no more entries */
			break;
		}

		if (name == NULL) {
			if (dir_entries[i].attribute != FAT32_ATTRIBUTE_VOLLABEL) {
				continue;
			}	

			char *r = ext_mem_alloc(12);
			memcpy(r, dir_entries[i].file_name_and_ext, 11);
			/* remove trailing spaces */
			for (int j = 10; j >= 0; --j) {
				if (r[j] == ' ') {
					r[j] = 0;
					continue;
				}
				break;
			}
			// alloc the file and leave
			*((char **)file) = r;
			ret = 0;
			goto done;
		}

		if (dir_entries[i].attribute == FAT32_LFN_ATTRIBUTE) {
			struct fat32_lfn_entry *lfn = (struct fat32_lfn_entry *)&dir_entries[i];

			if (lfn->sequence_number & 0b01000000) {
				/* first entry in the table, clear the lfn buf */
				memset(curr_lfn, ' ', sizeof(curr_lfn));
			}

			const uint32_t lfn_index = ((lfn->sequence_number & 0b00011111) - 1U) * 13U;
			if (lfn_index >= FAT32_LFN_MAX_ENTRIES * 13) {
				continue;
			}

			fat32_lfncopy(curr_lfn + lfn_index + 0, lfn->name1, 5);
			fat32_lfncopy(curr_lfn + lfn_index + 5, lfn->name2, 6);
			fat32_lfncopy(curr_lfn + lfn_index + 11, lfn->name3, 2);

			if (lfn_index != 0) {
				continue;
			}

			/* remove trailing spaces */
			for (int j = SIZEOF_ARRAY(curr_lfn) - 2; j >= -1; --j) {
				if (j == -1 || curr_lfn[j] != ' ') {
					curr_lfn[j + 1] = 0;
					break;
				}
			}

			if (strcmp(curr_lfn, name) == 0) {
				*file = dir_entries[i+1];
				ret = 0;
				goto done;
			}
		}

		if (dir_entries[i].attribute & (1 << 3)) {
			/* volume label, skip */	
			continue;
		}

		/* SFN */
		char fn[8+3];
		if (!fat32_fn_to_8_3(fn, name)) {
			continue;
		}
		if (!strncmp(dir_entries[i].file_name_and_ext, fn, 8+3)) {
			*file = dir_entries[i];
			ret = 0;
			goto done;
		}
	}

	/* file not found */
	ret = -1;

done:
	memmap_free(dir_entries, dir_chain_len * block_size);
	return ret;
}

char *
fat32_get_label(struct partition *part)
{
	struct fat32_context ctx;
	if (fat32_mount(&ctx, part) != 0) {
		return NULL;
	}

	return ctx.label;
}

struct filehandle *
fat32_open(struct partition *part, const char *path) 
{
	struct fat32_context ctx;
	if (fat32_mount(&ctx, part) != 0) {
		return NULL;
	}

	struct fat32_directory_entry _curr_dir;
	struct fat32_directory_entry *curr_dir;
	struct fat32_directory_entry curr_file;

	uint32_t curr_idx = 0;
	char curr_part[FAT32_LFN_MAX_FILENAME_LENGTH];
	/* skip trailing slash */
	while (path[curr_idx] == '/') {
		++curr_idx;
	}

	/* walk directory tree */
	switch (ctx.type) {
		case 12:
		case 16:
			curr_dir = NULL;
			break;
		case 32:
			_curr_dir.cluster_num_low = ctx.root_directory_cluster & 0xFFFF;
			_curr_dir.cluster_num_high = ctx.root_directory_cluster >> 16;
			curr_dir = &_curr_dir;
			break;
		default:
			__builtin_unreachable();
	}

	while (true) {
		bool expect_dir = false;
		for (uint32_t i = 0; i < SIZEOF_ARRAY(curr_part); ++i) {
			if (path[i + curr_idx] == 0) {
				memcpy(curr_part, path + curr_idx, i);
				curr_part[i] = 0;
				expect_dir = false;
				break;
			}
			if (path[i + curr_idx] == '/') {
				memcpy(curr_part, path + curr_idx, i);
				curr_part[i] = 0;
				curr_idx += i + 1;
				expect_dir = true;
				break;
			}
		}

		if (fat32_open_in(&ctx, curr_dir, &curr_file, curr_part) != 0) {
			return NULL;
		}

		if (expect_dir) {
			_curr_dir = curr_file;
			curr_dir = &_curr_dir;
		} else {
			struct filehandle *fh = ext_mem_alloc(sizeof(struct filehandle));
			struct fat32_filehandle *ret = ext_mem_alloc(sizeof(struct fat32_filehandle));

			ret->ctx = ctx;
			ret->first_cluster = curr_file.cluster_num_low;
			if (ctx.type == 32) {
				ret->first_cluster |= (uint64_t)curr_file.cluster_num_high << 16;
			}
			ret->size_bytes = curr_file.file_size_bytes;
			ret->size_sectors = DIV_ROUNDUP(curr_file.file_size_bytes, ctx.bytes_per_sector);
			ret->cluster_chain = cache_cluster_chain(&ctx, ret->first_cluster, &ret->chain_len);

			fh->fd = (void *)ret;
			fh->read = (void *)fat32_read;
			fh->close = (void *)fat32_close;
			fh->size = ret->size_bytes;
			fh->part = part;

			return fh;
		}
	}
}

static void
fat32_read(struct filehandle *fh, void *buf, uint64_t loc, uint64_t count) 
{
	struct fat32_filehandle *fd = (struct fat32_filehandle *)fh->fd;
	if (!read_cluster_chain(&fd->ctx, fd->cluster_chain, buf, loc, count))
	{
		serial_print("[WARNING] FAT32: Error reading cluster chain\n");
	}
}

static void
fat32_close(struct filehandle *fh) 
{
	struct fat32_filehandle *fd = (struct fat32_filehandle *)fh->fd;
	memmap_free(fd->cluster_chain, fd->chain_len * sizeof(uint32_t));
	memmap_free(fd, sizeof(struct fat32_filehandle));
}
