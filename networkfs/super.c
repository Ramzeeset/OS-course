#include "super.h"

#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/string.h>

const int TOKEN_SIZE = 100;

char TOKEN[100];

int networkfs_fill_super(struct super_block *sb, void *data, int silent) {
  printk(KERN_INFO "fill super\n");
  struct inode *inode;
  inode = networkfs_get_inode(sb, NULL, S_IFDIR | S_IRWXUGO, 1000);
  if (inode) {
    inode->i_fop = &networkfs_dir_ops;
  }
  sb->s_root = d_make_root(inode);
  if (sb->s_root == NULL) {
    return -ENOMEM;
  }
  void *token = kmalloc(100, GFP_KERNEL);
  if (token == 0) {
    return -10;
  }
  strcpy(token, TOKEN);
  sb->s_fs_info = token;
  printk(KERN_INFO "token in sb %s\n", TOKEN);
  printk(KERN_INFO "return 0\n");
  return 0;
}

struct dentry *networkfs_mount(struct file_system_type *fs_type, int flags,
                               const char *token, void *data) {
  printk(KERN_INFO "in mount\n");
  struct dentry *ret;
  strcpy(TOKEN, token);
  ret = mount_nodev(fs_type, flags, data, networkfs_fill_super);
  if (ret == NULL) {
    printk(KERN_WARNING "Can't mount file system");
  } else {
    printk(KERN_INFO "Mounted successfuly");
  }
  printk(KERN_INFO "token in mount %s\n", TOKEN);
  return ret;
}

void networkfs_kill_sb(struct super_block *sb) {
  kfree(sb->s_fs_info);
  printk(KERN_INFO
         "networkfs super block is destroyed. Unmount successfully.\n");
}

struct file_system_type networkfs_fs_type = {.name = "networkfs",
                                             .mount = networkfs_mount,
                                             .kill_sb = networkfs_kill_sb};
