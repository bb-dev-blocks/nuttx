/****************************************************************************
 * arch/sparc/src/ajit1/ajit1-serial.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <stdint.h>

#include <nuttx/irq.h>
#include <nuttx/serial/serial.h>

#include "ajit1-config.h"
#include "sparc_internal.h"

#ifdef USE_SERIALDRIVER

#ifdef HAVE_UART_DEVICE

static int  up_setup(struct uart_dev_s *dev);
static void up_shutdown(struct uart_dev_s *dev);
static int  up_attach(struct uart_dev_s *dev);
static void up_detach(struct uart_dev_s *dev);
static int  up_ioctl(struct file *filep, int cmd, unsigned long arg);
static int  up_receive(struct uart_dev_s *dev, uint32_t *status);
static void up_rxint(struct uart_dev_s *dev, bool enable);
static bool up_rxavailable(struct uart_dev_s *dev);
static void up_send(struct uart_dev_s *dev, int ch);
static void up_txint(struct uart_dev_s *dev, bool enable);
static bool up_txready(struct uart_dev_s *dev);
static bool up_txempty(struct uart_dev_s *dev);

static const struct uart_ops_s g_uart_ops =
{
  .setup       = up_setup,
  .shutdown    = up_shutdown,
  .attach      = up_attach,
  .detach      = up_detach,
  .ioctl       = up_ioctl,
  .receive     = up_receive,
  .rxint       = up_rxint,
  .rxavailable = up_rxavailable,
  .send        = up_send,
  .txint       = up_txint,
  .txready     = up_txready,
  .txempty     = up_txempty,
};

static char g_rxbuffer[CONFIG_UART1_RXBUFSIZE];
static char g_txbuffer[CONFIG_UART1_TXBUFSIZE];

static struct uart_dev_s g_uartport =
{
  .recv =
  {
    .size   = CONFIG_UART1_RXBUFSIZE,
    .buffer = g_rxbuffer,
  },
  .xmit =
  {
    .size   = CONFIG_UART1_TXBUFSIZE,
    .buffer = g_txbuffer,
  },
  .ops  = &g_uart_ops,
  .priv = NULL,
};

static int up_interrupt(int irq, void *context, void *arg)
{
  struct uart_dev_s *dev = arg;

  UNUSED(irq);
  UNUSED(context);

  if (up_rxavailable(dev))
    {
      uart_recvchars(dev);
    }

  return OK;
}

static int up_setup(struct uart_dev_s *dev)
{
  uint32_t ctrl;

  UNUSED(dev);

  ctrl = getreg32(AJIT1_UART_BASE + AJIT1_UART_CTRL);
  ctrl |= AJIT1_UART_TX_ENABLE | AJIT1_UART_RX_ENABLE;
  putreg32(ctrl, AJIT1_UART_BASE + AJIT1_UART_CTRL);
  return OK;
}

static void up_shutdown(struct uart_dev_s *dev)
{
  up_rxint(dev, false);
}

static int up_attach(struct uart_dev_s *dev)
{
  int ret;

  ret = irq_attach(AJIT1_IRQ_UART, up_interrupt, dev);
  if (ret == OK)
    {
      up_enable_irq(AJIT1_IRQ_UART);
    }

  return ret;
}

static void up_detach(struct uart_dev_s *dev)
{
  UNUSED(dev);
  up_disable_irq(AJIT1_IRQ_UART);
  irq_detach(AJIT1_IRQ_UART);
}

static int up_ioctl(struct file *filep, int cmd, unsigned long arg)
{
  UNUSED(filep);
  UNUSED(cmd);
  UNUSED(arg);
  return -ENOTTY;
}

static int up_receive(struct uart_dev_s *dev, uint32_t *status)
{
  UNUSED(dev);

  if (status)
    {
      *status = 0;
    }

  return (int)(getreg32(AJIT1_UART_BASE + AJIT1_UART_RX) & 0xff);
}

static void up_rxint(struct uart_dev_s *dev, bool enable)
{
  uint32_t ctrl;

  ctrl = getreg32(AJIT1_UART_BASE + AJIT1_UART_CTRL);
  if (enable)
    {
      ctrl |= AJIT1_UART_RX_INTERRUPT;
      putreg32(ctrl, AJIT1_UART_BASE + AJIT1_UART_CTRL);
      up_enable_irq(AJIT1_IRQ_UART);
      if (up_rxavailable(dev))
        {
          uart_recvchars(dev);
        }
    }
  else
    {
      ctrl &= ~AJIT1_UART_RX_INTERRUPT;
      putreg32(ctrl, AJIT1_UART_BASE + AJIT1_UART_CTRL);
    }
}

static bool up_rxavailable(struct uart_dev_s *dev)
{
  UNUSED(dev);
  return (getreg32(AJIT1_UART_BASE + AJIT1_UART_CTRL) &
          AJIT1_UART_RX_FULL) != 0;
}

static void up_send(struct uart_dev_s *dev, int ch)
{
  UNUSED(dev);
  sparc_lowputc(ch);
}

static void up_txint(struct uart_dev_s *dev, bool enable)
{
  if (enable)
    {
      uart_xmitchars(dev);
    }
}

static bool up_txready(struct uart_dev_s *dev)
{
  UNUSED(dev);
  return (getreg32(AJIT1_UART_BASE + AJIT1_UART_CTRL) &
          AJIT1_UART_TX_FULL) == 0;
}

static bool up_txempty(struct uart_dev_s *dev)
{
  return up_txready(dev);
}

void sparc_earlyserialinit(void)
{
#ifdef HAVE_SERIAL_CONSOLE
  g_uartport.isconsole = true;
  up_setup(&g_uartport);
#endif
}

void sparc_serialinit(void)
{
#ifdef HAVE_SERIAL_CONSOLE
  uart_register("/dev/console", &g_uartport);
#endif
  uart_register("/dev/ttyS0", &g_uartport);
}

void up_putc(int ch)
{
#ifdef HAVE_SERIAL_CONSOLE
  if (ch == '\n')
    {
      sparc_lowputc('\r');
    }

  sparc_lowputc(ch);
#else
  UNUSED(ch);
#endif
}

#else /* HAVE_UART_DEVICE */

void sparc_earlyserialinit(void)
{
}

void sparc_serialinit(void)
{
}

void up_putc(int ch)
{
  UNUSED(ch);
}

#endif /* HAVE_UART_DEVICE */

#else /* USE_SERIALDRIVER */

void up_putc(int ch)
{
#ifdef HAVE_SERIAL_CONSOLE
  sparc_lowputc(ch);
#else
  UNUSED(ch);
#endif
}

#endif /* USE_SERIALDRIVER */
