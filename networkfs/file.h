#ifndef OS_2022_NETWORKFS_RAMZEESET_FILE_H
#define OS_2022_NETWORKFS_RAMZEESET_FILE_H

#include <linux/fs.h>
#include <linux/kernel.h>

int networkfs_iterate(struct file *filp, struct dir_context *ctx);

extern const struct file_operations networkfs_dir_ops;

#endif  // OS_2022_NETWORKFS_RAMZEESET_FILE_H
