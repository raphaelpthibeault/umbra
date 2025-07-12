#include <common/config.h>
#include <types.h>
#include <fs/file.h>
#include <mm/pmm.h>
#include <lib/misc.h>
#include <drivers/serial.h>

#define NOT_CHILD (-1)
#define DIRECT_CHILD 0
#define INDIRECT_CHILD 1

static char *config_addr = NULL;

struct menu_entry *menu_tree = NULL;

static bool
get_entry_name(char *ret, size_t index, size_t limit)
{
	char *p = config_addr;	

	for (size_t i = 0; i <= index; ++i)
	{
		while (*p != '/')
		{
			if (!*p) return false;
			++p;
		}
		++p;
		if ((p - 1) != config_addr && *(p - 2) != '\n')
			--i;
	}

	--p;

	size_t i;
	for (i = 0; i < (limit - 1); ++i)
	{
		if (p[i] == '\n')
			break;
		ret[i] = p[i];
	}

	ret[i] = 0;
	return true;
}

static char *
get_entry(size_t *size, size_t index)
{
	char *ret, *p = config_addr;

	for (size_t i = 0; i <= index; ++i)
	{
		while (*p != '/')
		{
			if (!*p) return NULL;
			++p;
		}
		++p;
		if ((p - 1) != config_addr && *(p - 2) != '\n')
			--i;
	}

	do {
		++p;
	} while (*p != '\n');

	ret = p;

get_size:
	while (*p != '/' && *p)
		++p;

	if (*p && *(p - 1) != '\n')
	{
		++p;
		goto get_size;
	}

	*size = p - ret;

	return ret;
}


static int
is_child(char *buf, size_t limit, size_t curr_depth, size_t index)
{
	if (!get_entry_name(buf, index, limit))
		return NOT_CHILD;
	if (strlen(buf) < curr_depth + 1)
		return NOT_CHILD;

	for (size_t j = 0; j < curr_depth; ++j)
	{
		if (buf[j] != '/')
			return NOT_CHILD;
	}

	if (buf[curr_depth] == '/')
		return INDIRECT_CHILD;

	return DIRECT_CHILD;
}

static bool
is_directory(char *buf, size_t limit, size_t curr_depth, size_t index)
{
	switch (is_child(buf, limit, curr_depth + 1, index + 1))
	{
		default:
		case NOT_CHILD:
			return false;
		case INDIRECT_CHILD:
			serial_print("[PANIC] bad config: parentless child\n");
			while (1);
		case DIRECT_CHILD:
			return true;
	}
}

static struct menu_entry *
create_menu_tree(struct menu_entry *parent, size_t curr_depth, size_t index)
{
	struct menu_entry *root = NULL, *prev = NULL;	

	for (size_t i = index; ; ++i)
	{
		static char name[64];
		
		switch (is_child(name, 64, curr_depth, i))
		{
			case NOT_CHILD:
				return root;
			case INDIRECT_CHILD:
				continue;
			case DIRECT_CHILD:
				break;
		}

		struct menu_entry *entry = ext_mem_alloc(sizeof(struct menu_entry));

		if (root == NULL)
			root = entry;

		get_entry_name(name, i, 64); /* overrides is_child's call to get_entry_name */

		bool default_expanded = name[curr_depth] == '+';
		char *n = &name[curr_depth + default_expanded];
		while (*n == ' ') 
			++n;

		strcpy(entry->name, n);
		entry->parent = parent;

		size_t entry_size;
		char *config_entry = get_entry(&entry_size, i);
		entry->body = ext_mem_alloc(entry_size + 1);
		memcpy(entry->body, config_entry, entry_size);
		entry->body[entry_size] = 0;

		if (is_directory(name, 64, curr_depth, i))
		{
			entry->sub = create_menu_tree(entry, curr_depth + 1, i + 1);
			entry->expanded = default_expanded;
		}

		if (prev != NULL)
			prev->next = entry;

		prev = entry;
	}
}

static int
config_init(size_t config_size)
{
	/* TODO obviously the correct thing to do would be to add a checksum, but whatever this is a hobby project I'll do it later (never) */

	/* add trailing newline if not present */
	config_addr[config_size - 2] = '\n';

	/* remove carriage returns */
	for (size_t i = 0; i < config_size; ++i)
	{
		size_t skip = 0;
		if (config_addr[i] == ' ' || config_addr[i] == '\t')
		{
			while (config_addr[i + skip] == ' ' || config_addr[i + skip] == '\t')
				++skip;

			if (config_addr[i + skip] == '\n')
				goto skip_loop;

			skip = 0;
		}

		while (config_addr[i + skip] == '\r' 
				|| ((!i || config_addr[i - 1] == '\n') && (config_addr[i + skip] == ' ' || config_addr[i + skip] == '\t')))
				++skip;

skip_loop:
		if (skip)
		{
			for (size_t j = i; j < config_size; ++j)
				config_addr[j] = config_addr[j + skip];

			config_size -= skip;
		}
	}

	menu_tree = create_menu_tree(NULL, 1, 0);
	size_t s;
	char *c = get_entry(&s, 0);
	if (c != NULL)
	{
		while (*c != '/')
			--c;

		if (c > config_addr)
			c[-1] = 0;
	}

	return 0;
}

int
config_init_disk(disk_t *boot_disk)
{
	struct filehandle *fh;
	for (int i = 0; i < boot_disk->max_partition; ++i)
	{
		struct partition part = boot_disk->partition[i];
		if ((fh = fopen(&part, "/boot/bootloader/umbra.cfg")) != NULL)
		{
			goto opened;
		}
	}

	return -1;

opened:
	size_t config_size = fh->size + 2;
	config_addr = ext_mem_alloc(config_size);

	fread(fh, config_addr, 0, fh->size);
	fclose(fh);

	return config_init(config_size);
}


char *
config_get_value(const char *config, size_t index, const char *key)
{
	if (!key) 
		return NULL;

	if (!config)
		config = config_addr;

	size_t key_len = strlen(key);

	for (size_t i = 0; config[i]; ++i)
	{
		if (strncmp(&config[i], key, key_len) == 0 && config[i + key_len] == ':')
		{
			if (i && config[i - 1] != '\n')
				continue;
			if (index--)
				continue;
			i += key_len + 1;
			while (config[i] == ' ' || config[i] == '\t')
				++i;

			size_t value_len;
			for (value_len = 0; config[i + value_len] && config[i + value_len] != '\n'; ++value_len);
			char *buf = ext_mem_alloc(value_len + 1);
			memcpy(buf, config + i, value_len);
			return buf;
		}
	}

	return NULL;
}
