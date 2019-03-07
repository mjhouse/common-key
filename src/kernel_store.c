#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/ioctl.h>

#include "ks_common.h"

#define DEVICE_NAME "kernel_store"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael House");
MODULE_DESCRIPTION("A common data store for user-space applications.");
MODULE_VERSION("0.01");

/* Prototypes for device functions */
static int device_open(struct inode *i, struct file *f);
static int device_release(struct inode *i, struct file *f);
static ssize_t device_read(struct file *filp, char *buffer, size_t len, loff_t *offset);
static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *offset);
static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long param);

static int major_num;
static node stash = { .key = 0, .val = 0 };

/* Device function mapping for this module */
struct file_operations fops = {
    .owner   = THIS_MODULE,
    .read    = device_read,
    .write   = device_write,
    .open    = device_open,
    .release = device_release,
    .unlocked_ioctl = device_ioctl
};

static int device_open(struct inode *i, struct file *f){
    return 0;
};

static int device_release(struct inode *i, struct file *f){
    return 0;
};

static ssize_t device_read(struct file *filp, char *buffer, size_t len, loff_t *offset){
    return 0;
};

static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *offset){
    return 0;
};

static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long param) {
    node* n = (node*)param;

    size_t sk = 0;
    size_t sv = 0;
    unsigned long err = 0;

    if(!n) return -EINVAL;

    switch(cmd){
        case KS_GET_VALUE:
            printk("KS_GET_VALUE");

            sk = strlen(stash.key) + 1;
            sv = strlen(stash.val) + 1;

            printk("sk: %lu, sv: %lu",sk,sv);

            if( access_ok(VERIFY_WRITE,n->key,sk) &&
                access_ok(VERIFY_WRITE,n->val,sv)){

                err = copy_to_user( n->key, stash.key, sk);
                printk("key chars not copied: %lu",err);

                err = copy_to_user( n->val, stash.val, sv);
                printk("val chars not copied: %lu",err);

            }

            break;
        case KS_SET_VALUE:
            printk("KS_SET_VALUE");

            sk = strlen(n->key) + 1;
            sv = strlen(n->val) + 1;

            if(stash.key) kfree(stash.key);
            if(stash.val) kfree(stash.val);

            stash.key = (char*)kmalloc(sk,GFP_KERNEL);
            stash.val = (char*)kmalloc(sv,GFP_KERNEL);

            if( copy_from_user( stash.key, n->key, sk) ||
                copy_from_user( stash.val, n->val, sv) ){
                return -EINVAL;
            }

            printk("stash: %s/%s",stash.key,stash.val);
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

static int __init kernel_store_init(void) {
    major_num = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_num < 0) {
        printk(KERN_ALERT "could not register device: %d\n", major_num);
        return major_num;
    } else {
        printk(KERN_INFO "kernel_store module loaded with device major number %d\n", major_num);
    }

    // stash.key = (char*)kcalloc(32,sizeof(char),GFP_KERNEL);
    // stash.val = (char*)kcalloc(32,sizeof(char),GFP_KERNEL);
    //
    // if(!stash.key || !stash.val){
    //     return -ENOMEM;
    // }
    return 0;
}

static void __exit kernel_store_exit(void) {
    unregister_chrdev(major_num, DEVICE_NAME);
    printk("kernel_store unloaded\n");
}

module_init(kernel_store_init);
module_exit(kernel_store_exit);
