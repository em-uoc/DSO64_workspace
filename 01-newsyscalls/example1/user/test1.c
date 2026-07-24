#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define SYSCALL1	600
#define SYSCALL2	601

// Wrappers for new systemcalls
long int newsyscall1()
{
  return syscall(SYSCALL1);
}

long int newsyscall2(int level)
{
  return syscall(SYSCALL2, level);
}

int main (int argc, char *argv[])
{
  long int ret;

  printf ("%ld\n", newsyscall1());

  if (argc==1) ret = newsyscall2(123); else ret = newsyscall2(atoi(argv[1]));
  if (ret>=0) printf("ret=%ld\n", ret);
  else printf("ret=%ld errno=%d\n", ret, errno);
}
