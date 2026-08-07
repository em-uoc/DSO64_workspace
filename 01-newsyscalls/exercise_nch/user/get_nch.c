#include <unistd.h>

#define __NR_get_nch 602        // syscall code

long
get_nch (pid_t pid)
{
  return syscall (__NR_get_nch, pid);
}

