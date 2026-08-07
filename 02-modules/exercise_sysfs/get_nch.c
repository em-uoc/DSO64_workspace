#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>

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
