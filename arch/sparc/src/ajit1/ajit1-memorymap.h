/****************************************************************************
 * arch/sparc/src/ajit1/ajit1-memorymap.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __ARCH_SPARC_SRC_AJIT1_MEMORYMAP_H
#define __ARCH_SPARC_SRC_AJIT1_MEMORYMAP_H

/* qemu-ajit machine ajit1_generic */

#define AJIT1_INTC_BASE          0xffff3000
#define AJIT1_TIMER_BASE         0xffff3100
#define AJIT1_UART_BASE          0xffff3200

#define AJIT1_UART_CTRL          0x00
#define AJIT1_UART_TX            0x04
#define AJIT1_UART_RX            0x08

#define AJIT1_UART_TX_ENABLE     (1 << 0)
#define AJIT1_UART_RX_ENABLE     (1 << 1)
#define AJIT1_UART_RX_INTERRUPT  (1 << 2)
#define AJIT1_UART_TX_FULL       (1 << 3)
#define AJIT1_UART_RX_FULL       (1 << 4)

#define AJIT1_TIMER_CTRL         0x00
#define AJIT1_TIMER_ENABLE       (1 << 0)
#define AJIT1_TIMER_FREQ         40000000

/* INTC control at offset 0 is the CPU that performs the access.
 * Bit 0 enable, bits 15:1 = PIL mask.
 */

#define AJIT1_INTC_ENABLE        (1 << 0)
#define AJIT1_INTC_IPI_INTR_MASK 0x80
#define AJIT1_INTC_IPI_INT_VAL   0x84
#define AJIT1_IPI_PIL            11

#endif /* __ARCH_SPARC_SRC_AJIT1_MEMORYMAP_H */
