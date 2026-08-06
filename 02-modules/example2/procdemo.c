/*
 * /proc example module 
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Communication through /proc");

#define PROC_ENTRY "procdemo"

static pid_t target_pid = 0;

/* Read callback: displays target_pid data */
static int proc_read_show(struct seq_file *m, void *v)
{
    struct task_struct *task;
    pid_t current_pid = READ_ONCE(target_pid);

    if (!current_pid) {
        seq_puts(m, "No pid defined\n");
        return 0;
    }

    rcu_read_lock();

    task = pid_task(find_vpid(current_pid), PIDTYPE_PID);
    if (!task) {
        rcu_read_unlock();
        seq_printf(m, "pid %d does not exist\n", current_pid);
        return 0;
    }

    seq_printf(m, "Information about process %d\n", current_pid);
    seq_printf(m, "  ppid %d\n", task_ppid_nr(task));
    seq_printf(m, "  Voluntary context switches %lu\n", task->nvcsw);
    seq_printf(m, "  Involuntary context switches %lu\n", task->nivcsw);

    struct mm_struct *mm = get_task_mm(task);
    seq_printf(m, "  Page table address (pgd) %px\n", mm ? mm->pgd : NULL);
    if (mm)
        mmput(mm);

    rcu_read_unlock();
    return 0;
}

/* Write callback: updates target_pid from user input */
static ssize_t proc_write_cb(struct file *f, const char __user *buff, size_t len, loff_t *o)
{
#define MAX_LEN 16
    char kbuf[MAX_LEN];
    pid_t parsed_pid;
    int err;

    if (len >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buff, len))
        return -EFAULT;

    kbuf[len] = '\0';

    err = kstrtouint(kbuf, 10, &parsed_pid);
    if (err) {
        pr_warn("Invalid PID written to /proc/%s\n", PROC_ENTRY);
        return -EINVAL;
    }

    WRITE_ONCE(target_pid, parsed_pid);
    pr_info("Updated PID to %d via /proc/%s\n", parsed_pid, PROC_ENTRY);
    return len;
}

static int proc_open_cb(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_show, NULL);
}

static const struct proc_ops proc_operations = {
    .proc_open    = proc_open_cb,
    .proc_read    = seq_read,
    .proc_write   = proc_write_cb,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init procdemo_init(void)
{
    /* Register /proc entry */
    if (!proc_create(PROC_ENTRY, 0666, NULL, &proc_operations)) {
        pr_err("Failed to create /proc/%s entry\n", PROC_ENTRY);
        return -ENOMEM;
    }

    pr_info("Module loaded successfully: /proc/%s active\n", PROC_ENTRY);
    return 0;
}

static void __exit procdemo_cleanup(void)
{
    /* Unregister /proc entry */
    remove_proc_entry(PROC_ENTRY, NULL);

    pr_info("Cleanup complete\n");
}

module_init(procdemo_init);
module_exit(procdemo_cleanup);
