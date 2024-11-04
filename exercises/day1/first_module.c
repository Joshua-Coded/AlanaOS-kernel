// required headers for development

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

// module license 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joshua Alana");
MODULE_DESCRIPTION("Learning kernel module");

// function called when module is loaded
static int __init first_module_init(void)
{
	printk(KERN_INFO "Module: Loading\n");
	return 0;
}

// function called when module is unloaded
static void __exit first_module_exit(void)
{
	printk(KERN_INFO "Module: unloading\n");
}

// tell kernel which function to call
module_init(first_module_init);
module_exit(first_module_exit);



