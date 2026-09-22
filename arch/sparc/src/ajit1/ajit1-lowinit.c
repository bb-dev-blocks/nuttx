/****************************************************************************
 * arch/sparc/src/ajit1/ajit1-lowinit.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include "ajit1-config.h"
#include "sparc_internal.h"

extern void ajit1_boardinitialize(void);
extern void ajit1_consoleinit(void);

#ifdef CONFIG_DEBUG_FEATURES
#  define showprogress(c) sparc_lowputc(c)
#else
#  define showprogress(c)
#endif

void up_lowinit(void)
{
  uint32_t *dest;

  ajit1_consoleinit();
  showprogress('A');

  for (dest = (uint32_t *)_bss_start; dest < (uint32_t *)_end; )
    {
      *dest++ = 0;
    }

  showprogress('B');

#ifdef USE_EARLYSERIALINIT
  sparc_earlyserialinit();
#endif

  ajit1_boardinitialize();
  showprogress('\n');
}
