#ifndef _XEN_P2M_H
#define _XEN_P2M_H

#include <xen/mm.h>

struct domain;

/* Per-p2m-table state */
struct p2m_domain {
    /* Lock that protects updates to the p2m */
    spinlock_t lock;

    /* Pages used to construct the p2m */
    struct page_list_head pages;

    /* Root of p2m page tables, 2 contiguous pages */
    struct page_info *first_level;

    /* Current VMID in use */
    uint8_t vmid;

    /* Highest guest frame that's ever been mapped in the p2m
     * Only takes into account ram and foreign mapping
     */
    unsigned long max_mapped_gfn;

    /* Lowest mapped gfn in the p2m. When releasing mapped gfn's in a
     * preemptible manner this is update to track recall where to
     * resume the search. Apart from during teardown this can only
     * decrease. */
    unsigned long lowest_mapped_gfn;
};

/* List of possible type for each page in the p2m entry.
 * The number of available bit per page in the pte for this purpose is 4 bits.
 * So it's possible to only have 16 fields. If we run out of value in the
 * future, it's possible to use higher value for pseudo-type and don't store
 * them in the p2m entry.
 */
typedef enum {
    p2m_invalid = 0,    /* Nothing mapped here */
    p2m_ram_rw,         /* Normal read/write guest RAM */
    p2m_ram_ro,         /* Read-only; writes are silently dropped */
    p2m_mmio_direct,    /* Read/write mapping of genuine MMIO area */
    p2m_map_foreign,    /* Ram pages from foreign domain */
    p2m_grant_map_rw,   /* Read/write grant mapping */
    p2m_grant_map_ro,   /* Read-only grant mapping */
    p2m_max_real_type,  /* Types after this won't be store in the p2m */
} p2m_type_t;

#define p2m_is_foreign(_t)  ((_t) == p2m_map_foreign)
#define p2m_is_ram(_t)      ((_t) == p2m_ram_rw || (_t) == p2m_ram_ro)

/* Initialise vmid allocator */
void p2m_vmid_allocator_init(void);

/* Init the datastructures for later use by the p2m code */
int p2m_init(struct domain *d);

/* Return all the p2m resources to Xen. */
void p2m_teardown(struct domain *d);

/* Remove mapping refcount on each mapping page in the p2m
 *
 * TODO: For the moment only foreign mappings are handled
 */
int relinquish_p2m_mapping(struct domain *d);

/* Allocate a new p2m table for a domain.
 *
 * Returns 0 for success or -errno.
 */
int p2m_alloc_table(struct domain *d);

/* */
void p2m_load_VTTBR(struct domain *d);

/* Look up the MFN corresponding to a domain's PFN. */
paddr_t p2m_lookup(struct domain *d, paddr_t gpfn, p2m_type_t *t);

/* Clean & invalidate caches corresponding to a region of guest address space */
int p2m_cache_flush(struct domain *d, xen_pfn_t start_mfn, xen_pfn_t end_mfn);

/* Setup p2m RAM mapping for domain d from start-end. */
int p2m_populate_ram(struct domain *d, paddr_t start, paddr_t end);
/* Map MMIO regions in the p2m: start_gaddr and end_gaddr is the range
 * in the guest physical address space to map, starting from the machine
 * address maddr. */
int map_mmio_regions(struct domain *d, paddr_t start_gaddr,
                     paddr_t end_gaddr, paddr_t maddr);

int guest_physmap_add_entry(struct domain *d,
                            unsigned long gfn,
                            unsigned long mfn,
                            unsigned long page_order,
                            p2m_type_t t);

/* Untyped version for RAM only, for compatibility */
static inline int guest_physmap_add_page(struct domain *d,
                                         unsigned long gfn,
                                         unsigned long mfn,
                                         unsigned int page_order)
{
    return guest_physmap_add_entry(d, gfn, mfn, page_order, p2m_ram_rw);
}

void guest_physmap_remove_page(struct domain *d,
                               unsigned long gpfn,
                               unsigned long mfn, unsigned int page_order);

unsigned long gmfn_to_mfn(struct domain *d, unsigned long gpfn);

/*
 * Populate-on-demand
 */

/* Call when decreasing memory reservation to handle PoD entries properly.
 * Will return '1' if all entries were handled and nothing more need be done.*/
int
p2m_pod_decrease_reservation(struct domain *d,
                             xen_pfn_t gpfn,
                             unsigned int order);

/* Look up a GFN and take a reference count on the backing page. */
typedef unsigned int p2m_query_t;
#define P2M_ALLOC    (1u<<0)   /* Populate PoD and paged-out entries */
#define P2M_UNSHARE  (1u<<1)   /* Break CoW sharing */

static inline struct page_info *get_page_from_gfn(
    struct domain *d, unsigned long gfn, p2m_type_t *t, p2m_query_t q)
{
    struct page_info *page;
    p2m_type_t p2mt;
    paddr_t maddr = p2m_lookup(d, pfn_to_paddr(gfn), &p2mt);
    unsigned long mfn = maddr >> PAGE_SHIFT;

    if (t)
        *t = p2mt;

    if ( p2mt == p2m_invalid || p2mt == p2m_mmio_direct )
        return NULL;

    if ( !mfn_valid(mfn) )
        return NULL;
    page = mfn_to_page(mfn);

    /* get_page won't work on foreign mapping because the page doesn't
     * belong to the current domain.
     */
    if ( p2mt == p2m_map_foreign )
    {
        struct domain *fdom = page_get_owner_and_reference(page);
        ASSERT(fdom != NULL);
        ASSERT(fdom != d);
        return page;
    }

    if ( !get_page(page, d) )
        return NULL;
    return page;
}

int get_page_type(struct page_info *page, unsigned long type);
int is_iomem_page(unsigned long mfn);
static inline int get_page_and_type(struct page_info *page,
                                    struct domain *domain,
                                    unsigned long type)
{
    int rc = get_page(page, domain);

    if ( likely(rc) && unlikely(!get_page_type(page, type)) )
    {
        put_page(page);
        rc = 0;
    }

    return rc;
}

#endif /* _XEN_P2M_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
