#ifndef __FAT32_H__
#define __FAT32_H__

#include <types.h>
#include <lib/partition.h>

#define FAT32_LFN_MAX_ENTRIES 20
#define FAT32_LFN_MAX_FILENAME_LENGTH (FAT32_LFN_MAX_ENTRIES * 13 + 1)

#define FAT32_ATTRIBUTE_SUBDIRECTORY 0x10
#define FAT32_LFN_ATTRIBUTE 0x0F
#define FAT32_ATTRIBUTE_VOLLABEL 0x08

struct fat32_context {
	struct partition *part;
	int type;
	char *label;
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t number_of_fats;
	uint32_t hidden_sectors;
	uint32_t sectors_per_fat;
	uint32_t fat_start_lba;
	uint32_t data_start_lba;
	uint32_t root_directory_cluster;
	uint16_t root_entries;
	uint32_t root_start;
	uint32_t root_size;
};

struct fat32_file_handle {
	struct fat32_context context;
	uint32_t first_cluster;
	uint32_t size_bytes;
	uint32_t size_clusters;
	uint32_t *cluster_chain;
	size_t chain_len;
};

struct fat32_bpb {
	union {
		struct {
			uint8_t jump[3];
			char oem[8];
			uint16_t bytes_per_sector;
			uint8_t sectors_per_cluster;
			uint16_t reserved_sectors;
			uint8_t fats_count;
			uint16_t root_entries_count;
			uint16_t sectors_count_16;
			uint8_t media_descriptor_type;
			uint16_t sectors_per_fat_16;
			uint16_t sectors_per_track;
			uint16_t heads_count;
			uint32_t hidden_sectors_count;
			uint32_t sectors_count_32;
			uint32_t sectors_per_fat_32;
			uint16_t flags;
			uint16_t fat_version_number;
			uint32_t root_directory_cluster;
			uint16_t fs_info_sector;
			uint16_t backup_boot_sector;
			uint8_t reserved[12];
			uint8_t drive_number;
			uint8_t nt_flags;
			uint8_t signature;
			uint32_t volume_serial_number;
			char label[11];
			char system_identifier[8];
		} __attribute__((packed));

		uint8_t padding[512];
	};
} __attribute__((packed));

struct fat32_directory_entry {
    char file_name_and_ext[8 + 3];
    uint8_t attribute;
    uint8_t file_data_1[8];
    uint16_t cluster_num_high;
    uint8_t file_data_2[4];
    uint16_t cluster_num_low;
    uint32_t file_size_bytes;
} __attribute__((packed));

struct fat32_lfn_entry {
    uint8_t sequence_number;
    char name1[10];
    uint8_t attribute;
    uint8_t type;
    uint8_t dos_checksum;
    char name2[12];
    uint16_t first_cluster;
    char name3[4];
} __attribute__((packed));

char *fat32_get_label(struct partition *part);
int fat32_open_in(struct fat32_context *ctx, struct fat32_directory_entry *dir, struct fat32_directory_entry *file, const char *name);

#endif // !__FAT32_H__
