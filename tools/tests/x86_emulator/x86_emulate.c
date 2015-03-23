#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xen/xen.h>

typedef bool bool_t;

#define BUG() abort()
#define ASSERT assert

#define cpu_has_amd_erratum(nr) 0
#define mark_regs_dirty(r) ((void)(r))

#include "x86_emulate/x86_emulate.h"
#include "x86_emulate/x86_emulate.c"
