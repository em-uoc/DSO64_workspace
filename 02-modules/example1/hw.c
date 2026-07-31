/*
 * Hello World Module
 */


#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("Hello world!");

static int __init
hw_init (void)
{
  pr_info ("Hello world!\n");
  return 0;
}

static void __exit
hw_exit (void)
{
  pr_info ("Bye world!\n");
}

module_init (hw_init);
module_exit (hw_exit);
