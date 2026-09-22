/****************************************************************************
 * arch/sparc/src/ajit1/ajit1-lowconsole.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include "ajit1-config.h"
#include "sparc_internal.h"

void sparc_lowputc(char ch)
{
#ifdef HAVE_SERIAL_CONSOLE
  uint32_t ctrl;

  ctrl = getreg32(AJIT1_UART_BASE + AJIT1_UART_CTRL);
  ctrl |= AJIT1_UART_TX_ENABLE | AJIT1_UART_RX_ENABLE;
  putreg32(ctrl, AJIT1_UART_BASE + AJIT1_UART_CTRL);

  while ((getreg32(AJIT1_UART_BASE + AJIT1_UART_CTRL) &
          AJIT1_UART_TX_FULL) != 0)
    {
    }

  putreg32((uint32_t)ch, AJIT1_UART_BASE + AJIT1_UART_TX);
#else
  UNUSED(ch);
#endif
}

void ajit1_consoleinit(void)
{
#ifdef HAVE_SERIAL_CONSOLE
  uint32_t ctrl;

  ctrl = getreg32(AJIT1_UART_BASE + AJIT1_UART_CTRL);
  ctrl |= AJIT1_UART_TX_ENABLE | AJIT1_UART_RX_ENABLE;
  putreg32(ctrl, AJIT1_UART_BASE + AJIT1_UART_CTRL);
#endif
}
