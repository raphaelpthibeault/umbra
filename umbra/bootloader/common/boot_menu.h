#ifndef __BOOT_MENU_H__
#define __BOOT_MENU_H__

#include <types.h>

// TODO: noreturn void boot_menu(void); // in boot_menu.S of arch/${ARCH}/
noreturn void _boot_menu(uint8_t drive);

#endif // !__BOOT_MENU_H__
