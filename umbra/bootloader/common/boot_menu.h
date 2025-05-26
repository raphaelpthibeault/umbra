#ifndef __BOOT_MENU_H__
#define __BOOT_MENU_H__

#include <types.h>

noreturn void boot_menu(void); // in boot_menu.S of arch/${ARCH}/
noreturn void boot(void);

#endif // !__BOOT_MENU_H__
