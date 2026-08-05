#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>

#define BUFSIZE 128

int
main (int argc, char *argv[])
{
  int fd, channel, nbytes;
  char buf[BUFSIZE] = { 0 };

  if (argc != 3)
    error (1, 0, "Wrong arguments. Expected arguments: channel nbytes");

  channel = atoi (argv[1]);
  nbytes = atoi (argv[2]);

  int max_ch = sysconf (_SC_OPEN_MAX);

  if ((channel < 0) || (channel >= max_ch))
    error (1, 0, "Wrong argument: channel out of limits (0...%d)",
           max_ch - 1);
  if (nbytes < 0)
    error (1, 0, "Wrong argument: nbytes can not be negative");

  if ((fd = open ("/dev/null", O_WRONLY)) < 0)
    error (1, errno, "Error opening output file");

  if (fd != channel)
    {
      if (dup2 (fd, channel) < 0)
        error (1, errno, "Error dup2");
      close (fd);
    }

  while (nbytes >= BUFSIZE)
    {
      if (write (channel, buf, BUFSIZE) != BUFSIZE)
        error (1, errno, "Error write()");
      nbytes -= BUFSIZE;
    }

  if (nbytes)
    if (write (channel, buf, nbytes) != nbytes)
      error (1, errno, "Error partial write()");

  close (channel);
}
