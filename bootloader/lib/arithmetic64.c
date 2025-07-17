#include <types.h>

/* GCC 64-bit Integer Library Routines for 32-bit systems that can't link to libgcc.a
 * Meant to be used by my hobby operating system github.com/raphaelpthibeault/umbra
 * but of course can be used whereever libgcc cannot e.g. embedded, os, etc
 * Reference:
 * https://gcc.gnu.org/onlinedocs/gccint/Integer-library-routines.html
 * */

typedef union {
	uint64_t u64;
	int64_t s64;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__	
		uint32_t lo; uint32_t hi;
#else
		uint32_t hi; uint32_t lo;
#endif
	} u32;

	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__	
		int32_t lo; int32_t hi;
#else
		int32_t hi; int32_t lo;
#endif
	} s32;
} word64;

/* extract high and low bits from a 64-bit value */
#define LOW(n) (word64){.u64=n}.u32.lo
#define HIGH(n) (word64){.u64=n}.u32.hi

/* negate a if b is negative, but xor'ing always results in  1 less so add 1 back (if neg) */
#define NEGATE(a, b) (((a) ^ ((((int64_t)(b)) >= 0) - 1)) + (((int64_t)(b)) < 0))
#define ABS(a) NEGATE(a, a)

/* return the result of shifting a left by b bits */
int64_t
__ashldi3(int64_t a, int b) 
{
	word64 w = {.s64 = a};

	b &= 0x3f;

	if (b >= 0x20) {
		/* just move low into high, and flush low with 0 */
		w.u32.hi = w.u32.lo << (b - 32);
		w.u32.lo = 0;
	} else {
		w.u32.hi = (w.u32.hi << b) | (w.u32.lo >> (32 - b));
		w.u32.lo <<= b;
	}

	return w.s64;
}

/* return the result of arithmetically shifting a right by b bits */
int64_t
__ashrti3(int64_t a, int b) 
{
	word64 w = {.s64 = a};

	b &= 0x3f;

	if (b >= 0x20) {
		/* put high into low, flush high */
		w.u32.lo = w.u32.hi >> (b - 32);
		w.u32.hi >>= 31; // 0xFFFFFFFF or 0
	} else {
		w.u32.lo = (w.u32.lo >> b) | (w.u32.hi << (32 - b));
		w.u32.hi >>= b;
	}

	return w.s64;

}

/* return the absolute value of a */
int64_t 
__absvdi2(int64_t a) 
{
	return ABS(a);
}

/* These functions return the number of leading 0-bits in a, starting at the most significant bit position. 
 * If a is zero, the result is undefined */
int
__clzsi2(uint32_t a) 
{
	int b, n = 0;
	/* a combination of several tricks
	 * */
	b = (!(a & 0xffff0000)) << 4; // if x is small, t = 16, else 0
	a >>= 16 - b; // a = [0 - 0xffff]
	n += b;	// n = [0 or 16]
	// return r + __clzsi2(a)
	
	b = (!(a & 0x0000ff00)) << 3;
	a >>= 8 - b; // a = [0 - 0xff]
	n += b; // n = [0, 8, 16, or 24]
	// return r + __clzsi2(a)

	b = (!(a & 0x000000f0)) << 2;
	a >>= 4 - b; // a = [0 - 0xf]
	n += b; // n = [0, 4, 8, 12, 16, 20, 24, or 28]
	// return r + __clzsi2(a)

	b = (!(a & 0x0000000c)) << 1;
	a >>= 2 - b; // a = [0 - 3]
	n += b; // n = [0-30 and is even]
	// return n + __clzsi2(a)
	/* switch(a) {
	 * case 0:
	 *	return n+2;
	 * case 1:
	 *	return n+1
	 * case 2:
	 * case 3:
	 *	return n;
	 * }
	 * */

	return n + ((2 - a) & -(!(a & 2)));
}

int
__clzdi2(uint64_t a) 
{
	/* same as __clzsi2 but check first 32 as well */
	int b, n = 0;

	b = (!(a & 0xffffffff00000000ULL)) << 5; // if x is small, t = 32, else 0
	a >>= 32 - b; // a = [0 - 0xffffffff]
	n += b; // n = [0 or 32]

	b = (!(a & 0x00000000ffff0000ULL)) << 4; // if x is small, t = 16, else 0
	a >>= 16 - b; // a = [0 - 0xffff]
	n += b;	// n = [0, 16, 32, or 48]
	
	b = (!(a & 0x000000000000ff00ULL)) << 3;
	a >>= 8 - b; // a = [0 - 0xff]
	n += b; // n = [0, 8, 16, 24, 32, 40, 48, or 56]

	b = (!(a & 0x00000000000000f0ULL)) << 2;
	a >>= 4 - b; // a = [0 - 0xf]
	n += b; // n = [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, or 60]

	b = (!(a & 0x000000000000000cULL)) << 1;
	a >>= 2 - b; // a = [0 - 3]
	n += b; // n = [0-62]

	return n + ((2 - a) & -(!(a & 2)));
}

/* calculate both the quotient and remainder of the unsigned division of a and b. The return value is the quotient, and the remainder is placed in variable pointed to by c */
uint64_t
__udivmoddi4(uint64_t a, uint64_t b, uint64_t *c) 
{
	if (b > a) { /* denominator is greater than numerator */
		if (c) {
			*c = a; /* remainder = numerator */
		}
		return 0; /* quotient */
	}	

	if (!HIGH(b)) { /* denominator is 32-bit */
		if (b == 0) { /* divide by 0 */
			volatile char x = 0;
			x = 1 / x; /* force exception */
		}	
		if (b == 1) { /* a / 1 ; remainder is 0, quotient is a */
			if (c) {
				*c = 0; 
			}	
			return a;
		}
		if (!HIGH(a)) { /* numerator is also 32-bit so just use normal arithmetic operators */
			if (c) {
				*c = LOW(a) % LOW(b);
			}	
			return LOW(a) / LOW(b);
		}
	}	

	/* we've got to do some 64-bit division now */
	char bits = __clzdi2(b) - __clzdi2(a) + 1;  /* number of bits to iterate (a and b are non-zero) */
	uint64_t rem = a >> bits; /* initialize remainder */
	a <<= 64 - bits; /* shift numerator to the high bit */
	uint64_t wrap = 0;
	while (bits-- > 0) {
		rem = (rem << 1) | (a >> 63); // shift numerator MSB to remainder LSB
		a = (a << 1) | (wrap & 1); // shift out the numerator, shift in wrap
		wrap = ((int64_t)(b - rem - 1) >> 63); // wrap = (b > rem) ? 0 : 0xffffffffffffffff (via sign extension)
		rem -= b & wrap; // if (wrap) rem -= b
	}
	if (c) {
		*c = rem;
	}

	return (a << 1) | (wrap & 1); 
}

/* return the quotient of the unsigned division of a and b */
uint64_t
__udivdi3(uint64_t a, uint64_t b)
{
	return __udivmoddi4(a, b, (void *)0);
}

/*  return the remainder of the unsigned division of a and b */
uint64_t
__umoddi3(uint64_t a, uint64_t b)
{
	uint64_t rem;	
	(void)__udivmoddi4(a, b, &rem);
	return rem;	
}

