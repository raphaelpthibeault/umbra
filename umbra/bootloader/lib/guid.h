#ifndef __GUID_H__
#define __GUID_H__

#include <types.h>

struct guid {
	uint32_t a;
	uint16_t b;
	uint16_t c;
	uint8_t d[8];
};

#endif // !__GUID_H__
