#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("kprobe (captures write syscall)");

static int tracked_fd = 10; // Default channel
static int bytes_written = 0;

/* Module Parameter Callback (/sys/module/.../parameters/tracked_fd) */
static int param_set_tracked_fd(const char *val, const struct kernel_param *kp)
{
    int new_fd;
    int err;

    err = kstrtoint(strstrip((char *)val), 10, &new_fd);
    if (err || new_fd < 0 || new_fd >= task_rlimit(current, RLIMIT_NOFILE)) {
        pr_warn("Invalid channel set via module parameter (up to %lu)\n", task_rlimit(current, RLIMIT_NOFILE)-1 );
        return -EINVAL;
    }

    if (new_fd != tracked_fd) {
        pr_info("tracked_fd modified dynamically: %d -> %d\n", tracked_fd, new_fd);
        pr_info("%d bytes written at channel %d\n", bytes_written, tracked_fd);
	bytes_written = 0;
        WRITE_ONCE(tracked_fd, new_fd);
    }
    
    return 0;
}

//  Callbacks for our parameter
static const struct kernel_param_ops tracked_fd_ops = {
    .set = param_set_tracked_fd, // Our custom set function
    .get = param_get_int,        // Default get function (cat /sys/module/...)
};

module_param_cb(tracked_fd, &tracked_fd_ops, &tracked_fd, 0644);
MODULE_PARM_DESC(tracked_fd, "File descriptor channel to monitor");

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    /* ksys_write(int fd, const char __user *addr, int count)
     * Arg 0: fd
     * Arg 2: count 
     */
    int fd = regs_get_kernel_argument(regs, 0);

    if (fd == tracked_fd) { 
    	bytes_written += regs_get_kernel_argument(regs, 2);
//	pr_info("write at channel %d\n", tracked_fd);
    }
    return 0;
}

static struct kprobe kp = {
    .symbol_name = "ksys_write",
    .pre_handler = handler_pre,
};

static int __init kprobe_init(void)
{
    int ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("Error at register time: %d\n", ret);
        return ret;
    }

    pr_info("module loaded (monitoring writes at channel %d)\n", tracked_fd);
    return 0;
}

static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp);
    pr_info("module unloaded: %d bytes written at channel %d\n", bytes_written, tracked_fd);
}

module_init(kprobe_init);
module_exit(kprobe_exit);
