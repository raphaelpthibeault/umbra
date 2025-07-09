#ifndef __KBD_H__
#define __KBD_H__

#include <types.h>

#define GETCHAR_CURSOR_LEFT  (-10)
#define GETCHAR_CURSOR_RIGHT (-11)
#define GETCHAR_CURSOR_UP    (-12)
#define GETCHAR_CURSOR_DOWN  (-13)
#define GETCHAR_DELETE       (-14)
#define GETCHAR_END          (-15)
#define GETCHAR_HOME         (-16)
#define GETCHAR_PGUP         (-17)
#define GETCHAR_PGDOWN       (-18)
#define GETCHAR_F10          (-19)
#define GETCHAR_ESCAPE       (-20)
#define GETCHAR_RCTRL 0x4
#define GETCHAR_LCTRL GETCHAR_RCTRL
#define GETCHAR_ENTER (10)

int getchar(void);

int pit_sleep_and_quit_on_keypress(uint32_t ticks) /* defined in arch/$ARCH/kbd.S */
	__attribute__((regparm(1)));

#endif // !_KBD_H__
