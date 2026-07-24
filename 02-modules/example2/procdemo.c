/*
 * /proc example module
 */

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("/proc demo");

/* /proc interface to the module */
#define PROC_ENTRY "procdemo"	/* File name */

static pid_t pid = 0;

/* Read: Shows information about process "pid" */
/* Information is written using seq_printf function */
static int
ex2_read(struct seq_file *m, void *p)
{
  if (pid)
    {
      struct task_struct *task;

      task = pid_task (find_vpid (pid), PIDTYPE_PID);	// Get task struct from pid
      if (!task)
	seq_printf (m, "pid %d does not exist\n", pid);
      else
	{
	  seq_printf (m, "Information about process %u\n", pid);
	  seq_printf (m, " ppid %u\n", task->parent->pid);
	  seq_printf (m, " Voluntary context switches %lu\n", task->nvcsw);
	  seq_printf (m, " Involuntary context switches %lu\n", task->nivcsw);
	  seq_printf (m, " Page table address %px\n",
		      (task->mm ? task->mm->pgd : NULL));
	}
    }
  else
    seq_printf (m, "No pid defined\n");
  return 0;
}

/* Write: captures the pid of the process */
static ssize_t
ex2_write(struct file *f, const char __user * buff, size_t len, loff_t * o)
{
#define MAX_LEN 7
  char c[MAX_LEN + 1];

  if (len > MAX_LEN)
    return -EINVAL;

  if (copy_from_user (c, buff, len))
    return -EFAULT;

  c[len] = 0;
  if (!sscanf (c, "%d\n", &pid))
    printk (KERN_INFO "No valid pid read\n");
  else
    printk (KERN_INFO "Read pid %d\n", pid);
  return len;
}

static int
ex2_open (struct inode *inode, struct file *file)
{
  return single_open (file, ex2_read, NULL);
}

static struct proc_ops proc_operations = {
  .proc_open = ex2_open,
  .proc_read = seq_read,
  .proc_write = ex2_write,
};

static int __init
procdemo_init (void)
{
  /* Register /proc/procdemo */
  if (!proc_create (PROC_ENTRY, S_IFREG, NULL, &proc_operations))
    return -ENOMEM;

  printk (KERN_INFO "procdemo: Correctly installed\n");
  return 0;
}

static void __exit
procdemo_cleanup (void)
{
  remove_proc_entry (PROC_ENTRY, NULL);	/* Unregister /proc/procdemo */
  printk (KERN_INFO "procdemo: Cleanup successful\n");
}

module_init (procdemo_init);
module_exit (procdemo_cleanup);
