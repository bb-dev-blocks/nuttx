/****************************************************************************
 * arch/sparc/src/ajit1/ajit1_cpuidlestack.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

#include <nuttx/arch.h>
#include <nuttx/sched.h>

#include "sparc_internal.h"

#ifdef CONFIG_SMP

#define SMP_STACK_MASK  7
#define SMP_STACK_SIZE  ((CONFIG_IDLETHREAD_STACKSIZE + 7) & ~7)
#define STACK_ISALIGNED(a) ((uintptr_t)(a) & ~SMP_STACK_MASK)

extern uint32_t g_cpu0_idlestack[];
extern uint32_t g_cpu1_idlestack[];
#if CONFIG_SMP_NCPUS > 2
extern uint32_t g_cpu2_idlestack[];
#endif
#if CONFIG_SMP_NCPUS > 3
extern uint32_t g_cpu3_idlestack[];
#endif

static const uint32_t *g_cpu_stackalloc[CONFIG_SMP_NCPUS] =
{
  g_cpu0_idlestack,
  g_cpu1_idlestack,
#if CONFIG_SMP_NCPUS > 2
  g_cpu2_idlestack,
#endif
#if CONFIG_SMP_NCPUS > 3
  g_cpu3_idlestack,
#endif
};

int up_cpu_idlestack(int cpu, struct tcb_s *tcb, size_t stack_size)
{
  uintptr_t stack_alloc;

  DEBUGASSERT(cpu > 0 && cpu < CONFIG_SMP_NCPUS && tcb != NULL &&
              stack_size <= SMP_STACK_SIZE);

  stack_alloc = (uintptr_t)g_cpu_stackalloc[cpu];
  DEBUGASSERT(stack_alloc != 0 && STACK_ISALIGNED(stack_alloc));

  tcb->adj_stack_size = SMP_STACK_SIZE;
  tcb->stack_alloc_ptr = (void *)stack_alloc;
  tcb->stack_base_ptr = tcb->stack_alloc_ptr;
  return OK;
}

#endif
