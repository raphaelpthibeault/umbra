#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <types.h>
#include <fs/file.h>

struct menu_entry
{
	char name[64];
	bool expanded;
	struct menu_entry *parent;
	struct menu_entry *sub;
	struct menu_entry *next;
	char *body;
};

extern struct menu_entry *menu_tree;

int config_init_disk(disk_t *boot_disk);
char *config_get_value(const char *config, size_t index, const char *key);

#endif // !__CONFIG_H__
