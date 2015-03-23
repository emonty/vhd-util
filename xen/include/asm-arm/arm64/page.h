#ifndef __ARM_ARM64_PAGE_H__
#define __ARM_ARM64_PAGE_H__

#ifndef __ASSEMBLY__

/* Write a pagetable entry */
static inline void write_pte(lpae_t *p, lpae_t pte)
{
    asm volatile (
        /* Ensure any writes have completed with the old mappings. */
        "dsb sy;"
        "str %0, [%1];"         /* Write the entry */
        "dsb sy;"
        : : "r" (pte.bits), "r" (p) : "memory");
}

/* Inline ASM to flush dcache on register R (may be an inline asm operand) */
#define __clean_xen_dcache_one(R) "dc cvac, %" #R ";"

/* Inline ASM to clean and invalidate dcache on register R (may be an
 * inline asm operand) */
#define __clean_and_invalidate_xen_dcache_one(R) "dc  civac, %" #R ";"

/*
 * Flush all hypervisor mappings from the TLB
 * This is needed after changing Xen code mappings.
 *
 * The caller needs to issue the necessary DSB and D-cache flushes
 * before calling flush_xen_text_tlb.
 */
static inline void flush_xen_text_tlb(void)
{
    asm volatile (
        "isb;"       /* Ensure synchronization with previous changes to text */
        "tlbi   alle2;"                 /* Flush hypervisor TLB */
        "ic     iallu;"                 /* Flush I-cache */
        "dsb    sy;"                    /* Ensure completion of TLB flush */
        "isb;"
        : : : "memory");
}

/*
 * Flush all hypervisor mappings from the data TLB. This is not
 * sufficient when changing code mappings or for self modifying code.
 */
static inline void flush_xen_data_tlb(void)
{
    asm volatile (
        "dsb    sy;"                    /* Ensure visibility of PTE writes */
        "tlbi   alle2;"                 /* Flush hypervisor TLB */
        "dsb    sy;"                    /* Ensure completion of TLB flush */
        "isb;"
        : : : "memory");
}

/*
 * Flush a range of VA's hypervisor mappings from the data TLB. This is not
 * sufficient when changing code mappings or for self modifying code.
 */
static inline void flush_xen_data_tlb_range_va(unsigned long va, unsigned long size)
{
    unsigned long end = va + size;
    dsb(); /* Ensure preceding are visible */
    while ( va < end ) {
        asm volatile("tlbi vae2, %0;"
                     : : "r" (va>>PAGE_SHIFT) : "memory");
        va += PAGE_SIZE;
    }
    dsb(); /* Ensure completion of the TLB flush */
    isb();
}

/* Ask the MMU to translate a VA for us */
static inline uint64_t __va_to_par(vaddr_t va)
{
    uint64_t par, tmp = READ_SYSREG64(PAR_EL1);

    asm volatile ("at s1e2r, %0;" : : "r" (va));
    isb();
    par = READ_SYSREG64(PAR_EL1);
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
}

/* Ask the MMU to translate a Guest VA for us */
static inline uint64_t gva_to_ma_par(vaddr_t va)
{
    uint64_t par, tmp = READ_SYSREG64(PAR_EL1);

    asm volatile ("at s12e1r, %0;" : : "r" (va));
    isb();
    par = READ_SYSREG64(PAR_EL1);
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
}

static inline uint64_t gva_to_ipa_par(vaddr_t va)
{
    uint64_t par, tmp = READ_SYSREG64(PAR_EL1);

    asm volatile ("at s1e1r, %0;" : : "r" (va));
    isb();
    par = READ_SYSREG64(PAR_EL1);
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
}

#endif /* __ASSEMBLY__ */

#endif /* __ARM_ARM64_PAGE_H__ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
