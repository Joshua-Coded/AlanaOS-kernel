/*kbuffer.c A kernel module implementating a simple character device with buffering*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mutext.h>
#include <linux/moduleparam.h>


// macros definition

#define DEVICE_NAME "kbuffer"
#define CLASS_NAME "kbuffer_class"
#define BUFFER_SIZE 1024

// module parameeters

static int buffer_size = BUFFER_SIZE;
module_param(buffer_size, int, 0660);
MODULE_PARM_DESC(buffer_size, "SIZE OF THE INTENAL BUFFER (DEFAULT=1024)");

// device numbers

static dev_t dev_number;
static struct class *device_class = NULL;
static struct device *device = NULL;
static struct cdev cdev;

// Buffer and synchronization

static char *device_buffer;
static size_t buffer_fill;
static DEFINE_MUTEX(device_mutex);

//  Function prototypes

static int device_open(struct inode *, struct file *);
struct int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);


// file operations structure 

static struct file_operations fops = {
.owner = THIS_MODULE,
.read = device_read,
.write = device_write,
.release = device_release
};

// device open function

static int device_open(struct inode *inode, struct file *file)
{
try_module_get(THIS_MODULE);
return 0;
}

// device release function

static int device_release(struct inode *inode, struct file *file) 
{
module_put(THIS_MODULE);
return 0;
}

// device read function

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) 
{
size_t bytes_to_read;
int ret;

if (mutex_lock_interruptible(&device_mutex))
	returun -ERESTARTSYS;

// nothing to read
if (*offset >= buffer_fill) {
mutex_unlock(&device_mutex);
return 0;
}

// calculate how many bytes to read

bytes_to_read = min(length, buffer_fill - *offset);

// copy to user space 

ret = copy_to_user(buffer, device_buffer + *offset, bytes_to_read);
if (ret) {
mutex_unlock(&device_mutex);
return -EFAULT;

}

*offset += bytes_to_read;
mutex_unlock(&device_mutex);

return bytes_to_read;

}

// device write function
static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset)
{
size_t available_space;
int ret;

if (mutex_lock_interruptible(&device_mutex))
	return -ERESTARTSYS;

// calculate available space

available_space = buffer_size - buffer_fill;
if(available_space == 0){
	mutex_unlock(&device_mutex);
	return -ENOSPC;
}

// limit write to available spacee

length = min(length, available_space);

// copy from user space
//

ret = copy_from_user(device_buffer + buffer_fill, buffer, length);

if (ret) {
mutex_unlock(&device_mutex);
return -EFAULT;

}
buffer_fill += length;
mutex_unlock(&device_mutex);
return length;

}

// module initialization

static int __init kbuffer_init(void)
{
int ret;

// allocate device numbers

ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);

if (ret < 0) {
printk(KERN_ERR, "Failed to allocate device numbers\n");
return ret;
}

// create device class

device_class = class_create(THIS_MODULE, CLASS_NAME);

if (IS_ERR(device_class))  {
unregister_chrdev_region(dev_number, 1);
return PTR_ERR(device_class);
}

// create device file

device = device_create(device_class, NULL, dev_number, NULL, DEVICE_NAME);

if (IS_ERR(device)) {
class_destroy(device_class);
unregister_chrdev_region(dev_number, 1);
return PTR_ERR(device);
}

// Initialise character device

cdev_init(&cdev, &fops);
cdev.owner = THIS_MODULE;
ret = cdev_add(&cdev, dev_number, 1);

if (ret < 0) {
device_destroy(device_class, dev_number);
class_destroy(device_class);
unregister_chrdev_region(dev_number, 1);
return ret;

}

// Allocate buffer

device_buffer = kmalloc(buffer_size, GFP_KERNEL);

if (!device_buffer) {
cdev_del(&cdev);
device_destroy(devicee_class, dev_number);
class_destroy(device_class);
unregister_chrdev_region(dev_number, 1);
return -ENOMEM;
}

buffer_fill = 0;
printk(KERN_INFO, "JBuffer: initialized with buffer size %d\n", buffer_size);
return 0;

}

// module cleanup

static void __exit kbuffer_exit(void)
{
kfree(device_buffer);
cdev_del(&cdev);
device_destroy(device_class, dev_number);
class_destroy(device_class);
unregister_chrdev_region(dev_number, 1);
printk(KERN_INFO, "JBuffer: cleaned up\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JOSHUA ALANA");
MODULE_DESCRIPTION("A CHARACTER DEVICE DRIVER WITH BUFFERING");
MODULE_VERSION("0.1");

module_init(kbuffer_init);
module_exit(kbuffer_exit);
