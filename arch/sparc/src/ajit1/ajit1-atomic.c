/****************************************************************************
 * arch/sparc/src/ajit1/ajit1-atomic.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * sparc-linux-gcc -mcpu=v8 calls these instead of CASA. libatomic's
 * versions take pthread mutexes, which trap before the syscall handler
 * is installed. ldstub is SPARC V8 and does not need the OS.
 * ponytail: one global lock. Per-address locks if this shows up in profiles.
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

static uint8_t g_atomic_lock;

static void ajit1_atomic_lock(void)
{
  uint8_t val;

  do
    {
      __asm__ volatile ("ldstub [%1], %0"
                        : "=r" (val)
                        : "r" (&g_atomic_lock)
                        : "memory");
    }
  while (val != 0);
}

static void ajit1_atomic_unlock(void)
{
  __asm__ volatile ("stb %%g0, [%0]"
                    :
                    : "r" (&g_atomic_lock)
                    : "memory");
}

bool __atomic_compare_exchange_4(volatile void *mem, void *expect,
                                 uint32_t desired, bool weak,
                                 int success, int failure)
{
  volatile uint32_t *ptr = mem;
  uint32_t *exp = expect;
  uint32_t old;
  bool ok;

  (void)weak;
  (void)success;
  (void)failure;

  ajit1_atomic_lock();
  old = *ptr;
  ok = (old == *exp);
  if (ok)
    {
      *ptr = desired;
    }
  else
    {
      *exp = old;
    }

  ajit1_atomic_unlock();
  return ok;
}

static uint32_t ajit1_fetch_op(volatile void *mem, uint32_t val, int op)
{
  volatile uint32_t *ptr = mem;
  uint32_t old;

  ajit1_atomic_lock();
  old = *ptr;
  switch (op)
    {
      case 0:
        *ptr = old + val;
        break;

      case 1:
        *ptr = old - val;
        break;

      case 2:
        *ptr = old & val;
        break;

      default:
        *ptr = old | val;
        break;
    }

  ajit1_atomic_unlock();
  return old;
}

uint32_t __atomic_fetch_add_4(volatile void *mem, uint32_t val, int memorder)
{
  (void)memorder;
  return ajit1_fetch_op(mem, val, 0);
}

uint32_t __atomic_fetch_sub_4(volatile void *mem, uint32_t val, int memorder)
{
  (void)memorder;
  return ajit1_fetch_op(mem, val, 1);
}

uint32_t __atomic_fetch_and_4(volatile void *mem, uint32_t val, int memorder)
{
  (void)memorder;
  return ajit1_fetch_op(mem, val, 2);
}

uint32_t __atomic_fetch_or_4(volatile void *mem, uint32_t val, int memorder)
{
  (void)memorder;
  return ajit1_fetch_op(mem, val, 3);
}
