#include <stddef.h>
#include <stdint.h>

extern "C" void free(void *);
extern "C" int *__errno(void);
extern "C" void *__dso_handle = 0;

extern "C" int *__errno_location(void) {
  return __errno();
}

// eyalroz printf_ (pulled in by microlite) writes through this hook.
extern "C" void putchar_(char c)
{
  volatile uint32_t *ctrl = (volatile uint32_t *)0xFFFF3200u;
  volatile uint8_t *tx = (volatile uint8_t *)0xFFFF3204u;

  *ctrl = 3;
  while ((*ctrl & 8u) != 0)
    {
    }
  *tx = (uint8_t)c;
}

void operator delete(void *p, size_t) noexcept
{
  free(p);
}

void operator delete(void *p, unsigned long) noexcept
{
  free(p);
}

void operator delete[](void *p, size_t) noexcept
{
  free(p);
}

void operator delete[](void *p, unsigned long) noexcept
{
  free(p);
}
