/*
 * Dual Interface Kernel Module (/proc and sysfs)
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Unified process telemetry module via /proc and sysfs");

#define PROC_ENTRY "procdemo"

static int target_pid = 0;
static struct kobject *procdemo_kobj;

/* Shared helper for atomic PID updates and validation */
static int update_target_pid(int new_pid)
{
    if (new_pid < 0)
        return -EINVAL;

    WRITE_ONCE(target_pid, new_pid);
    return 0;
}

/* Module Parameter Callback (/sys/module/procdemo/parameters/target_pid) */
static int set_target_pid_param(const char *val, const struct kernel_param *kp)
{
    int new_pid;
    int err;

    err = kstrtoint(strstrip((char *)val), 10, &new_pid);
    if (err || update_target_pid(new_pid) < 0) {
        pr_warn("Invalid PID set via module parameter\n");
        return -EINVAL;
    }

    pr_info("PID updated via sysfs module parameter to %d\n", new_pid);
    return 0;
}

static const struct kernel_param_ops target_pid_ops = {
    .set = set_target_pid_param,
    .get = param_get_int,
};

module_param_cb(target_pid, &target_pid_ops, &target_pid, 0644);
MODULE_PARM_DESC(target_pid, "Target PID to inspect (>= 0)");

/* =========================================================================
 * 1. /proc INTERFACE IMPLEMENTATION
 * ========================================================================= */

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
    seq_printf(m, "  Page table address %px\n", mm ? mm->pgd : NULL);
    if (mm)
        mmput(mm);

    rcu_read_unlock();
    return 0;
}

static ssize_t proc_write_cb(struct file *f, const char __user *buff, size_t len, loff_t *o)
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
        pr_warn("Invalid PID written to /proc/%s\n", PROC_ENTRY);
        return -EINVAL;
    }

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

/* =========================================================================
 * 2. SYSFS INTERFACE IMPLEMENTATION (/sys/kernel/procdemo/)
 * ========================================================================= */

static struct task_struct *get_current_task(pid_t *pid_out)
{
    *pid_out = READ_ONCE(target_pid);
    if (!*pid_out)
        return NULL;

    return pid_task(find_vpid(*pid_out), PIDTYPE_PID);
}

static ssize_t ppid_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct task_struct *task;
    pid_t pid, ppid = 0;

    rcu_read_lock();
    task = get_current_task(&pid);
    if (task)
        ppid = task_ppid_nr(task);
    rcu_read_unlock();

    if (!pid || !task)
        return sysfs_emit(buf, "0\n");

    return sysfs_emit(buf, "%d\n", ppid);
}

static ssize_t nvcsw_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct task_struct *task;
    pid_t pid;
    unsigned long nvcsw = 0;

    rcu_read_lock();
    task = get_current_task(&pid);
    if (task)
        nvcsw = task->nvcsw;
    rcu_read_unlock();

    if (!pid || !task)
        return sysfs_emit(buf, "0\n");

    return sysfs_emit(buf, "%lu\n", nvcsw);
}

static ssize_t nivcsw_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct task_struct *task;
    pid_t pid;
    unsigned long nivcsw = 0;

    rcu_read_lock();
    task = get_current_task(&pid);
    if (task)
        nivcsw = task->nivcsw;
    rcu_read_unlock();

    if (!pid || !task)
        return sysfs_emit(buf, "0\n");

    return sysfs_emit(buf, "%lu\n", nivcsw);
}

static ssize_t pgd_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct task_struct *task;
    struct mm_struct *mm = NULL;
    pgd_t *pgd_addr = NULL;
    pid_t pid;

    rcu_read_lock();
    task = get_current_task(&pid);
    if (task)
        mm = get_task_mm(task);
    rcu_read_unlock();

    if (mm) {
        pgd_addr = mm->pgd;
        mmput(mm);
    }

    if (!pid || !task || !pgd_addr)
        return sysfs_emit(buf, "0x0\n");

    return sysfs_emit(buf, "%px\n", pgd_addr);
}

static struct kobj_attribute ppid_attribute   = __ATTR_RO(ppid);
static struct kobj_attribute nvcsw_attribute  = __ATTR_RO(nvcsw);
static struct kobj_attribute nivcsw_attribute = __ATTR_RO(nivcsw);
static struct kobj_attribute pgd_attribute    = __ATTR_RO(pgd);

static struct attribute *attrs[] = {
    &ppid_attribute.attr,
    &nvcsw_attribute.attr,
    &nivcsw_attribute.attr,
    &pgd_attribute.attr,
    NULL,
};

static const struct attribute_group attr_group = {
    .attrs = attrs,
};

/* =========================================================================
 * MODULE INIT & CLEANUP
 * ========================================================================= */

static int __init procdemo_init(void)
{
    int err;

    /* 1. Register /proc entry */
    if (!proc_create(PROC_ENTRY, 0666, NULL, &proc_operations)) {
        pr_err("Failed to create /proc/%s entry\n", PROC_ENTRY);
        return -ENOMEM;
    }

    /* 2. Register sysfs entry under /sys/kernel/procdemo/ */
    procdemo_kobj = kobject_create_and_add("procdemo", kernel_kobj);
    if (!procdemo_kobj) {
        pr_err("Failed to create sysfs kobject\n");
        remove_proc_entry(PROC_ENTRY, NULL);
        return -ENOMEM;
    }

    err = sysfs_create_group(procdemo_kobj, &attr_group);
    if (err) {
        pr_err("Failed to create sysfs attribute group\n");
        kobject_put(procdemo_kobj);
        remove_proc_entry(PROC_ENTRY, NULL);
        return err;
    }

    pr_info("Module loaded successfully: /proc/%s and /sys/kernel/procdemo/ active\n", PROC_ENTRY);
    return 0;
}

static void __exit procdemo_cleanup(void)
{
    /* Unregister sysfs tree */
    kobject_put(procdemo_kobj);

    /* Unregister /proc entry */
    remove_proc_entry(PROC_ENTRY, NULL);

    pr_info("Cleanup complete\n");
}

module_init(procdemo_init);
module_exit(procdemo_cleanup);
