/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>
  Copyright (C) 2013       Andy Sayler <www.andysayler.com>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall `pkg-config fuse --cflags` fuseenc.c -o fuseenc `pkg-config fuse --libs`

  Note: This implementation is largely stateless and does not maintain
        open file handels between open and release calls (fi->fh).
        Instead, files are opened and closed as necessary inside read(), write(),
        etc calls. As such, the functions that rely on maintaining file handles are
        not implmented (fgetattr(), etc). Those seeking a more efficient and
        more complete implementation may wish to add fi->fh support to minimize
        open() and close() calls and support fh dependent functions.

*/

#define FUSE_USE_VERSION 29
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
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

static int enc_getattr(const char *path, struct stat *stbuf)
{
    int res;

    res = lstat(path, stbuf);
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


static int enc_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;

    dp = opendir(path);
    if (dp == NULL)
	return -errno;

    while ((de = readdir(dp)) != NULL) {
	struct stat st;
	memset(&st, 0, sizeof(st));
	st.st_ino = de->d_ino;
	st.st_mode = de->d_type << 12;
	if (filler(buf, de->d_name, &st, 0))
	    break;
    }

    closedir(dp);
    return 0;
}

static int enc_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int res;

    /* On Linux this could just be 'mknod(path, mode, rdev)' but this
       is more portable */
    if (S_ISREG(mode)) {
	res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
	if (res >= 0)
	    res = close(res);
    } else if (S_ISFIFO(mode))
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

static int enc_open(const char *path, struct fuse_file_info *fi)
{
    int res;

    res = open(path, fi->flags);
    if (res == -1)
	return -errno;

    close(res);
    return 0;
}

static int enc_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    int fd;
    int res;

    (void) fi;
    fd = open(path, O_RDONLY);
    if (fd == -1)
	return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
	res = -errno;

    close(fd);
    return res;
}

static int enc_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
    int fd;
    int res;

    (void) fi;
    fd = open(path, O_WRONLY);
    if (fd == -1)
	return -errno;

    res = pwrite(fd, buf, size, offset);
    if (res == -1)
	res = -errno;

    close(fd);
    return res;
}

static int enc_statfs(const char *path, struct statvfs *stbuf)
{
    int res;

    res = statvfs(path, stbuf);
    if (res == -1)
	return -errno;

    return 0;
}

static int enc_release(const char *path, struct fuse_file_info *fi)
{
    /* Just a stub. This method is optional and can safely be left
       unimplemented */

    (void) path;
    (void) fi;
    return 0;
}

static int enc_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
    /* Just a stub. This method is optional and can safely be left
       unimplemented */

    (void) path;
    (void) isdatasync;
    (void) fi;
    return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int enc_fallocate(const char *path, int mode,
			 off_t offset, off_t length, struct fuse_file_info *fi)
{
    int fd;
    int res;

    (void) fi;

    if (mode)
	return -EOPNOTSUPP;

    fd = open(path, O_WRONLY);
    if (fd == -1)
	return -errno;

    res = -posix_fallocate(fd, offset, length);

    close(fd);
    return res;
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

static struct fuse_operations enc_oper = {
    .getattr= enc_getattr,
    .access= enc_access,
    .readlink= enc_readlink,
    .readdir= enc_readdir,
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
#ifdef HAVE_UTIMENSAT
    .utimens= enc_utimens,
#endif
    .open= enc_open,
    .read= enc_read,
    .write= enc_write,
    .statfs= enc_statfs,
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
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &enc_oper, NULL);
}
