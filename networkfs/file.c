#include "file.h"

#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "http.h"

struct entries {
  size_t entries_count;
  struct entry {
    unsigned char entry_type;  // DT_DIR (4) or DT_REG (8)
    ino_t ino;
    char name[256];
  } entries[16];
};

int networkfs_iterate(struct file *filp, struct dir_context *ctx) {
  char fsname[256];
  struct inode *inode = filp->f_path.dentry->d_inode;
  unsigned long offset = filp->f_pos;
  int stored = 0;
  ino_t ino = inode->i_ino;
  printk(KERN_INFO "ino %ld in iterate with offset %ld\n", ino, offset);
  char i_ino[16];
  sprintf(i_ino, "%ld", inode->i_ino);
  const char *token = inode->i_sb->s_fs_info;
  struct entries * buffer = (struct entries *) kmalloc(sizeof (struct entries), GFP_KERNEL);
  int64_t error =
      networkfs_http_call(token, "list", (char *)buffer,
                          sizeof(struct entries), 1, "inode", i_ino);
  if (error) {
    printk(KERN_WARNING "http call for list return %lld\n", error);
    kfree(buffer);
    return error;
  }
  size_t entries_count = buffer->entries_count;
  printk(KERN_INFO "entries %ld\n", entries_count);
  if (offset >= entries_count) {
    kfree(buffer);
    return stored;
  }
  for (size_t i = 0; i < entries_count; ++i) {
    unsigned char ftype = buffer->entries[i].entry_type;
    printk(KERN_INFO "flag is %d\n", ftype);
    ino_t dino = buffer->entries[i].ino;
    printk(KERN_INFO "ino is %ld\n", dino);
    memcpy(&fsname, buffer->entries[i].name, 256);
    printk(KERN_INFO "file : \"%s\"\n", fsname);

    dir_emit(ctx, fsname, strlen(fsname), dino, ftype);
    stored++;
    offset++;
    ctx->pos = offset;
  }
  kfree(buffer);
  return 0;
}

const struct file_operations networkfs_dir_ops = {
    .iterate = networkfs_iterate,
};
