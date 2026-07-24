#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>

#include "relatives.h"

#define DEVICE "/dev/numrelatives"

#define MAX_PROCS 10

#define TESTING(x) fprintf(stderr, "Testing %s ...\n", x);

struct infochild
{
  pid_t pid;
  int num;
} childs[MAX_PROCS + 2];

int r2, r4;
int i, j, n;
int fd, fd2;
int pid, last_pid;
int v[MAX_PROCS + 2];
off_t l1;

void
foo (int signo)
{
}

void
killall ()
{
  system ("killall testdriver");
  exit (0);
}

void
test_open ()
{
  TESTING ("Opening modes");
  /* Open device file */
  fd = open (DEVICE, O_WRONLY);
  assert (fd == -1);
  assert (errno == EACCES);

  fd = open (DEVICE, O_RDONLY);
  assert (fd > 0);

  fd2 = open (DEVICE, O_RDONLY);
  assert (fd2 == -1);
  assert (errno == EBUSY);
}

void
create_processes ()
{
  signal (SIGUSR1, foo);
  for (i = 0; i < n; i++)
    {
      switch (childs[i].pid = fork ())
        {
        case -1:
          assert (0);
        case 0:
          // If this check fails, contact your teacher
          assert ((i == 0) || ((childs[i - 1].pid + 1) == getpid ()));

          pause ();             // waits for SIGUSR1

          for (j = 0; j < childs[i].num; j++)
            {
              switch (pid = fork ())
                {
                case -1:
                  assert (0);
                case 0:
                  // If this check fails, contact your teacher
                  assert ((j == 0) || ((last_pid + 1) == getpid ()));
                  pause ();
                  exit (0);
                default:
                  last_pid = pid;
                }
            }
          pause ();
          exit (0);
        }
    }
}

void
test_siblings ()
{
  TESTING ("Number siblings");
  /* Checking number of siblings */
  assert (ioctl (fd, NUM_SIBLINGS) == 0);

  /* one by one */
  for (i = 0; i < n; i++)
    {
      l1 = lseek (fd, childs[i].pid, SEEK_SET);
      assert (l1 == childs[i].pid);
      assert (read (fd, &r4, 1) == 1);
      assert (r4 == n);
    }

  /* all at once  */
  l1 = lseek (fd, childs[0].pid, SEEK_SET);
  assert (l1 == childs[0].pid);
  memset (v, 0xFF, sizeof (v));
  assert (read (fd, &v, n) == n);
  for (i = 0; i < n; i++)
    assert (v[i] == n);
}

void
test_killing ()
{
  TESTING ("killing process");
  j = rand () % n;
  assert (kill (childs[j].pid, SIGKILL) == 0);
  assert (waitpid (childs[j].pid, NULL, 0) == childs[j].pid);

  l1 = lseek (fd, childs[0].pid, SEEK_SET);
  assert (l1 == childs[0].pid);
  memset (v, 0xFF, sizeof (v));
  assert (read (fd, &v, n) == (n - 1));

  for (i = 0; i < n; i++)
    {
      if (i == j)
        assert (v[i] == -1);
      else
        assert (v[i] == (n - 1));
    }
}

void
test_invalid ()
{
  TESTING ("invalid values");

  /* ioctl */
  r2 = ioctl (fd, 40);
  assert (r2 == -1);
  assert (errno == EINVAL);

  /* Testing EFAULT */
  char *addr = sbrk (0);
  l1 = lseek (fd, getpid (), SEEK_SET);
  assert (l1 == getpid ());
  assert (read (fd, addr, 10) == -1);
  assert (errno == EFAULT);

  /* lseek */
  assert (lseek (fd, 0, SEEK_END) == -1);
  assert (errno == EINVAL);

  assert (lseek (fd, -5, SEEK_SET) == -1);
  assert (errno == EINVAL);
}

void
test_children ()
{
  TESTING ("Basic num children");
  /* Getting number of child processes of current process... it must be 0 */
  l1 = lseek (fd, getpid (), SEEK_SET);
  assert (l1 == getpid ());
  assert (read (fd, &r4, 1) == 1);
  assert (r4 == 0);

  create_processes ();

  /* Getting number of child processes of current process... it must be n */
  l1 = lseek (fd, getpid (), SEEK_SET);
  assert (l1 == getpid ());
  assert (read (fd, &r4, 1) == 1);
  assert (r4 == n);

  /* Child processes create new processes */
  for (i = 0; i < n; i++)
    {
      sleep (1);
      TESTING ("awakening child");
      assert (kill (childs[i].pid, SIGUSR1) == 0);
      sleep (1);

      //Read one by one
      l1 = lseek (fd, childs[i].pid, SEEK_SET);
      assert (l1 == childs[i].pid);
      assert (read (fd, &r4, 1) == 1);

      assert (r4 == childs[i].num);
    }

  // Read all at once
  l1 = lseek (fd, childs[0].pid, SEEK_SET);
  assert (l1 == childs[0].pid);
  memset (v, 0xFF, sizeof (v));
  assert (read (fd, v, n) == n);
  for (i = 0; i < n; i++)
    assert (v[i] == childs[i].num);
}

int
main (int argc, char *argv[])
{

  switch (fork ())
    {
    case -1:
      assert (0);

    case 0:
      // Initializations
      signal (SIGABRT, killall);
      srand (getpid ());
      /* random number of child processes and grand-child processes */
      n = rand () % MAX_PROCS + 2;      // number child processes
      for (i = 0; i < n; i++)
        childs[i].num = rand () % MAX_PROCS;    // childs processes of each child

      test_open ();
      test_children ();
      test_siblings ();         // expects processes created by test_children
      test_killing ();          // expects processes created by test_children
      test_invalid ();

      /* If program reaches this point, all tests passed OK */
      printf ("All tests seem to be OK!!! %s %s\n", __DATE__, __TIME__);

      killall ();

    default:
      wait (NULL);
    }
}
