#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/rcupdate.h>

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

SYSCALL_DEFINE1(nch, pid_t, pid)
{
    struct task_struct *task;
    unsigned int count;

    if (pid < 0)
        return -EINVAL; 

    if (pid == 0) {
        task = current;
        get_task_struct(task); /* Adquire task_struct */
    } else {
        /* Look for processes */
        rcu_read_lock();
        task = pid_task(find_vpid(pid), PIDTYPE_PID);
        if (!task) {
            rcu_read_unlock();
            return -ESRCH; /* Does not exit */
        }
        get_task_struct(task); /* Adquire task_struct */
        rcu_read_unlock();
    }

    count = count_open_fds(task);

    /* Release task struct */
    put_task_struct(task);

    return count;
}
