/****************************************************************************
 * arch/sparc/src/ajit1/ajit1_smpcall.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <strings.h>

#include <nuttx/arch.h>

#include "ajit1-memorymap.h"
#include "sched/sched.h"
#include "sparc_internal.h"
#include "sparc_v8.h"

#ifdef CONFIG_SMP

extern volatile uint8_t g_cpu_present[4];

static int ajit1_smp_call_handler(int irq, void *context, void *arg)
{
  struct tcb_s *tcb;
  int cpu = this_cpu();

  UNUSED(irq);
  UNUSED(context);
  UNUSED(arg);

  tcb = current_task(cpu);
  sparc_savestate(tcb->xcp.regs);
  nxsched_smp_call_handler(irq, context, arg);
  nxsched_process_delivered(cpu);
  tcb = current_task(cpu);
  sparc_restorestate(tcb->xcp.regs);
  return OK;
}

void ajit1_smp_init(void)
{
  irq_attach(AJIT1_IRQ_IPI, ajit1_smp_call_handler, NULL);
  up_enable_irq(AJIT1_IRQ_IPI);
  putreg32((1u << CONFIG_SMP_NCPUS) - 1,
           AJIT1_INTC_BASE + AJIT1_INTC_IPI_INT_VAL);
}

static void ajit1_ipi(int cpu)
{
  uintptr_t maskreg = AJIT1_INTC_BASE + AJIT1_INTC_IPI_INTR_MASK;
  uint32_t mask;

  if ((unsigned)cpu >= 4 || g_cpu_present[cpu] == 0)
    {
      return;
    }

  mask = getreg32(maskreg);
  putreg32(mask & ~(1u << cpu), maskreg);
  putreg32(mask | (1u << cpu), maskreg);
}

int up_send_smp_sched(int cpu)
{
  ajit1_ipi(cpu);
  return OK;
}

void up_send_smp_call(cpu_set_t cpuset)
{
  int cpu;

  for (; cpuset != 0; cpuset &= ~(1 << cpu))
    {
      cpu = ffs(cpuset) - 1;
      up_send_smp_sched(cpu);
    }
}

#endif
