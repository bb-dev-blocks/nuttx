/****************************************************************************
 * boards/sparc/ajit1/ajit1-qemu/src/ajit1_boot.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_FS_PROCFS
#include <sys/mount.h>
#endif

void ajit1_boardinitialize(void)
{
}

#ifdef CONFIG_BOARD_LATE_INITIALIZE
void board_late_initialize(void)
{
#ifdef CONFIG_FS_PROCFS
  mount(NULL, "/proc", "procfs", 0, NULL);
#endif
}
#endif
