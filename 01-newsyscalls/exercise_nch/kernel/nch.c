#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/rcupdate.h>

/**
 * count_open_fds_iterative - Rutina auxiliar iterativa para contar canales.
 * @task: Puntero al task_struct objetivo.
 */
static unsigned int count_open_fds_iterative(struct task_struct *task)
{
    struct files_struct *files;
    struct fdtable *fdt;
    unsigned int count = 0;
    int i;

    if (!task)
        return 0;

    files = task->files;
    if (!files)
        return 0; /* Por si es un kernel thread sin tabla de archivos */

    /* Obtenemos la fdtable */
    fdt = rcu_dereference_raw(files->fdt);
    if (!fdt || !fdt->fd)
        return 0;

    /* Recorremos el array de punteros 'fd' */
    for (i = 0; i < fdt->max_fds; i++) {
        struct file *file = rcu_dereference_raw(fdt->fd[i]);

        /* Si la posición apunta a un struct file válido, el canal está abierto */
        if (file) {
            count++;
        }
    }

    return count;
}

/**
 * sys_count_process_fds - Llamada al sistema parametrizada con el PID.
 * @pid: PID del proceso objetivo (si es 0, analiza el proceso actual).
 *
 * Return: Número de canales abiertos o un código de error negativo (-ESRCH, -EINVAL).
 */
SYSCALL_DEFINE1(nch, pid_t, pid)
{
    struct task_struct *task;
    unsigned int count;

    if (pid < 0)
        return -EINVAL; /* PID no válido */

    /* Si el PID es 0, nos referimos al proceso que invoca la syscall */
    if (pid == 0) {
        task = current;
        get_task_struct(task); /* Retenemos el task_struct */
    } else {
        /* Búsqueda segura del task_struct usando la tabla de PIDs */
        rcu_read_lock();
        task = pid_task(find_vpid(pid), PIDTYPE_PID);
        if (!task) {
            rcu_read_unlock();
            return -ESRCH; /* El proceso no existe */
        }
        get_task_struct(task); /* Incrementa refcount para evitar que el task se libere */
        rcu_read_unlock();
    }

    /* Invocación de la rutina auxiliar */
    count = count_open_fds_iterative(task);

    /* Liberamos la referencia tomada sobre la tarea */
    put_task_struct(task);

    return count;
}
