#include <xen/config.h>
#include <xen/init.h>
#include <xen/kernel.h>
#include <xen/lib.h>
#include <xen/domain.h>
#include <xen/sched.h>
#include <xen/trace.h>

void __trace_hypercall_entry(void)
{
    struct cpu_user_regs *regs = guest_cpu_user_regs();
    unsigned long args[6];

    if ( is_pv_32on64_vcpu(current) )
    {
        args[0] = regs->ebx;
        args[1] = regs->ecx;
        args[2] = regs->edx;
        args[3] = regs->esi;
        args[4] = regs->edi;
        args[5] = regs->ebp;
    }
    else
    {
        args[0] = regs->rdi;
        args[1] = regs->rsi;
        args[2] = regs->rdx;
        args[3] = regs->r10;
        args[4] = regs->r8;
        args[5] = regs->r9;
    }

    __trace_hypercall(TRC_PV_HYPERCALL_V2, regs->eax, args);
}

void __trace_pv_trap(int trapnr, unsigned long eip,
                     int use_error_code, unsigned error_code)
{
    if ( is_pv_32on64_vcpu(current) )
    {
        struct {
            unsigned eip:32,
                trapnr:15,
                use_error_code:1,
                error_code:16;
        } __attribute__((packed)) d;

        d.eip = eip;
        d.trapnr = trapnr;
        d.error_code = error_code;
        d.use_error_code=!!use_error_code;
                
        __trace_var(TRC_PV_TRAP, 1, sizeof(d), &d);
    }
    else
    {
        struct {
            unsigned long eip;
            unsigned trapnr:15,
                use_error_code:1,
                error_code:16;
        } __attribute__((packed)) d;
        unsigned event;

        d.eip = eip;
        d.trapnr = trapnr;
        d.error_code = error_code;
        d.use_error_code=!!use_error_code;
                
        event = TRC_PV_TRAP;
        event |= TRC_64_FLAG;
        __trace_var(event, 1, sizeof(d), &d);
    }
}

void __trace_pv_page_fault(unsigned long addr, unsigned error_code)
{
    unsigned long eip = guest_cpu_user_regs()->eip;

    if ( is_pv_32on64_vcpu(current) )
    {
        struct {
            u32 eip, addr, error_code;
        } __attribute__((packed)) d;

        d.eip = eip;
        d.addr = addr;
        d.error_code = error_code;
                
        __trace_var(TRC_PV_PAGE_FAULT, 1, sizeof(d), &d);
    }
    else
    {
        struct {
            unsigned long eip, addr;
            u32 error_code;
        } __attribute__((packed)) d;
        unsigned event;

        d.eip = eip;
        d.addr = addr;
        d.error_code = error_code;
        event = TRC_PV_PAGE_FAULT;
        event |= TRC_64_FLAG;
        __trace_var(event, 1, sizeof(d), &d);
    }
}

void __trace_trap_one_addr(unsigned event, unsigned long va)
{
    if ( is_pv_32on64_vcpu(current) )
    {
        u32 d = va;
        __trace_var(event, 1, sizeof(d), &d);
    }
    else
    {
        event |= TRC_64_FLAG;
        __trace_var(event, 1, sizeof(va), &va);
    }
}

void __trace_trap_two_addr(unsigned event, unsigned long va1,
                           unsigned long va2)
{
    if ( is_pv_32on64_vcpu(current) )
    {
        struct {
            u32 va1, va2;
        } __attribute__((packed)) d;
        d.va1=va1;
        d.va2=va2;
        __trace_var(event, 1, sizeof(d), &d);
    }
    else
    {
        struct {
            unsigned long va1, va2;
        } __attribute__((packed)) d;
        d.va1=va1;
        d.va2=va2;
        event |= TRC_64_FLAG;
        __trace_var(event, 1, sizeof(d), &d);
    }
}

void __trace_ptwr_emulation(unsigned long addr, l1_pgentry_t npte)
{
    unsigned long eip = guest_cpu_user_regs()->eip;

    /* We have a couple of different modes to worry about:
     * - 32-on-32: 32-bit pte, 32-bit virtual addresses
     * - pae-on-pae, pae-on-64: 64-bit pte, 32-bit virtual addresses
     * - 64-on-64: 64-bit pte, 64-bit virtual addresses
     * pae-on-64 is the only one that requires extra code; in all other
     * cases, "unsigned long" is the size of a guest virtual address.
     */

    if ( is_pv_32on64_vcpu(current) )
    {
        struct {
            l1_pgentry_t pte;
            u32 addr, eip;
        } __attribute__((packed)) d;
        d.addr = addr;
        d.eip = eip;
        d.pte = npte;

        __trace_var(TRC_PV_PTWR_EMULATION_PAE, 1, sizeof(d), &d);
    }
    else
    {
        struct {
            l1_pgentry_t pte;
            unsigned long addr, eip;
        } d;
        unsigned event;

        d.addr = addr;
        d.eip = eip;
        d.pte = npte;

        event = TRC_PV_PTWR_EMULATION;
        event |= TRC_64_FLAG;
        __trace_var(event, 1/*tsc*/, sizeof(d), &d);
    }
}
