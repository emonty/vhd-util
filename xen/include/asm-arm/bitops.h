/*
 * Copyright 1995, Russell King.
 * Various bits and pieces copyrights include:
 *  Linus Torvalds (test_bit).
 * Big endian support: Copyright 2001, Nicolas Pitre
 *  reworked by rmk.
 */

#ifndef _ARM_BITOPS_H
#define _ARM_BITOPS_H

/*
 * Non-atomic bit manipulation.
 *
 * Implemented using atomics to be interrupt safe. Could alternatively
 * implement with local interrupt masking.
 */
#define __set_bit(n,p)            set_bit(n,p)
#define __clear_bit(n,p)          clear_bit(n,p)

#define BIT(nr)                 (1UL << (nr))
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE           8

#define ADDR (*(volatile long *) addr)
#define CONST_ADDR (*(const volatile long *) addr)

#if defined(CONFIG_ARM_32)
# include <asm/arm32/bitops.h>
#elif defined(CONFIG_ARM_64)
# include <asm/arm64/bitops.h>
#else
# error "unknown ARM variant"
#endif

/**
 * __test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static inline int __test_and_set_bit(int nr, volatile void *addr)
{
        unsigned long mask = BIT_MASK(nr);
        volatile unsigned long *p =
                ((volatile unsigned long *)addr) + BIT_WORD(nr);
        unsigned long old = *p;

        *p = old | mask;
        return (old & mask) != 0;
}

/**
 * __test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static inline int __test_and_clear_bit(int nr, volatile void *addr)
{
        unsigned long mask = BIT_MASK(nr);
        volatile unsigned long *p =
                ((volatile unsigned long *)addr) + BIT_WORD(nr);
        unsigned long old = *p;

        *p = old & ~mask;
        return (old & mask) != 0;
}

/* WARNING: non atomic and it can be reordered! */
static inline int __test_and_change_bit(int nr,
                                            volatile void *addr)
{
        unsigned long mask = BIT_MASK(nr);
        volatile unsigned long *p =
                ((volatile unsigned long *)addr) + BIT_WORD(nr);
        unsigned long old = *p;

        *p = old ^ mask;
        return (old & mask) != 0;
}

/**
 * test_bit - Determine whether a bit is set
 * @nr: bit number to test
 * @addr: Address to start counting from
 */
static inline int test_bit(int nr, const volatile void *addr)
{
        const volatile unsigned long *p = (const volatile unsigned long *)addr;
        return 1UL & (p[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
}

static inline int constant_fls(int x)
{
        int r = 32;

        if (!x)
                return 0;
        if (!(x & 0xffff0000u)) {
                x <<= 16;
                r -= 16;
        }
        if (!(x & 0xff000000u)) {
                x <<= 8;
                r -= 8;
        }
        if (!(x & 0xf0000000u)) {
                x <<= 4;
                r -= 4;
        }
        if (!(x & 0xc0000000u)) {
                x <<= 2;
                r -= 2;
        }
        if (!(x & 0x80000000u)) {
                x <<= 1;
                r -= 1;
        }
        return r;
}

/*
 * On ARMv5 and above those functions can be implemented around
 * the clz instruction for much better code efficiency.
 */

static inline int fls(int x)
{
        int ret;

        if (__builtin_constant_p(x))
               return constant_fls(x);

        asm("clz\t%0, %1" : "=r" (ret) : "r" (x));
        ret = BITS_PER_LONG - ret;
        return ret;
}


#define ffs(x) ({ unsigned long __t = (x); fls(__t & -__t); })

/**
 * find_first_set_bit - find the first set bit in @word
 * @word: the word to search
 *
 * Returns the bit-number of the first set bit (first bit being 0).
 * The input must *not* be zero.
 */
static inline unsigned int find_first_set_bit(unsigned long word)
{
        return ffs(word) - 1;
}

/**
 * hweightN - returns the hamming weight of a N-bit word
 * @x: the word to weigh
 *
 * The Hamming Weight of a number is the total number of bits set in it.
 */
#define hweight64(x) generic_hweight64(x)
#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

#endif /* _ARM_BITOPS_H */
/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
