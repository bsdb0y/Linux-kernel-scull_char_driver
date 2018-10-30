#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/kdev_t.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Neeraj Pal");
MODULE_DESCRIPTION("Scull char device");

struct scull_qset{
  void **data;
  struct scull_qset *next;
};
// Device structure
struct scull_dev {
  struct scull_qset *data; // Pointer to the first quantum set
  int quantum;             // The current quantum size
  int qset;                // The current array size
  unsigned long size;      // Amount of data store here
  unsigned int access_key; //used by sculluid & scullpriv
  struct semaphore sem;    // Mutual exclusion semaphore
  struct cdev c_dev;        // Char device structure
};
static int c;
static struct class *cl;
struct scull_dev *scull_device; //contains the scull devices
static dev_t first;
/*
static struct file_operations scull_fops = 
{
	.owner = THIS_MODULE,
	.open = scull_open,
	.read = scull_read,
	.write = scull_write,
	.release = scull_close
};
*/
static int scull_open(struct inode *i,struct file *f) {
	c++;
	printk(KERN_INFO "Scull Driver open %d times(s)\n",c);
	return 0;
	}

static int scull_close(struct inode *i,struct file *f) {
	
	printk(KERN_INFO "Scull Driver closed\n");
	return 0;
	}

static ssize_t scull_read(struct file *f,char __user *buf, size_t len,loff_t *off) {
	printk(KERN_INFO "Scull Driver : read()\n");
	return 0;
	}

static ssize_t scull_write(struct file *f , const char __user *buf,size_t len,loff_t *off) {
	printk(KERN_INFO "Driver: write()\n");
	printk(KERN_INFO "%d bytes send to kernel\n",(int)len);
	return len;
	}


static struct file_operations scull_fops =
{
        .owner = THIS_MODULE,
        .open = scull_open,
        .read = scull_read,
        .write = scull_write,
        .release = scull_close
};


/*
static void scull_setup_cdev(struct scull_dev *dev,dev_t devno) {
	int err;		
	cdev_init(&dev->c_dev,&scull_fops);
	dev->c_dev.owner = THIS_MODULE;
	dev->c_dev.ops = &scull_fops;
	err  = cdev_add(&dev->c_dev,devno,1);
	if (err)
	
		printk(KERN_NOTICE "Error %d adding scull",err);
}
*/

static int __init scull_init(void) {

	printk(KERN_INFO "Welcome Boy , Module Inserted\n");
	if (alloc_chrdev_region(&first,0,1,"") < 0)
	{
		return -1;
	}
	
	if ((cl = class_create(THIS_MODULE, "scull_driver")) == NULL)
	{
		unregister_chrdev_region(first,1);
		return -1;
	}

	if (device_create(cl,NULL,first,NULL,"myscull") == NULL)
	{
		class_destroy(cl);
		unregister_chrdev_region(first,1);
		return -1;
	}

	scull_device = kmalloc(sizeof(struct scull_dev),GFP_KERNEL) ;
	memset(scull_device,0,sizeof(struct scull_dev));

	cdev_init(&scull_device->c_dev, &scull_fops);
    	scull_device->c_dev.owner = THIS_MODULE;
    	scull_device->c_dev.ops = &scull_fops;
    	if (cdev_add(&scull_device->c_dev, first, 1) == -1)
  	{
    		device_destroy(cl, first);
    		class_destroy(cl);
    		unregister_chrdev_region(first, 1);
    		return -1;
  	}
	return 0;
}

static void __exit scull_exit(void) /* Destructor */
{
	cdev_del(&scull_device->c_dev);
  	device_destroy(cl, first);
  	class_destroy(cl);
  	unregister_chrdev_region(first, 1);
  	printk(KERN_INFO "Cruel World , Will meet soon , Module removed\n");
}
 
module_init(scull_init);
module_exit(scull_exit);	
