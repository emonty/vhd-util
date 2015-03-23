########################################
# x86-specific definitions

HAS_IOPORTS := y
HAS_ACPI := y
HAS_VGA  := y
HAS_VIDEO  := y
HAS_CPUFREQ := y
HAS_PCI := y
HAS_PASSTHROUGH := y
HAS_NS16550 := y
HAS_EHCI := y
HAS_KEXEC := y
HAS_GDBSX := y
xenoprof := y

#
# If you change any of these configuration options then you must
# 'make clean' before rebuilding.
#
supervisor_mode_kernel ?= n

CFLAGS += -I$(BASEDIR)/include 
CFLAGS += -I$(BASEDIR)/include/asm-x86/mach-generic
CFLAGS += -I$(BASEDIR)/include/asm-x86/mach-default

# Prevent floating-point variables from creeping into Xen.
CFLAGS += -msoft-float

$(call cc-options-add,CFLAGS,CC,$(EMBEDDED_EXTRA_CFLAGS))
$(call cc-option-add,CFLAGS,CC,-Wnested-externs)
$(call as-insn-check,CFLAGS,CC,"vmcall",-DHAVE_GAS_VMX)
$(call as-insn-check,CFLAGS,CC,"invept (%rax)$$(comma)%rax",-DHAVE_GAS_EPT)
$(call as-insn-check,CFLAGS,CC,"rdfsbase %rax",-DHAVE_GAS_FSGSBASE)

ifeq ($(supervisor_mode_kernel),y)
CFLAGS += -DCONFIG_X86_SUPERVISOR_MODE_KERNEL=1
endif

x86 := y
x86_32 := n
x86_64 := y

CFLAGS += -mno-red-zone -mno-sse -fpic
CFLAGS += -fno-asynchronous-unwind-tables
# -fvisibility=hidden reduces -fpic cost, if it's available
ifneq ($(call cc-option,$(CC),-fvisibility=hidden,n),n)
CFLAGS += -DGCC_HAS_VISIBILITY_ATTRIBUTE
endif
