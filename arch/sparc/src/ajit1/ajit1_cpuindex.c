/****************************************************************************
 * arch/sparc/src/ajit1/ajit1_cpuindex.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <nuttx/arch.h>

#ifdef CONFIG_ARCH_HAVE_MULTICPU

/* qemu ASR29: 0x5052_0c0t with THREADS_PER_CORE 2. Index = core * 2 + thread. */

int up_cpu_index(void)
{
  uint32_t asr;
  unsigned int core;
  unsigned int thread;

  __asm__ __volatile__
  (
    "rd %%asr29, %0\n"
    : "=r" (asr)
  );

  thread = asr & 0xff;
  core = (asr >> 8) & 0xff;
  return (int)(core * 2 + thread);
}

#endif
