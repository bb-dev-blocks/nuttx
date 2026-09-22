/****************************************************************************
 * arch/sparc/src/ajit1/ajit1_cpustart.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <nuttx/arch.h>
#include <nuttx/sched.h>

#include "ajit1-memorymap.h"
#include "init/init.h"
#include "sched/sched.h"
#include "sparc_internal.h"

#ifdef CONFIG_SMP

/* Head.S places these in .data. CPU0's BSS clear does not touch them.
 * ponytail: one image, CONFIG_SMP_NCPUS 4. QEMU -smp may be smaller.
 * Absent CPUs never set their present byte; they are not scheduled.
 */

extern volatile uint8_t g_cpu_present[4];
extern volatile uint8_t g_cpu_release[4];

static volatile bool g_cpu_boot;
static struct tcb_s g_offline_tcb;

static void ajit1_mark_offline(int cpu)
{
  if (g_offline_tcb.pid == 0)
    {
      g_offline_tcb.pid = CONFIG_SMP_NCPUS;
      g_offline_tcb.sched_priority = SCHED_PRIORITY_MAX;
    }

  /* select_cpu looks at g_assignedtasks, not g_running_tasks.
   * ponytail: one shared TCB. Fine while nothing writes per-CPU
   * state into it. Give each absent CPU its own TCB if that shows up.
   */

  g_assignedtasks[cpu] = &g_offline_tcb;
  g_running_tasks[cpu] = &g_offline_tcb;
  g_idletcb[cpu].task_state = TSTATE_TASK_INACTIVE;
}

void ajit1_cpu_boot(void)
{
  uint32_t ctrl;

  ctrl = AJIT1_INTC_ENABLE | (1u << AJIT1_IPI_PIL);
  putreg32(ctrl, AJIT1_INTC_BASE);

  {
    int cpu = this_cpu();
    char msg[] = "CPU0 online\r\n";

    msg[3] = '0' + cpu;

    for (char *p = msg; *p != '\0'; p++)
      {
        sparc_lowputc(*p);
      }
  }

  g_cpu_boot = true;

#ifndef CONFIG_SUPPRESS_INTERRUPTS
  up_irq_enable();
#endif

  nx_idle_trampoline();
}

int up_cpu_start(int cpu)
{
  if (g_cpu_present[cpu] == 0)
    {
      ajit1_mark_offline(cpu);
      return 0;
    }

  g_cpu_boot = false;
  g_cpu_release[cpu] = 1;

  while (!g_cpu_boot)
    {
    }

  return 0;
}

#endif
