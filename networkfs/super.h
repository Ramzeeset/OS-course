#ifndef OS_2022_NETWORKFS_RAMZEESET_SUPER_H
#define OS_2022_NETWORKFS_RAMZEESET_SUPER_H

#include "file.h"
#include "inode.h"

int networkfs_fill_super(struct super_block *sb, void *data, int silent);

struct dentry *networkfs_mount(struct file_system_type *fs_type, int flags,
                               const char *token, void *data);

void networkfs_kill_sb(struct super_block *sb);

extern struct file_system_type networkfs_fs_type;

#endif  // OS_2022_NETWORKFS_RAMZEESET_SUPER_H
