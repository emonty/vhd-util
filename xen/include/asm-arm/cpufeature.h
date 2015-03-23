#ifndef __ASM_ARM_CPUFEATURE_H
#define __ASM_ARM_CPUFEATURE_H

#ifdef CONFIG_ARM_64
#define cpu_feature64(c, feat)         ((c)->pfr64.feat)
#define boot_cpu_feature64(feat)       (boot_cpu_data.pfr64.feat)

#define cpu_has_el0_32    (boot_cpu_feature64(el0) == 2)
#define cpu_has_el0_64    (boot_cpu_feature64(el0) >= 1)
#define cpu_has_el1_32    (boot_cpu_feature64(el1) == 2)
#define cpu_has_el1_64    (boot_cpu_feature64(el1) >= 1)
#define cpu_has_el2_32    (boot_cpu_feature64(el2) == 2)
#define cpu_has_el2_64    (boot_cpu_feature64(el2) >= 1)
#define cpu_has_el3_32    (boot_cpu_feature64(el3) == 2)
#define cpu_has_el3_64    (boot_cpu_feature64(el3) >= 1)
#define cpu_has_fp        (boot_cpu_feature64(fp) == 0)
#define cpu_has_simd      (boot_cpu_feature64(simd) == 0)
#endif

#define cpu_feature32(c, feat)         ((c)->pfr32.feat)
#define boot_cpu_feature32(feat)       (boot_cpu_data.pfr32.feat)

#define cpu_has_aarch32   (boot_cpu_feature32(arm) == 1)
#define cpu_has_thumb     (boot_cpu_feature32(thumb) >= 1)
#define cpu_has_thumb2    (boot_cpu_feature32(thumb) >= 3)
#define cpu_has_jazelle   (boot_cpu_feature32(jazelle) >= 0)
#define cpu_has_thumbee   (boot_cpu_feature32(thumbee) == 1)

#define cpu_has_gentimer  (boot_cpu_feature32(gentimer) == 1)
#define cpu_has_security  (boot_cpu_feature32(security) > 0)

#endif
/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
