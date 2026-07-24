#include <linux/module.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("kprobe example (captures open syscall)");

#define TRACE_FILE 	"/etc/passwd"

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    /* do_sys_openat2(int dfd, const char __user *filename, struct open_how *how)
     * Arg 0: dfd
     * Arg 1: filename
     */
    const char __user *filename = (const char __user *)regs_get_kernel_argument(regs, 1);

    if (filename) {
        char buf[256];
        if (strncpy_from_user(buf, filename, sizeof(buf)) > 0) {
            if (strncmp(buf, TRACE_FILE, sizeof(TRACE_FILE)) == 0) {
                pr_info("kprobe: Open %s\n", TRACE_FILE);
            }
        }
    }
    return 0;
}

static struct kprobe kp = {
    .symbol_name = "do_sys_openat2",
    .pre_handler = handler_pre,
};

static int __init kprobe_init(void)
{
    int ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("kprobe: Error at register time: %d\n", ret);
        return ret;
    }

    pr_info("kprobe: module loaded (monitoring opens on %s)\n", TRACE_FILE);
    return 0;
}

static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp);
    pr_info("kprobe: module unloaded\n");
}

module_init(kprobe_init);
module_exit(kprobe_exit);
