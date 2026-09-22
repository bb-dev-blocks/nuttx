/****************************************************************************
 * arch/sparc/src/ajit1/ajit1-config.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __ARCH_SPARC_SRC_AJIT1_CONFIG_H
#define __ARCH_SPARC_SRC_AJIT1_CONFIG_H

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "chip.h"

#undef HAVE_UART_DEVICE
#ifdef CONFIG_AJIT1_UART
#  define HAVE_UART_DEVICE 1
#endif

#undef HAVE_SERIAL_CONSOLE
#if defined(CONFIG_UART1_SERIAL_CONSOLE) && defined(CONFIG_AJIT1_UART)
#  define HAVE_SERIAL_CONSOLE 1
#else
#  undef CONFIG_UART1_SERIAL_CONSOLE
#endif

#endif /* __ARCH_SPARC_SRC_AJIT1_CONFIG_H */
