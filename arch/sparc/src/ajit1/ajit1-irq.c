/****************************************************************************
 * arch/sparc/src/ajit1/ajit1-irq.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include "ajit1-config.h"
#include "sparc_internal.h"

#if CONFIG_ARCH_INTERRUPTSTACK > 7
#if defined(CONFIG_SMP)
#  define INTSTACK_ALLOC (CONFIG_SMP_NCPUS * INTSTACK_SIZE)
#else
#  define INTSTACK_ALLOC (INTSTACK_SIZE)
#endif

static uint64_t g_intstack_alloc[INTSTACK_ALLOC >> 3];

uintptr_t g_cpu_intstack_top[CONFIG_SMP_NCPUS] =
{
  (uintptr_t)g_intstack_alloc + INTSTACK_SIZE,
#if defined(CONFIG_SMP)
#if CONFIG_SMP_NCPUS > 1
  (uintptr_t)g_intstack_alloc + (2 * INTSTACK_SIZE),
#if CONFIG_SMP_NCPUS > 2
  (uintptr_t)g_intstack_alloc + (3 * INTSTACK_SIZE),
#if CONFIG_SMP_NCPUS > 3
  (uintptr_t)g_intstack_alloc + (4 * INTSTACK_SIZE),
#endif
#endif
#endif
#endif
};
#endif

/* PILs enabled on the CPU that calls up_enable_irq. */

static uint32_t g_pil_mask;

#ifdef CONFIG_SMP
void ajit1_smp_init(void);
#endif

static void ajit1_intc_commit(void)
{
  uint32_t val = 0;

  if (g_pil_mask != 0)
    {
      val = g_pil_mask | AJIT1_INTC_ENABLE;
    }

  putreg32(val, AJIT1_INTC_BASE);
}

void up_irqinitialize(void)
{
  g_pil_mask = 0;
  putreg32(0, AJIT1_INTC_BASE);

  irq_attach(AJIT1_IRQ_SW_SYSCALL_TA0, sparc_swint0, NULL);
  irq_attach(AJIT1_IRQ_SW_SYSCALL_TA8, sparc_swint1, NULL);

#ifdef CONFIG_SMP
  ajit1_smp_init();
#endif

#ifndef CONFIG_SUPPRESS_INTERRUPTS
  up_irq_enable();
#endif
}

void up_enable_irq(int irq)
{
  unsigned int pil;

  if ((unsigned)irq < 0x11 || (unsigned)irq > 0x1f)
    {
      return;
    }

  pil = (unsigned)irq - 0x10;
  g_pil_mask |= 1u << pil;
  ajit1_intc_commit();
}

void up_disable_irq(int irq)
{
  unsigned int pil;

  if ((unsigned)irq < 0x11 || (unsigned)irq > 0x1f)
    {
      return;
    }

  pil = (unsigned)irq - 0x10;
  g_pil_mask &= ~(1u << pil);
  ajit1_intc_commit();
}

void sparc_clrpend_irq(int irq)
{
  UNUSED(irq);
}

bool sparc_pending_irq(int irq)
{
  UNUSED(irq);
  return false;
}

#if CONFIG_ARCH_INTERRUPTSTACK > 7
uintptr_t up_get_intstackbase(int cpu)
{
  return g_cpu_intstack_top[cpu] - INTSTACK_SIZE;
}
#endif
