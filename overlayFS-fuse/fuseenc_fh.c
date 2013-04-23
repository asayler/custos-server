/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>
  Copyright (C) 2013       Andy Sayler <www.andysayler.com>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall fuseenc_fh.c `pkg-config fuse --cflags --libs` -lulockmgr -o fuseenc_fh
*/

#define FUSE_USE_VERSION 29
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <fuse.h>
#include <ulockmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include <sys/file.h> /* flock(2) */

static int enc_getattr(const char *path, struct stat *stbuf)
{
    int res;

    res = lstat(path, stbuf);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_fgetattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)
{
    int res;

    (void) path;

    res = fstat(fi->fh, stbuf);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_access(const char *path, int mask)
{
    int res;

    res = access(path, mask);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_readlink(const char *path, char *buf, size_t size)
{
    int res;

    res = readlink(path, buf, size - 1);
    if (res == -1)
	return -errno;

    buf[res] = '\0';
    return 0;
}

struct enc_dirp {
    DIR *dp;
    struct dirent *entry;
    off_t offset;
};

static int enc_opendir(const char *path, struct fuse_file_info *fi)
{
    int res;
    struct enc_dirp *d = malloc(sizeof(struct enc_dirp));
    if (d == NULL)
	return -ENOMEM;

    d->dp = opendir(path);
    if (d->dp == NULL) {
	res = -errno;
	free(d);
	return res;
    }
    d->offset = 0;
    d->entry = NULL;

    fi->fh = (unsigned long) d;
    return 0;
}

static inline struct enc_dirp *get_dirp(struct fuse_file_info *fi)
{
    return (struct enc_dirp *) (uintptr_t) fi->fh;
}

static int enc_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    struct enc_dirp *d = get_dirp(fi);

    (void) path;
    if (offset != d->offset) {
	seekdir(d->dp, offset);
	d->entry = NULL;
	d->offset = offset;
    }
    while (1) {
	struct stat st;
	off_t nextoff;

	if (!d->entry) {
	    d->entry = readdir(d->dp);
	    if (!d->entry)
		break;
	}

	memset(&st, 0, sizeof(st));
	st.st_ino = d->entry->d_ino;
	st.st_mode = d->entry->d_type << 12;
	nextoff = telldir(d->dp);
	if (filler(buf, d->entry->d_name, &st, nextoff))
	    break;

	d->entry = NULL;
	d->offset = nextoff;
    }

    return 0;
}

static int enc_releasedir(const char *path, struct fuse_file_info *fi)
{
    struct enc_dirp *d = get_dirp(fi);
    (void) path;
    closedir(d->dp);
    free(d);
    return 0;
}

static int enc_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int res;

    if (S_ISFIFO(mode))
	res = mkfifo(path, mode);
    else
	res = mknod(path, mode, rdev);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_mkdir(const char *path, mode_t mode)
{
    int res;

    res = mkdir(path, mode);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_unlink(const char *path)
{
    int res;

    res = unlink(path);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_rmdir(const char *path)
{
    int res;

    res = rmdir(path);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_symlink(const char *from, const char *to)
{
    int res;

    res = symlink(from, to);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_rename(const char *from, const char *to)
{
    int res;

    res = rename(from, to);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_link(const char *from, const char *to)
{
    int res;

    res = link(from, to);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_chmod(const char *path, mode_t mode)
{
    int res;

    res = chmod(path, mode);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_chown(const char *path, uid_t uid, gid_t gid)
{
    int res;

    res = lchown(path, uid, gid);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_truncate(const char *path, off_t size)
{
    int res;

    res = truncate(path, size);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_ftruncate(const char *path, off_t size,
			 struct fuse_file_info *fi)
{
    int res;

    (void) path;

    res = ftruncate(fi->fh, size);
    if (res == -1)
	return -errno;

    return 0;
}

#ifdef HAVE_UTIMENSAT
static int enc_utimens(const char *path, const struct timespec ts[2])
{
    int res;

    /* don't use utime/utimes since they follow symlinks */
    res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
    if (res == -1)
	return -errno;

    return 0;
}
#endif

static int enc_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int fd;

    fd = open(path, fi->flags, mode);
    if (fd == -1)
	return -errno;

    fi->fh = fd;
    return 0;
}

static int enc_open(const char *path, struct fuse_file_info *fi)
{
    int fd;

    fd = open(path, fi->flags);
    if (fd == -1)
	return -errno;

    fi->fh = fd;
    return 0;
}

static int enc_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    int res;

    (void) path;
    res = pread(fi->fh, buf, size, offset);
    if (res == -1)
	res = -errno;

    return res;
}

static int enc_read_buf(const char *path, struct fuse_bufvec **bufp,
			size_t size, off_t offset, struct fuse_file_info *fi)
{
    struct fuse_bufvec *src;

    (void) path;

    src = malloc(sizeof(struct fuse_bufvec));
    if (src == NULL)
	return -ENOMEM;

    *src = FUSE_BUFVEC_INIT(size);

    src->buf[0].flags = FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK;
    src->buf[0].fd = fi->fh;
    src->buf[0].pos = offset;

    *bufp = src;

    return 0;
}

static int enc_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
    int res;

    (void) path;
    res = pwrite(fi->fh, buf, size, offset);
    if (res == -1)
	res = -errno;

    return res;
}

static int enc_write_buf(const char *path, struct fuse_bufvec *buf,
			 off_t offset, struct fuse_file_info *fi)
{
    struct fuse_bufvec dst = FUSE_BUFVEC_INIT(fuse_buf_size(buf));

    (void) path;

    dst.buf[0].flags = FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK;
    dst.buf[0].fd = fi->fh;
    dst.buf[0].pos = offset;

    return fuse_buf_copy(&dst, buf, FUSE_BUF_SPLICE_NONBLOCK);
}

static int enc_statfs(const char *path, struct statvfs *stbuf)
{
    int res;

    res = statvfs(path, stbuf);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_flush(const char *path, struct fuse_file_info *fi)
{
    int res;

    (void) path;
    /* This is called from every close on an open file, so call the
       close on the underlying filesystem.But since flush may be
          called multiple times for an open file, this must not really
	     close the file.  This is important if used on a network
	     filesystem like NFS which flush the data/metadata on close() */
    res = close(dup(fi->fh));
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_release(const char *path, struct fuse_file_info *fi)
{
    (void) path;
    close(fi->fh);

    return 0;
}

static int enc_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
    int res;
    (void) path;

#ifndef HAVE_FDATASYNC
    (void) isdatasync;
#else
    if (isdatasync)
	res = fdatasync(fi->fh);
    else
#endif
	res = fsync(fi->fh);
    if (res == -1)
	return -errno;

    return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int enc_fallocate(const char *path, int mode,
			 off_t offset, off_t length, struct fuse_file_info *fi)
{
    (void) path;

    if (mode)
	return -EOPNOTSUPP;

    return -posix_fallocate(fi->fh, offset, length);
}
#endif

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int enc_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
    int res = lsetxattr(path, name, value, size, flags);
    if (res == -1)
	return -errno;
    return 0;
}

static int enc_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
    int res = lgetxattr(path, name, value, size);
    if (res == -1)
	return -errno;
    return res;
}

static int enc_listxattr(const char *path, char *list, size_t size)
{
    int res = llistxattr(path, list, size);
    if (res == -1)
	return -errno;
    return res;
}

static int enc_removexattr(const char *path, const char *name)
{
    int res = lremovexattr(path, name);
    if (res == -1)
	return -errno;
    return 0;
}
#endif /* HAVE_SETXATTR */

static int enc_lock(const char *path, struct fuse_file_info *fi, int cmd,
		    struct flock *lock)
{
    (void) path;

    return ulockmgr_op(fi->fh, cmd, lock, &fi->lock_owner,
		       sizeof(fi->lock_owner));
}

static int enc_flock(const char *path, struct fuse_file_info *fi, int op)
{
    int res;
    (void) path;

    res = flock(fi->fh, op);
    if (res == -1)
	return -errno;

    return 0;
}

static struct fuse_operations enc_oper = {
    .getattr= enc_getattr,
    .fgetattr= enc_fgetattr,
    .access= enc_access,
    .readlink= enc_readlink,
    .opendir= enc_opendir,
    .readdir= enc_readdir,
    .releasedir= enc_releasedir,
    .mknod= enc_mknod,
    .mkdir= enc_mkdir,
    .symlink= enc_symlink,
    .unlink= enc_unlink,
    .rmdir= enc_rmdir,
    .rename= enc_rename,
    .link= enc_link,
    .chmod= enc_chmod,
    .chown= enc_chown,
    .truncate= enc_truncate,
    .ftruncate= enc_ftruncate,
#ifdef HAVE_UTIMENSAT
    .utimens= enc_utimens,
#endif
    .create= enc_create,
    .open= enc_open,
    .read= enc_read,
    .read_buf= enc_read_buf,
    .write= enc_write,
    .write_buf= enc_write_buf,
    .statfs= enc_statfs,
    .flush= enc_flush,
    .release= enc_release,
    .fsync= enc_fsync,
#ifdef HAVE_POSIX_FALLOCATE
    .fallocate= enc_fallocate,
#endif
#ifdef HAVE_SETXATTR
    .setxattr= enc_setxattr,
    .getxattr= enc_getxattr,
    .listxattr= enc_listxattr,
    .removexattr= enc_removexattr,
#endif
    .lock= enc_lock,
    .flock= enc_flock,

    .flag_nullpath_ok = 1,
#if HAVE_UTIMENSAT
    .flag_utime_omit_ok = 1,
#endif
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &enc_oper, NULL);
}
