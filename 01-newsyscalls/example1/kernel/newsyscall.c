#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_new1 (void)
{
  pr_info ("Hello world\n");
  return 27;
}

SYSCALL_DEFINE0 (new1)
{
  return sys_new1 ();
}

asmlinkage long sys_new2 (int par)
{
  pr_info ("Hello world %d\n", par);
  return par + 1;
}

SYSCALL_DEFINE1 (new2, int, par)
{
  return sys_new2 (par);
}
