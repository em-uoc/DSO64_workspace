#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include <error.h>
#include <signal.h>
#include <string.h>

// Custom assert 
#define assert(expr, ...) do { \
    int ok = !!(expr); \
    printf("%s '%s' (%s:%d)", (ok?"✅":"❌"), #expr, __FILE__, __LINE__); \
    __VA_OPT__(printf(" -> " __VA_ARGS__);) \
    printf("\n"); \
    fflush(stdout); \
    if (!ok) kill(0, SIGABRT);  \
} while (0)

#define NTESTS  10		// Number of randomly created tests
#define MAX_CH 200		// Max number of channels at each test

int max_ch;			// System-wide limit on the number of channels of each process

//#define __NR_get_nch 602        // syscall code

#define MODULE_NAME "sysdemo"

long
get_nch (pid_t pid)
{
  FILE *f;
  long nch = -1;
  int err_code = 0;

  /* Write target PID to the module parameter in sysfs */
  f = fopen ("/sys/module/" MODULE_NAME "/parameters/target_pid", "w");
  if (!f)
    error (1, 0, "Module %s is not installed?", MODULE_NAME);
  fprintf (f, "%d", pid);
  fclose (f);

  /* Read errno reported by the module */
  f = fopen ("/sys/kernel/" MODULE_NAME "/errno", "r");
  if (!f)
    error (1, 0, "Module %s is not installed?", MODULE_NAME);
  if (fscanf (f, "%d", &err_code) == 1 && err_code != 0)
    {
      errno = err_code;		/* Set global errno (e.g., ESRCH, EINVAL) */
      fclose (f);
      return -1;
    }
  fclose (f);

  /* Read total number of open channels */
  f = fopen ("/sys/kernel/sysdemo/nch", "r");
  if (!f)
    error (1, 0, "Module %s is not installed?", MODULE_NAME);
  if (fscanf (f, "%ld", &nch) == 1)
    {
      fclose (f);
      return nch;
    }
  fclose (f);

  error (1, 0, "Unexpected error");
}


void
test_random (int n)
{
  int n_orig = n;

  switch (fork ())
    {
    case 0:
      // Close standard channels
      close (0);
      close (1);
      close (2);

      if (n)
	{
	  open ("/dev/null", O_RDONLY);	// must return 0
	  int first = rand () % max_ch;

	  if (first != 0)
	    {
	      dup2 (0, first);
	      close (0);
	    }

	  n--;
	  while (n)
	    {
	      int target = rand () % max_ch;
	      // if target channel is unused, take it and decrease "n"
	      if ((fcntl (target, F_GETFD) == -1) && (errno == EBADF))
		{
		  dup2 (first, target);
		  n--;
		}
	    }
	}

      int tmp = get_nch (getpid ());
      int out = open ("/dev/tty", O_WRONLY);	// Reopen terminal for printing
      if (out == -1)
	error (1, errno, "Error reopening /dev/tty");
      if (out != 1)
	{
	  dup2 (out, 1);
	  close (out);
	}

      assert (tmp == n_orig, "tmp=%d, n_orig=%d", tmp, n_orig);
      exit (0);

    default:
      wait (NULL);
    }
}

void
sigabrt (int signo)
{
  char *s = "\nAbort. A test failed \n";
  if (getpid () == getpgrp ())	// Just the leader process prints the message
    write (1, s, strlen (s));

  exit (1);
}

int
main (int argc, char *argv[])
{
  // Deals with test errors
  signal (SIGABRT, sigabrt);

  // Test invalid argument
  assert ((get_nch (-5) == -1)
	  && (errno == EINVAL), "EINVAL=%d, errno=%d", EINVAL, errno);

  // Test myself twice (standard channels)
  assert (get_nch (getpid ()) == 3);
  assert (get_nch (0) == 3);

  pid_t p;
  switch (p = fork ())
    {
    case 0:
      pause ();
    default:
      // Test child process with standard channels
      assert (get_nch (p) == 3);
      kill (p, SIGKILL);
      wait (NULL);
      // Test non-existing process
      assert ((get_nch (p) == -1)
	      && (errno == ESRCH), "ESRCH=%d, errno=%d", ESRCH, errno);
    }

  // Test NTESTS randomly created child processes with up to MAX_CH channels
  max_ch = sysconf (_SC_OPEN_MAX);	// Max number of open files
  if (MAX_CH > max_ch)
    error (1, 0,
	   "MAX_CH (%d) can not be larger than the max number of open files (%d)\n",
	   MAX_CH, max_ch);

  srand (time (NULL));
  int i;
  for (i = 0; i < NTESTS; i++)
    test_random (rand () % MAX_CH);

  // Test no channels
  test_random (0);

  printf ("\nAll tests seem to be OK!\n");
}
