/****************************************************************************
 * arch/sparc/include/ajit1/irq.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __ARCH_SPARC_INCLUDE_AJIT1_IRQ_H
#define __ARCH_SPARC_INCLUDE_AJIT1_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/irq.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Trap numbers. External IRQ n uses trap 0x10 + PIL. */

#define AJIT1_IRQ_TIMER              0x1a  /* PIL 10 */
#define AJIT1_IRQ_IPI                0x1b  /* PIL 11 */
#define AJIT1_IRQ_UART               0x1c  /* PIL 12 */
#define AJIT1_IRQ_SW_SYSCALL_TA0     0x80
#define AJIT1_IRQ_SW_SYSCALL_TA8     0x88

#define NR_IRQS                      256

#endif /* __ARCH_SPARC_INCLUDE_AJIT1_IRQ_H */
