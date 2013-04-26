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
#include <sys/xattr.h>
#include <sys/file.h>

#include "aes-crypt.h"

typedef struct fuse_args fuse_args_t;
typedef struct fuse_bufvec fuse_bufvec_t;
typedef struct fuse_conn_info fuse_conn_info_t;
typedef struct fuse_file_info fuse_file_info_t;

typedef struct flock flock_t;
typedef struct stat stat_t;
typedef struct statvfs statvfs_t;
typedef struct timespec timespec_t;

typedef struct enc_dirp {
    DIR *dp;
    struct dirent *entry;
    off_t offset;
} enc_dirp_t;

static inline enc_dirp_t* get_dirp(fuse_file_info_t* fi) {
    return (enc_dirp_t*) (uintptr_t) fi->fh;
}

typedef struct fsState {
    char* basePath;
} fsState_t;


#define DEBUG

#define RETURN_FAILURE -1
#define RETURN_SUCCESS 0
#define PATHBUFSIZE 1024
#define PATHDELIMINATOR '/'
#define NULLTERM '\0'
#define TEMPNAME_PRE  "."
#define TEMPNAME_POST ".decrypt"

static int buildPath(const char* path, char* buf, size_t bufSize) {

    size_t size = 0;
    fsState_t* state = NULL;

    /* Input Checks */
    if(path == NULL) {
	fprintf(stderr, "ERROR buildPath: path must not be NULL\n");
	return RETURN_FAILURE;
    }
    if(buf == NULL) {
	fprintf(stderr, "ERROR buildPath: buf must not be NULL\n");
	return RETURN_FAILURE;
    }

    /* Get State */
    state = (fsState_t*)(fuse_get_context()->private_data);
    if(state == NULL) {
	fprintf(stderr, "ERROR buildPath: state must not be NULL\n");
	return RETURN_FAILURE;
    }

    /* Concatenate in Buffer */
    size = snprintf(buf, bufSize, "%s%s", state->basePath, path);
    if(size > (bufSize - 1)) {
	fprintf(stderr, "ERROR buildPath: length too large for buffer\n");
	return RETURN_FAILURE;
    }

#ifdef DEBUG
    fprintf(stderr, "INFO buildPath: buf = %s\n", buf);
#endif

    return RETURN_SUCCESS;
    
}

static int buildTempPath(const char* fullPath, char* tempPath, size_t bufSize) {

    char* pFileName = NULL;
    char buf[PATHBUFSIZE];
    size_t length;

    /* Input Checks */
    if(fullPath == NULL) {
	fprintf(stderr, "ERROR buildTempPath: fullPath must not be NULL\n");
	return RETURN_FAILURE;
    }
    if(tempPath == NULL) {
	fprintf(stderr, "ERROR buildTempPath: tempPath must not be NULL\n");
	return RETURN_FAILURE;
    }
    
    /* Copy input path to buf */
    length = snprintf(buf, sizeof(buf), "%s", fullPath);
    if(length > (sizeof(buf) - 1)) {
	fprintf(stderr, "ERROR buildTempPath: Overflowed buf\n");
	return RETURN_FAILURE;
    }
    
    /* Find start of file name */
    pFileName = strrchr(buf, PATHDELIMINATOR);
    if(pFileName == NULL) {
	fprintf(stderr, "ERROR buildTempPath: Could not find deliminator in path\n");
	return RETURN_FAILURE;
    }
    *pFileName = NULLTERM;
		      
    /* Build Temp Path */
    length = snprintf(tempPath, bufSize, "%s%c%s%s%s",
		      buf, PATHDELIMINATOR, TEMPNAME_PRE, (pFileName + 1), TEMPNAME_POST);
    if(length > (bufSize - 1)) {
	fprintf(stderr, "ERROR buildTempPath: Overflowed tempPath\n");
	return RETURN_FAILURE;
    }

#ifdef DEBUG
    fprintf(stderr, "INFO buildTempPath: tempPath = %s\n", tempPath);
#endif

    return RETURN_SUCCESS;

}


static int enc_getattr(const char* path, stat_t* stbuf) {
    
    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0) {
	fprintf(stderr, "ERROR enc_getattr: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(lstat(fullPath, stbuf) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_fgetattr(const char* path, stat_t* stbuf,
			fuse_file_info_t* fi) {

    (void) path;

    if(fstat(fi->fh, stbuf) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_access(const char* path, int mask) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_access: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(access(fullPath, mask) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_readlink(const char* path, char* buf, size_t size) {

    int res;
    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_readlink: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    res = readlink(fullPath, buf, (size-1));
    if(res < 0) {
	return -errno;
    }

    buf[res] = '\0';

    return RETURN_SUCCESS;

}

static int enc_opendir(const char* path, fuse_file_info_t* fi) {

    int res;
    enc_dirp_t* d = NULL;
    char fullPath[PATHBUFSIZE];

    d = malloc(sizeof(*d));
    if(d == NULL) {
	return -ENOMEM;
    }
    
    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_opendir: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    d->dp = opendir(fullPath);
    if(d->dp == NULL) {
	res = -errno;
	free(d);
	return res;
    }
    d->offset = 0;
    d->entry = NULL;

    fi->fh = (unsigned long) d;

    return RETURN_SUCCESS;

}

static int enc_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
		       off_t offset, fuse_file_info_t* fi) {

    enc_dirp_t* d = NULL;

    (void) path;

    d = get_dirp(fi);

    if (offset != d->offset) {
	seekdir(d->dp, offset);
	d->entry = NULL;
	d->offset = offset;
    }
    while (1) {
	stat_t st;
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

    return RETURN_SUCCESS;

}

static int enc_releasedir(const char* path, fuse_file_info_t* fi) {

    enc_dirp_t* d = NULL;

    (void) path;

    d = get_dirp(fi);
    closedir(d->dp);
    free(d);

    return RETURN_SUCCESS;

}

static int enc_mknod(const char* path, mode_t mode, dev_t rdev) {

    int res;
    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_mknod: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(S_ISFIFO(mode)) {
	res = mkfifo(fullPath, mode);
    }
    else {
	res = mknod(fullPath, mode, rdev);
    }
    if(res < 0) {
	return -errno;
    }
    
    return RETURN_SUCCESS;

}

static int enc_mkdir(const char* path, mode_t mode) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_mkdir: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(mkdir(fullPath, mode) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_unlink(const char* path) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_unlink: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(unlink(fullPath) < 0){
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_rmdir(const char* path) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_rmdir: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(rmdir(fullPath)) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_symlink(const char* from, const char* to) {

    char fullFrom[PATHBUFSIZE];
    char fullTo[PATHBUFSIZE];

    if(buildPath(from, fullFrom, sizeof(fullFrom)) < 0){
	fprintf(stderr, "ERROR enc_symlink: buildPath failed on from\n");
	return RETURN_FAILURE;
    }
    from = NULL;

    if(buildPath(to, fullTo, sizeof(fullTo)) < 0){
	fprintf(stderr, "ERROR enc_symlink: buildPath failed on to\n");
	return RETURN_FAILURE;
    }
    to = NULL;

    if(symlink(fullFrom, fullTo)) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_rename(const char* from, const char* to) {

    char fullFrom[PATHBUFSIZE];
    char fullTo[PATHBUFSIZE];

    if(buildPath(from, fullFrom, sizeof(fullFrom)) < 0){
	fprintf(stderr, "ERROR enc_rename: buildPath failed on from\n");
	return RETURN_FAILURE;
    }
    from = NULL;

    if(buildPath(to, fullTo, sizeof(fullTo)) < 0){
	fprintf(stderr, "ERROR enc_rename: buildPath failed on to\n");
	return RETURN_FAILURE;
    }
    to = NULL;

    if(rename(fullFrom, fullTo) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_link(const char* from, const char* to) {

    char fullFrom[PATHBUFSIZE];
    char fullTo[PATHBUFSIZE];

    if(buildPath(from, fullFrom, sizeof(fullFrom)) < 0){
	fprintf(stderr, "ERROR enc_link: buildPath failed on from\n");
	return RETURN_FAILURE;
    }
    from = NULL;

    if(buildPath(to, fullTo, sizeof(fullTo)) < 0){
	fprintf(stderr, "ERROR enc_link: buildPath failed on to\n");
	return RETURN_FAILURE;
    }
    to = NULL;

    if(link(fullFrom, fullTo) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_chmod(const char* path, mode_t mode) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_chmod: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(chmod(fullPath, mode) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_chown(const char* path, uid_t uid, gid_t gid) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_chown: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(lchown(fullPath, uid, gid) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_truncate(const char* path, off_t size) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_rmdir: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(truncate(fullPath, size)) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_ftruncate(const char* path, off_t size,
			 fuse_file_info_t* fi) {

    (void) path;

    if(ftruncate(fi->fh, size) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_utimens(const char* path, const timespec_t ts[2]) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_utimens: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    /* don't use utime/utimes since they follow symlinks */
    if(utimensat(0, fullPath, ts, AT_SYMLINK_NOFOLLOW) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_create(const char* path, mode_t mode, fuse_file_info_t* fi) {

    int fd;
    char fullPath[PATHBUFSIZE];
    char tempPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_create: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(buildTempPath(fullPath, tempPath, sizeof(tempPath)) < 0){
	fprintf(stderr, "ERROR enc_create: buildTempPath failed\n");
	return RETURN_FAILURE;
    }

    fd = open(fullPath, fi->flags, mode);
    if(fd < 0) {
	return -errno;
    }

    fi->fh = fd;

    return RETURN_SUCCESS;

}

static int enc_open(const char* path, fuse_file_info_t* fi) {

    int fd;
    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_open: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    fd = open(fullPath, fi->flags);
    if(fd < 0) {
	return -errno;
    }

    fi->fh = fd;

    return EXIT_SUCCESS;

}

static int enc_read(const char* path, char* buf, size_t size, off_t offset,
		    fuse_file_info_t* fi) {

    int res;

    (void) path;

    res = pread(fi->fh, buf, size, offset);
    if(res < 0) {
	res = -errno;
    }

    return res;

}

static int enc_write(const char* path, const char* buf, size_t size,
		     off_t offset, fuse_file_info_t* fi) {

    int res;

    (void) path;

    res = pwrite(fi->fh, buf, size, offset);
    if(res < 0) {
	res = -errno;
    }

    return res;

}

static int enc_statfs(const char* path, statvfs_t* stbuf) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_statfs: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(statvfs(fullPath, stbuf) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_flush(const char* path, fuse_file_info_t* fi) {

    (void) path;

    /* This is called from every close on an open file, so call the
       close on the underlying filesystem. But since flush may be
       called multiple times for an open file, this must not really
       close the file.  This is important if used on a network
       filesystem like NFS which flush the data/metadata on close() */

    if(close(dup(fi->fh)) < 0) {
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_release(const char* path, fuse_file_info_t* fi) {

    (void) path;

    close(fi->fh);

    return RETURN_SUCCESS;

}

static int enc_fsync(const char* path, int isdatasync,
		     fuse_file_info_t* fi) {

    int res;

    (void) path;

    if(isdatasync) {
	res = fdatasync(fi->fh);
    }
    else {
	res = fsync(fi->fh);
    }

    if(res < 0) {
	return -errno;
    }
    
    return RETURN_SUCCESS;
    
}
    
/* xattr operations are optional and can safely be left unimplemented */
static int enc_setxattr(const char* path, const char* name, const char* value,
			size_t size, int flags) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_setxattr: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(lsetxattr(fullPath, name, value, size, flags) < 0) {
        return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_getxattr(const char* path, const char* name, char* value,
			size_t size) {

    int res;
    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_getxattr: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    res = lgetxattr(fullPath, name, value, size);
    if(res < 0) {
        return -errno;
    }

    return res;

}

static int enc_listxattr(const char* path, char* list, size_t size) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_listxattr: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    int res = llistxattr(fullPath, list, size);
    if(res < 0) {
        return -errno;
    }

    return res;

}

static int enc_removexattr(const char* path, const char* name) {

    char fullPath[PATHBUFSIZE];

    if(buildPath(path, fullPath, sizeof(fullPath)) < 0){
	fprintf(stderr, "ERROR enc_removexattr: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    if(lremovexattr(fullPath, name) < 0) {
        return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_lock(const char* path, fuse_file_info_t* fi, int cmd,
		    flock_t* lock) {

    (void) path;

    return ulockmgr_op(fi->fh, cmd, lock, &fi->lock_owner,
		       sizeof(fi->lock_owner));

}

static int enc_flock(const char* path, fuse_file_info_t* fi, int op) {
    
    (void) path;
    
    if(flock(fi->fh, op) < 0) {
        return -errno;
    }
    
    return RETURN_SUCCESS;

}

static struct fuse_operations enc_oper = {

    /* Access Control */
    .access     = enc_access,       /* Check File Permissions */
    .lock       = enc_lock,         /* Lock File */    
    .flock      = enc_flock,        /* Lock Open File */

    /* Metadata */
    .chmod      = enc_chmod,        /* Change File Permissions */
    .chown      = enc_chown,        /* Change File Owner */
    .getattr    = enc_getattr,      /* Get File Attributes */
    .fgetattr   = enc_fgetattr,     /* Get Open File Attributes  */
    .statfs     = enc_statfs,       /* Get File System Statistics */
    .utimens    = enc_utimens,      /* Change the Times of a File*/

    /* Create and Delete */
    .create     = enc_create,       /* Create and Open a Regular File */
    .mkdir      = enc_mkdir,        /* Create a Directory */
    .mknod      = enc_mknod,        /* Create a Non-Regular File Node */
    .link       = enc_link,         /* Create a Hard Link */    
    .symlink    = enc_symlink,      /* Create a Symbolic Link */	
    .rmdir      = enc_rmdir,        /* Remove a Directory */
    .unlink     = enc_unlink,       /* Remove a File */

    /* Open and Close */
    .open       = enc_open,         /* Open a File */
    .opendir    = enc_opendir,      /* Open a Directory */
    .release    = enc_release,      /* Release an Open File */
    .releasedir = enc_releasedir,   /* Release an Open Directory */    

    /* Read and Write */
    .read        = enc_read,        /* Read a File */
    .readdir     = enc_readdir,     /* Read a Directory */
    .readlink    = enc_readlink,    /* Read the Target of a Symbolic Link */
    .write       = enc_write,       /* Write a File*/

    /* Modify */
    .rename      = enc_rename,      /* Rename a File */
    .truncate    = enc_truncate,    /* Change the Size of a File */
    .ftruncate   = enc_ftruncate,   /* Change the Size of an Open File*/    

    /* Buffering */
    .flush       = enc_flush,       /* Flush Cached Data */
    .fsync       = enc_fsync,       /* Synch Open File Contents */
    
    /* Extended Attributes */
    .setxattr    = enc_setxattr,    /* Set XATTR */
    .getxattr    = enc_getxattr,    /* Get XATTR */
    .listxattr   = enc_listxattr,   /* List XATTR */
    .removexattr = enc_removexattr, /* Remove XATTR */

    /* Flags */
    .flag_nullpath_ok   = 1,
    .flag_utime_omit_ok = 1,

};

int main(int argc, char *argv[]) {

    fuse_args_t args = FUSE_ARGS_INIT(0, NULL);
    fsState_t state;
    int i;

    if(argc < 3){
	fprintf(stderr,
		"Usage:\n %s <Mount Point> <Mirrored Directory>\n",
		argv[0]);
	exit(EXIT_FAILURE);
    }

    for(i = 0; i < argc; i++) {
	if (i == 2)
	    state.basePath = realpath(argv[i], NULL);
	else
	    fuse_opt_add_arg(&args, argv[i]);
    }

    umask(0);     

    return fuse_main(args.argc, args.argv, &enc_oper, &state);

}
