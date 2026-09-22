/****************************************************************************
 * arch/sparc/src/ajit1/ajit1-timerisr.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/arch.h>

#include "ajit1-config.h"
#include "sparc_internal.h"

/* qemu ptimer is 40 MHz and one-shot. count = value >> 1. */

static void ajit1_timer_arm(void)
{
  uint32_t count;
  uint32_t hz;

  hz = 1000000 / CONFIG_USEC_PER_TICK;
  count = AJIT1_TIMER_FREQ / hz;
  if (count < 1)
    {
      count = 1;
    }

  putreg32((count << 1) | AJIT1_TIMER_ENABLE, AJIT1_TIMER_BASE);
}

static int ajit1_timerisr(int irq, void *context, void *arg)
{
  UNUSED(irq);
  UNUSED(context);
  UNUSED(arg);

  ajit1_timer_arm();
  nxsched_process_timer();
  return 0;
}

void up_timer_initialize(void)
{
  irq_attach(AJIT1_IRQ_TIMER, ajit1_timerisr, NULL);
  up_enable_irq(AJIT1_IRQ_TIMER);
  ajit1_timer_arm();
}
