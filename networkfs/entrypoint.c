#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sync_file.h>

#include "super.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vadim Sadokhov");
MODULE_VERSION("0.01");

int networkfs_init(void) {
  int flag = register_filesystem(&networkfs_fs_type);
  if (flag == 0) {
    printk(KERN_INFO "Successfully register networkfs!\n");
  } else {
    printk(KERN_INFO "Can't register networkfs\n");
  }
  return 0;
}

void networkfs_exit(void) {
  int flag = unregister_filesystem(&networkfs_fs_type);
  if (flag == 0) {
    printk(KERN_INFO "Successfully unregister networkfs!\n");
  } else {
    printk(KERN_INFO "Can't unregister networkfs\n");
  }
}

module_init(networkfs_init);
module_exit(networkfs_exit);
