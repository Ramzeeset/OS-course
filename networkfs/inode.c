#include "inode.h"

#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/stat.h>

#include "file.h"
#include "http.h"

struct inode *networkfs_get_inode(struct super_block *sb,
                                  const struct inode *dir, umode_t mode,
                                  int i_ino) {
  printk(KERN_INFO "get inode\n");
  struct inode *inode;
  inode = new_inode(sb);
  if (inode != NULL) {
    inode->i_ino = i_ino;
    inode->i_op = &networkfs_inode_ops;
    inode_init_owner(&init_user_ns, inode, dir, mode);
  }
  return inode;
}

struct entry_info {
  unsigned char entry_type;  // DT_DIR (4) or DT_REG (8)
  ino_t ino;
};

struct dentry *networkfs_lookup(struct inode *parent_inode,
                                struct dentry *child_dentry,
                                __attribute__((unused)) unsigned int flag) {
  printk(KERN_INFO "called networkfs_lookup\n");
  const char *name = child_dentry->d_name.name;
  ino_t root = parent_inode->i_ino;
  const char *token = parent_inode->i_sb->s_fs_info;
  struct entry_info buffer = {0};
  char i_ino[16];
  sprintf(i_ino, "%ld", root);
  printk(KERN_INFO "ino of root %s\n", i_ino);
  printk(KERN_INFO "name is \"%s\"\n", name);
  int64_t error = networkfs_http_call(token, "lookup", (char *)&buffer,
                                      sizeof(struct entry_info), 2, "parent",
                                      i_ino, "name", name);
  if (error) {
    printk(KERN_WARNING "http call for lookup return %lld\n", error);
    return NULL;
  }
  char type = buffer.entry_type;
  printk(KERN_INFO "type is %d\n", type);
  ino_t ino = buffer.ino;
  printk(KERN_INFO "ino is %ld\n", ino);
  struct inode *inode = networkfs_get_inode(
      parent_inode->i_sb, parent_inode,
      type == 4 ? S_IFDIR | S_IRWXUGO : S_IFREG | S_IRWXUGO, ino);
  if (type == 4) {
    inode->i_fop = &networkfs_dir_ops;
  } else {
    inode->i_fop = NULL;
  }
  d_add(child_dentry, inode);
  return NULL;
}

int networkfs_create(struct user_namespace *mnt_userns,
                     struct inode *parent_inode, struct dentry *child_dentry,
                     umode_t mode, bool b) {
  printk(KERN_INFO "called networkfs_create\n");
  ino_t root;
  struct inode *inode;
  const char *name = child_dentry->d_name.name;
  char type[10];
  if (mode & S_IFDIR) {
    memcpy(type, "directory", 10);
  } else if (mode & S_IFREG) {
    memcpy(type, "file", 5);
  } else {
    printk(KERN_WARNING "Unsupported file type %d\n", mode);
    return -1;
  }
  root = parent_inode->i_ino;
  const char *token = parent_inode->i_sb->s_fs_info;
  ino_t buffer;
  char i_ino[16];
  sprintf(i_ino, "%ld", root);
  printk(KERN_INFO "ino of root %s\n", i_ino);
  printk(KERN_INFO "name is \"%s\"\n", name);
  printk(KERN_INFO "type is \"%s\"\n", type);
  int64_t error =
      networkfs_http_call(token, "create", (char *)&buffer, sizeof(ino_t), 3,
                          "parent", i_ino, "name", name, "type", type);
  if (error) {
    printk(KERN_WARNING "http call for create return %lld\n", error);
    return -1;
  }
  ino_t ino = buffer;
  inode = networkfs_get_inode(parent_inode->i_sb, parent_inode,
                              mode | S_IRWXUGO, ino);
  if (mode & S_IFDIR) {
    inode->i_fop = &networkfs_dir_ops;
  } else {
    inode->i_fop = NULL;
  }
  d_add(child_dentry, inode);
  return 0;
}

int networkfs_unlink(struct inode *parent_inode, struct dentry *child_dentry) {
  printk(KERN_INFO "called networkfs_unlink\n");
  ino_t root;
  const char *name = child_dentry->d_name.name;
  root = parent_inode->i_ino;
  const char *token = parent_inode->i_sb->s_fs_info;
  char i_ino[16];
  char empty_buffer;
  sprintf(i_ino, "%ld", root);
  printk(KERN_INFO "ino of root %s\n", i_ino);
  printk(KERN_INFO "name is \"%s\"\n", name);
  int64_t error = networkfs_http_call(token, "unlink", &empty_buffer, 0, 2,
                                      "parent", i_ino, "name", name);
  if (error) {
    printk(KERN_WARNING "http call for unlink return %lld\n", error);
    return -1;
  }
  return 0;
}

int networkfs_mkdir(struct user_namespace *mnt_userns, struct inode *dir,
                    struct dentry *dentry, umode_t mode) {
  printk(KERN_INFO "called networkfs_mkdir\n");
  return networkfs_create(mnt_userns, dir, dentry, mode | S_IFDIR, false);
}

int networkfs_rmdir(struct inode *parent_inode, struct dentry *child_dentry) {
  printk(KERN_INFO "called networkfs_rmdir\n");
  ino_t root;
  const char *name = child_dentry->d_name.name;
  root = parent_inode->i_ino;
  const char *token = parent_inode->i_sb->s_fs_info;
  char i_ino[16];
  char empty_buffer;
  sprintf(i_ino, "%ld", root);
  printk(KERN_INFO "ino of root %s\n", i_ino);
  printk(KERN_INFO "name is \"%s\"\n", name);
  int64_t error = networkfs_http_call(token, "rmdir", &empty_buffer, 0, 2,
                                      "parent", i_ino, "name", name);
  if (error) {
    printk(KERN_WARNING "http call for rmdir return %lld\n", error);
    return -1;
  }
  return 0;
}

const struct inode_operations networkfs_inode_ops = {.lookup = networkfs_lookup,
                                                     .create = networkfs_create,
                                                     .unlink = networkfs_unlink,
                                                     .mkdir = networkfs_mkdir,
                                                     .rmdir = networkfs_rmdir};