/*
 * /sysfs demo
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/rcupdate.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Communication through /sys");

static pid_t target_pid = 0;
static struct kobject *sysdemo_kobj;

/* Module Parameter Callback (/sys/module/sysdemo/parameters/target_pid) */
static int param_set_target_pid(const char *val, const struct kernel_param *kp)
{
    pid_t new_pid;
    int err;

    err = kstrtouint(strstrip((char *)val), 10, &new_pid);
    if (err) {
        pr_warn("Invalid PID set via module parameter\n");
        return -EINVAL;
    }
    
    WRITE_ONCE(target_pid, new_pid);
    pr_info("PID updated via sysfs module parameter to %d\n", new_pid);
    return 0;
}

static const struct kernel_param_ops target_pid_ops = {
    .set = param_set_target_pid,
    .get = param_get_uint,
};

module_param_cb(target_pid, &target_pid_ops, &target_pid, 0644);
MODULE_PARM_DESC(target_pid, "Target PID to inspect (>= 0)");

static unsigned int count_open_fds(struct task_struct *task)
{
    struct files_struct *files;
    struct fdtable *fdt;
    unsigned int count = 0;
    int i;

    if (!task)
        return 0;

    files = task->files;
    if (!files)
        return 0; /* Just in case */

    /* Get fdtable */
    fdt = rcu_dereference_raw(files->fdt);
    if (!fdt || !fdt->fd)
        return 0;

    /* Traverse fdtable */
    for (i = 0; i < fdt->max_fds; i++) {
        struct file *file = rcu_dereference_raw(fdt->fd[i]);

        if (file) {
            count++;
        }
    }

    return count;
}

static ssize_t nch_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct task_struct *task;
    unsigned int count;

    if (target_pid < 0)
    return sysfs_emit(buf, "-1\n");

    if (target_pid == 0) {
        task = current;
        get_task_struct(task); /* Adquire task_struct */
    } else {
        /* Look for processes */
        rcu_read_lock();
        task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
        if (!task) {
            rcu_read_unlock();
    		return sysfs_emit(buf, "-1\n"); /* Does not exit */
        }
        get_task_struct(task); /* Adquire task_struct */
        rcu_read_unlock();
    }

    count = count_open_fds(task);

    /* Release task struct */
    put_task_struct(task);

    return sysfs_emit(buf, "%d\n", count);
}

static ssize_t errno_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct task_struct *task;
//    unsigned int count;

    if (target_pid < 0)
    return sysfs_emit(buf, "%d\n", EINVAL);

    if (target_pid == 0) {
        task = current;
        get_task_struct(task); /* Adquire task_struct */
    } else {
        /* Look for processes */
        rcu_read_lock();
        task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
        if (!task) {
            rcu_read_unlock();
    	return sysfs_emit(buf, "%d\n", ESRCH);
        }
        get_task_struct(task); /* Adquire task_struct */
        rcu_read_unlock();
    }

//    count = count_open_fds(task);

    /* Release task struct */
    put_task_struct(task);

   return sysfs_emit(buf, "%lu\n", 0L);
}

static struct kobj_attribute nch_attribute   = __ATTR_RO(nch);
static struct kobj_attribute errno_attribute  = __ATTR_RO(errno);

static struct attribute *attrs[] = {
    &nch_attribute.attr,
    &errno_attribute.attr,
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
