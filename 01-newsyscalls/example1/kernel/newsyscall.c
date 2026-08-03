#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0 (new1)
{
  pr_info ("Hello world\n");
  return 27;
}

SYSCALL_DEFINE1 (new2, int, par)
{
  pr_info ("Hello world %d\n", par);
  return par + 1;
}
