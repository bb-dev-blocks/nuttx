/****************************************************************************
 * boards/sparc/ajit1/ajit1-qemu/src/ajit1_tflite_dir.c
 *
 * Mount a ROMFS at /tflite. The four names are files on that volume.
 * A binfmt accepts only those paths, so the bare name is not a command.
 * Input files live beside the programs and are not linked into the ELF.
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <nuttx/binfmt/binfmt.h>
#include <nuttx/drivers/ramdisk.h>

extern const unsigned char _binary_romfs_img_start[];
extern const unsigned char _binary_romfs_img_end[];

extern int hello_world_main(int argc, char *argv[]);
extern int micro_speech_main(int argc, char *argv[]);
extern int person_detection_main(int argc, char *argv[]);
extern int resnet50_main(int argc, char *argv[]);

struct tflite_prog_s
{
  const char *name;
  main_t entry;
  size_t stack;
};

static const struct tflite_prog_s g_progs[] =
{
  { "hello_world", hello_world_main, 65536 },
  { "micro_speech", micro_speech_main, 262144 },
  { "person_detection", person_detection_main, 262144 },
  { "resnet50", resnet50_main, 262144 },
};

int ajit1_tflite_read(const char *path, void *buf, size_t need)
{
  int fd;
  size_t got = 0;
  char extra;
  ssize_t n;

  fd = open(path, O_RDONLY);
  if (fd < 0)
    {
      return -1;
    }

  while (got < need)
    {
      n = read(fd, (FAR char *)buf + got, need - got);
      if (n <= 0)
        {
          close(fd);
          return -1;
        }

      got += (size_t)n;
    }

  n = read(fd, &extra, 1);
  close(fd);
  if (n != 0)
    {
      return -1;
    }

  return 0;
}

int ajit1_tflite_read_text(const char *path, char *buf, size_t cap)
{
  int fd;
  size_t got = 0;
  ssize_t n;

  if (cap < 2)
    {
      return -1;
    }

  fd = open(path, O_RDONLY);
  if (fd < 0)
    {
      return -1;
    }

  while (got < cap - 1)
    {
      n = read(fd, buf + got, cap - 1 - got);
      if (n < 0)
        {
          close(fd);
          return -1;
        }

      if (n == 0)
        {
          close(fd);
          buf[got] = '\0';
          return (int)got;
        }

      got += (size_t)n;
    }

  n = read(fd, buf, 1);
  close(fd);
  if (n != 0)
    {
      return -1;
    }

  buf[got] = '\0';
  return (int)got;
}

static int tflite_load(FAR struct binary_s *bin, FAR const char *filename,
                       FAR const struct symtab_s *exports, int nexports)
{
  const char *name;
  size_t i;

  if (strncmp(filename, "/tflite/", 8) != 0)
    {
      return -ENOENT;
    }

  name = filename + 8;
  if (name[0] == '\0' || strchr(name, '/') != NULL)
    {
      return -ENOENT;
    }

  for (i = 0; i < sizeof(g_progs) / sizeof(g_progs[0]); i++)
    {
      if (strcmp(name, g_progs[i].name) == 0)
        {
          bin->entrypt = g_progs[i].entry;
          bin->stacksize = g_progs[i].stack;
          bin->priority = 100;
          return OK;
        }
    }

  return -ENOENT;
}

static struct binfmt_s g_tflite_binfmt =
{
  NULL,
  tflite_load,
  NULL,
};

void ajit1_tflite_init(void)
{
  unsigned int len;
  unsigned int nsectors;
  int ret;

  len = (unsigned int)(_binary_romfs_img_end - _binary_romfs_img_start);
  nsectors = (len + 511u) / 512u;
  ret = romdisk_register(0, (FAR uint8_t *)_binary_romfs_img_start, nsectors,
                         512);
  if (ret < 0)
    {
      printf("tflite: romdisk failed: %d\n", ret);
      return;
    }

  if (mkdir("/tflite", 0755) < 0 && errno != EEXIST)
    {
      printf("tflite: mkdir /tflite failed: %d\n", errno);
      return;
    }

  if (mount("/dev/ram0", "/tflite", "romfs", 0, NULL) < 0)
    {
      printf("tflite: mount failed: %d\n", errno);
      return;
    }

  register_binfmt(&g_tflite_binfmt);
}
