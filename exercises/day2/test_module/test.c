#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("JOSHUA ALANA");
MODULE_DESCRIPTION("A SIMPLE TEST MODULE");
MODULE_VERSION("0.1");

static int __init test_init(void) {
	printk(KERN_INFO "Hello, kernel module loadded successfully!\n");
	return 0;
}

static void __exit test_exit(void) {
	printk(KERN_INFO "GOODBYE, KERNEL MODULE UNLOADED!\n");
}

module_init(test_init);
module_exit(test_exit);
