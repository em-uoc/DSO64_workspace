/*
 * /proc & sysfs parameters demo module 
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("/proc and module parameter demo");

#define PROC_ENTRY "procdemo"

static int target_pid = 0;

/* Helper for PID validation and atomic update */
static int update_target_pid(int new_pid)
{
    if (new_pid < 0)
        return -EINVAL;

    WRITE_ONCE(target_pid, new_pid);
    return 0;
}

/* 
 * Sysfs callback (/sys/module/procdemo/parameters/target_pid)
 * Handles runtime modifications via sysfs while enforcing PID validation.
 */
static int set_target_pid_param(const char *val, const struct kernel_param *kp)
{
    int new_pid;
    int err;

    err = kstrtoint(strstrip((char *)val), 10, &new_pid);
    if (err || update_target_pid(new_pid) < 0) {
        pr_warn("Attempted to set invalid PID via sysfs\n");
        return -EINVAL;
    }

    pr_info("PID updated via sysfs to %d\n", new_pid);
    return 0;
}

static const struct kernel_param_ops target_pid_ops = {
    .set = set_target_pid_param,
    .get = param_get_int,
};

module_param_cb(target_pid, &target_pid_ops, &target_pid, 0644);
MODULE_PARM_DESC(target_pid, "PID of the process to inspect (>= 0)");

/* Read callback: displays information for target_pid */
static int ex2_read(struct seq_file *m, void *v)
{
    struct task_struct *task;
    pid_t current_pid = READ_ONCE(target_pid);

    if (!current_pid) {
        seq_puts(m, "No pid defined\n");
        return 0;
    }

    /* RCU read-side critical section for safe task_struct access */
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

    /* Safely acquire and access mm_struct reference */
    struct mm_struct *mm = get_task_mm(task);
    seq_printf(m, "  Page table address %px\n", mm ? mm->pgd : NULL);
    if (mm)
        mmput(mm);

    rcu_read_unlock();

    return 0;
}

/* Write callback: receives PID input from /proc/procdemo */
static ssize_t ex2_write(struct file *f, const char __user *buff, size_t len, loff_t *o)
{
#define MAX_LEN 16

    char kbuf[MAX_LEN];
    int parsed_pid;
    int err;

    if (len >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buff, len))
        return -EFAULT;

    kbuf[len] = '\0';

    err = kstrtoint(strstrip(kbuf), 10, &parsed_pid);
    if (err || update_target_pid(parsed_pid) < 0) {
        pr_warn("Invalid PID written to /proc\n");
        return -EINVAL;
    }

    pr_info("Read pid %d via /proc\n", parsed_pid);
    return len;
}

static int ex2_open(struct inode *inode, struct file *file)
{
    return single_open(file, ex2_read, NULL);
}

static const struct proc_ops proc_operations = {
    .proc_open    = ex2_open,
    .proc_read    = seq_read,
    .proc_write   = ex2_write,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init procdemo_init(void)
{
    if (!proc_create(PROC_ENTRY, 0666, NULL, &proc_operations))
        return -ENOMEM;

    pr_info("Module loaded successfully (initial target_pid=%d)\n", target_pid);
    return 0;
}

static void __exit procdemo_cleanup(void)
{
    remove_proc_entry(PROC_ENTRY, NULL);
    pr_info("Cleanup successful\n");
}

module_init(procdemo_init);
module_exit(procdemo_cleanup);
