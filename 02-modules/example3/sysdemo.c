/*
 * /sysfs demo
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Communication through /sys");

static int target_pid = 0;
static struct kobject *sysdemo_kobj;

/* Module Parameter Callback (/sys/module/sysdemo/parameters/target_pid) */
static int param_set_target_pid(const char *val, const struct kernel_param *kp)
{
    int new_pid;
    int err;

    err = kstrtoint(strstrip((char *)val), 10, &new_pid);
    if (err || new_pid < 0) {
        pr_warn("Invalid PID set via module parameter\n");
        return -EINVAL;
    }
    
    WRITE_ONCE(target_pid, new_pid);
    pr_info("PID updated via sysfs module parameter to %d\n", new_pid);
    return 0;
}

static const struct kernel_param_ops target_pid_ops = {
    .set = param_set_target_pid,
    .get = param_get_int,
};

module_param_cb(target_pid, &target_pid_ops, &target_pid, 0644);
MODULE_PARM_DESC(target_pid, "Target PID to inspect (>= 0)");

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
        return sysfs_emit(buf, "-\n");

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
        return sysfs_emit(buf, "-\n");

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
        return sysfs_emit(buf, "-\n");

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
        return sysfs_emit(buf, "-\n");

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


static int __init sysdemo_init(void)
{
    int err;

    /* Register sysfs entry under /sys/kernel/sysdemo/ */
    sysdemo_kobj = kobject_create_and_add("sysdemo", kernel_kobj);
    if (!sysdemo_kobj) {
        pr_err("Failed to create sysfs kobject\n");
        return -ENOMEM;
    }

    err = sysfs_create_group(sysdemo_kobj, &attr_group);
    if (err) {
        pr_err("Failed to create sysfs attribute group\n");
        kobject_put(sysdemo_kobj);
        return err;
    }

    pr_info("Module loaded successfully: /sys/kernel/sysdemo active\n");
    return 0;
}

static void __exit sysdemo_cleanup(void)
{
    /* Unregister sysfs tree */
    kobject_put(sysdemo_kobj);

    pr_info("Cleanup complete\n");
}

module_init(sysdemo_init);
module_exit(sysdemo_cleanup);
