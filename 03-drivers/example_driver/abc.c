/*
 * Example driver
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

#define DRIVER_MAJOR 231
#define DRIVER_NAME "abc"

MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("Example driver");

char alphabet[] =
  { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

static int
do_open (struct inode *inode, struct file *filp)
{
  /* Just O_RDONLY mode, regardless funny mask bits */
  if ((filp->f_flags & O_ACCMODE) != O_RDONLY)
    return -EACCES;

  return 0;
}

static ssize_t
do_read (struct file *filp, char __user * buf, size_t count, loff_t * f_pos)
{
  loff_t real_read;

  /* The number of bytes to be read must be >= 0 */
  if (count < 0)
    return -EINVAL;

  /* If beyond end of file, returns 0 */
  if (*f_pos >= sizeof (alphabet))
    return 0;

  /* Driver limits the maximum number of characters to be read simultaneously */
  if (count > 3)
    count = 3;

  /* Computes the real number of characters that can be read */
  if ((*f_pos + count) < sizeof (alphabet))
    real_read = count;
  else
    real_read = sizeof (alphabet) - *f_pos;

  /* Checks access rights over the range of memory addresses that will be modified */
  /* This check has already been performed by vfs_read() from buff to buff+count: access_ok(buf, count) */
  /* Consequently, in abc-driver, this check is unnecessary */
  if (!access_ok (buf, real_read))
    return -EFAULT;

  /* Transfers data to user space */
  if (raw_copy_to_user (buf, alphabet + *f_pos, real_read) != 0)
    return -EFAULT;

  /* Updates file pointer */
  *f_pos += real_read;

  /* Returns number of read characters */
  return real_read;
}

static ssize_t
do_write (struct file *filp, const char __user * buf, size_t count,
	  loff_t * f_pos)
{
  return 0;
}

/* close system call */
static int
do_release (struct inode *inode, struct file *filp)
{
  return 0;
}

static long
do_ioctl (struct file *filp, u_int cmd, u_long arg)
{
  return 0;
}

static loff_t
do_llseek (struct file *file, loff_t offset, int orig)
{
  loff_t ret;

  switch (orig)
    {
    case SEEK_SET:
      ret = offset;
      break;
    case SEEK_CUR:
      ret = file->f_pos + offset;
      break;
    case SEEK_END:
      ret = sizeof (alphabet) + offset;
      break;
    default:
      ret = -EINVAL;
    }

  if (ret >= 0)
    file->f_pos = ret;
  else
    ret = -EINVAL;

  return ret;
}

struct file_operations abc_op = {
  .open = do_open,
  .read = do_read,
  .write = do_write,
  .release = do_release,	/* close system call */
  .unlocked_ioctl = do_ioctl,
  .llseek = do_llseek
};

static int __init
abc_init (void)
{
  int result;

  result = register_chrdev (DRIVER_MAJOR, DRIVER_NAME, &abc_op);
  if (result < 0)
    {
      pr_err ("Unable to register device\n");
      return result;
    }

  pr_info ("abc driver correctly installed\n");
  return (0);
}

static void __exit
abc_cleanup (void)
{
  unregister_chrdev (DRIVER_MAJOR, DRIVER_NAME);
  pr_info ("abc Cleanup successful\n");
}

module_init (abc_init);
module_exit (abc_cleanup);
