#ifndef OS_2022_NETWORKFS_RAMZEESET_INODE_H
#define OS_2022_NETWORKFS_RAMZEESET_INODE_H

#include <linux/fs.h>
#include <linux/kernel.h>

struct inode *networkfs_get_inode(struct super_block *sb,
                                  const struct inode *dir, umode_t mode,
                                  int i_ino);

struct dentry *networkfs_lookup(struct inode *parent_inode,
                                struct dentry *child_dentry,
                                __attribute__((unused)) unsigned int flag);

int networkfs_create(struct user_namespace *mnt_userns,
                     struct inode *parent_inode, struct dentry *child_dentry,
                     umode_t mode, bool b);

int networkfs_unlink(struct inode *parent_inode, struct dentry *child_dentry);

int networkfs_mkdir(struct user_namespace *mnt_userns, struct inode *dir,
                    struct dentry *dentry, umode_t mode);

int networkfs_rmdir(struct inode *parent_inode, struct dentry *child_dentry);

extern const struct inode_operations networkfs_inode_ops;

#endif  // OS_2022_NETWORKFS_RAMZEESET_INODE_H
