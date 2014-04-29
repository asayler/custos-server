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
#include "libcustos/custos_client.h"

typedef struct fuse_args fuse_args_t;
typedef struct fuse_bufvec fuse_bufvec_t;
typedef struct fuse_conn_info fuse_conn_info_t;
typedef struct fuse_file_info fuse_file_info_t;

typedef struct flock flock_t;
typedef struct stat stat_t;
typedef struct statvfs statvfs_t;
typedef struct timespec timespec_t;

#define FHS_DIRTY 1
#define FHS_CLEAN 0

typedef struct enc_fhs {
    uint64_t encFH;
    uint64_t clearFH;
    char     dirty;
    char     padding[7];
} enc_fhs_t;

static inline enc_fhs_t* get_fhs(uint64_t fh) {
    return (enc_fhs_t*) fh;
}

static inline uint64_t put_fhs(enc_fhs_t* fhs) {
    return (uint64_t) fhs;
}

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

#define TESTKEY "Password"

static int buildPath(const char* path, char* buf, size_t bufSize) {

    size_t size = 0;
    fsState_t* state = NULL;

    /* Input Checks */
    if(path == NULL) {
	fprintf(stderr, "ERROR buildPath: path must not be NULL\n");
	return -EINVAL;
    }
    if(buf == NULL) {
	fprintf(stderr, "ERROR buildPath: buf must not be NULL\n");
	return -EINVAL;
    }

    /* Get State */
    state = (fsState_t*)(fuse_get_context()->private_data);
    if(state == NULL) {
	fprintf(stderr, "ERROR buildPath: state must not be NULL\n");
	return -EINVAL;
    }

    /* Concatenate in Buffer */
    size = snprintf(buf, bufSize, "%s%s", state->basePath, path);
    if(size > (bufSize - 1)) {
	fprintf(stderr, "ERROR buildPath: length too large for buffer\n");
	return -ENAMETOOLONG;
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
	return -EINVAL;
    }
    if(tempPath == NULL) {
	fprintf(stderr, "ERROR buildTempPath: tempPath must not be NULL\n");
	return -EINVAL;
    }

    /* Copy input path to buf */
    length = snprintf(buf, sizeof(buf), "%s", fullPath);
    if(length > (sizeof(buf) - 1)) {
	fprintf(stderr, "ERROR buildTempPath: Overflowed buf\n");
	return -ENAMETOOLONG;
    }

    /* Find start of file name */
    pFileName = strrchr(buf, PATHDELIMINATOR);
    if(pFileName == NULL) {
	fprintf(stderr, "ERROR buildTempPath: Could not find deliminator in path\n");
	return -EINVAL;
    }
    *pFileName = NULLTERM;

    /* Build Temp Path */
    length = snprintf(tempPath, bufSize, "%s%c%s%s%s",
		      buf, PATHDELIMINATOR, TEMPNAME_PRE, (pFileName + 1), TEMPNAME_POST);
    if(length > (bufSize - 1)) {
	fprintf(stderr, "ERROR buildTempPath: Overflowed tempPath\n");
	return -ENAMETOOLONG;
    }

#ifdef DEBUG
    fprintf(stderr, "INFO buildTempPath: tempPath = %s\n", tempPath);
#endif

    return RETURN_SUCCESS;

}

static enc_fhs_t* createFilePair(const char* encPath, const char* clearPath,
				 int flags, mode_t mode) {

    int ret;
    enc_fhs_t* fhs = NULL;

    fhs = malloc(sizeof(*fhs));
    if(!fhs) {
	fprintf(stderr, "ERROR createFilePair: malloc failed\n");
	perror("ERROR createFilePair");
	return NULL;
    }

    ret = open(encPath, flags, mode);
    if(ret < 0) {
	fprintf(stderr, "ERROR createFilePair: open(encPath) failed\n");
	perror("ERROR createFilePair");
	return NULL;
    }
    fhs->encFH = ret;

    ret = open(clearPath, flags, mode);
    if(ret < 0) {
	fprintf(stderr, "ERROR createFilePair: open(clearPath) failed\n");
	perror("ERROR createFilePair");
	return NULL;
    }
    fhs->clearFH = ret;

    return fhs;

}

static enc_fhs_t* openFilePair(const char* encPath, const char* clearPath,
			       int flags) {

    int ret;
    enc_fhs_t* fhs = NULL;

    fhs = malloc(sizeof(*fhs));
    if(!fhs) {
	fprintf(stderr, "ERROR openFilePair: malloc failed\n");
	perror("ERROR openFilePair");
	return NULL;
    }

    ret = open(encPath, flags);
    if(ret < 0) {
	fprintf(stderr, "ERROR openFilePair: open(encPath) failed\n");
	perror("ERROR openFilePair");
	return NULL;
    }
    fhs->encFH = ret;

    ret = open(clearPath, flags);
    if(ret < 0) {
	fprintf(stderr, "ERROR openFilePair: open(clearPath) failed\n");
	perror("ERROR openFilePair");
	return NULL;
    }
    fhs->clearFH = ret;

    return fhs;

}

static int closeFilePair(enc_fhs_t* fhs) {

    if(!fhs) {
	fprintf(stderr, "ERROR closeFilePair: fhs must not be NULL\n");
   	return -EINVAL;
    }

    if(close(fhs->encFH) < 0) {
	fprintf(stderr, "ERROR closeFilePair: close(encFH) failed\n");
	perror("ERROR enc_release");
	return -errno;
    }

    if(close(fhs->clearFH) < 0) {
	fprintf(stderr, "ERROR closeFilePair: close(clearFH) failed\n");
	perror("ERROR enc_release");
	return -errno;
    }

    free(fhs);

    return RETURN_SUCCESS;

}

static int removeFile(const char* filePath) {

    int ret;

#ifdef DEBUG
    fprintf(stderr, "INFO removeFile: function called on %s\n", filePath);
#endif

    ret = unlink(filePath);
    if(ret < 0) {
	fprintf(stderr, "ERROR removeFile: unlink failed\n");
	perror("ERROR removeTemp");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int decryptFile(const char* encPath, const char* plainPath) {

    int ret;
    //int i;
    FILE* encFP = NULL;
    FILE* plainFP = NULL;
    //custosKeyReq_t* req = NULL;
    //custosKeyRes_t* res = NULL;
    //uuid_t uuid;
    char* key;

    encFP = fopen(encPath, "r");
    if(!encFP) {
        fprintf(stderr, "ERROR decryptFile: fopen(encPath) failed\n");
        perror("ERROR decryptFile");
        ret = -errno;
        goto ERROR_0;
    }

    plainFP = fopen(plainPath, "w");
    if(!plainFP) {
        fprintf(stderr, "ERROR decryptFile: fopen(plainPath) failed\n");
        perror("ERROR decryptFile");
        ret = -errno;
        goto ERROR_1;
    }

    /* /\* Create a new Custos request *\/ */
    /* uuid_generate(uuid); */
    /* req = custos_createKeyReq(uuid, "http://test.com"); */
    /* if(!req) { */
	/* fprintf(stderr, "ERROR decryptFile: custos_createKeyReq failed\n"); */
	/* ret = -errno; */
	/* goto ERROR_2; */
    /* } */

    /* /\* Get Key - 1st Attempt *\/ */
    /* res = custos_getKeyRes(req); */
    /* if(!res) { */
	/* fprintf(stderr, "ERROR decryptFile: custos_getKeyRes failed\n"); */
	/* ret = -errno; */
	/* goto ERROR_3; */
    /* } */

    /* if(res->resStat) { */
	/* fprintf(stderr, "ERROR decryptFile: response error %d\n", res->resStat); */
	/* ret = -errno; */
	/* goto ERROR_4; */
    /* } */

    /* if(!(res->key)) { */

    /*     /\* Update Request *\/ */
    /*     for(i = 0; i < CUS_ATTRID_MAX; i++) { */
    /*         if(res->attrStat[i] == CUS_ATTRSTAT_REQ) { */
    /*             switch(i) { */
    /*             case CUS_ATTRID_PSK: */
    /*                 ret = custos_updateKeyReq(req, i, CUS_TEST_PSK, */
    /*                                           (strlen(CUS_TEST_PSK) + 1)); */
    /*                 if(ret < 0) { */
    /*                     fprintf(stderr, "ERROR decryptFile: custos_updateKeyReq failed\n"); */
    /*                     goto ERROR_4; */
    /*                 } */
    /*                 break; */
    /*             default: */
    /*                 fprintf(stderr, "ERROR decryptFile: Unknown Custos Attr %d required\n", i); */
    /*                 goto ERROR_4; */
    /*                 break; */
    /*             } */
    /*         } */
    /*         else { */
    /*             fprintf(stderr, "ERROR decryptFile: Custos Attr %d Error %d\n", */
    /*                     i, res->attrStat[i]); */
    /*             goto ERROR_4; */
    /*         } */
    /*     } */

    /*     /\* Free Response *\/ */
    /*     ret = custos_destroyKeyRes(&res); */
    /*     if(ret < 0) { */
    /*         fprintf(stderr, "ERROR decryptFile: custos_destroyKeyRes failed\n"); */
    /*         goto ERROR_3; */
    /*     } */

    /*     /\* Get Key - 2nd Attempt *\/ */
    /*     res = custos_getKeyRes(req); */
    /*     if(!res) { */
    /*         fprintf(stderr, "ERROR decryptFile: custos_getKeyRes failed\n"); */
    /*         ret = -errno; */
    /*         goto ERROR_3; */
    /*     } */

    /*     if(res->resStat) { */
    /*         fprintf(stderr, "ERROR decryptFile: response error %d\n", res->resStat); */
    /*         ret = -errno; */
    /*         goto ERROR_4; */
    /*     } */

    /*     if(!(res->key)) { */
    /*         fprintf(stderr, "ERROR decryptFile: request failed\n"); */
    /*         ret = RETURN_FAILURE; */
    /*         goto ERROR_4; */
    /*     } */
    /* } */

    /* Decrypt */
    //key = (char*)(res->key);
    key = TESTKEY;
    ret = crypt_decrypt(encFP, plainFP, key);
    if(ret < 0) {
        fprintf(stderr, "ERROR decryptFile: crypt_decrypt() failed\n");
        goto ERROR_2;
    }

    /* /\* Free Response *\/ */
    /* ret = custos_destroyKeyRes(&res); */
    /* if(ret < 0) { */
	/* fprintf(stderr, "ERROR decryptFile: custos_destroyKeyRes failed\n"); */
	/* return EXIT_FAILURE; */
    /* } */

    /* /\* Free Request *\/ */
    /* ret = custos_destroyKeyReq(&req); */
    /* if(ret < 0) { */
	/* fprintf(stderr, "ERROR decryptFile: custos_destroyKeyReq failed\n"); */
	/* return EXIT_FAILURE; */
    /* } */

    if(fclose(plainFP)) {
        fprintf(stderr, "ERROR decryptFile: fclose(plainFP) failed\n");
        perror("ERROR decryptFile");
        ret = -errno;
        goto ERROR_1;
    }

    if(fclose(encFP)) {
        fprintf(stderr, "ERROR decryptFile: fclose(encFP) failed\n");
        perror("ERROR decryptFile");
        ret = -errno;
        goto ERROR_0;
    }

    return RETURN_SUCCESS;

    /* ERROR_4: */
    /*    ret = custos_destroyKeyRes(&res); */
    /*    if(ret < 0) { */
    /*    fprintf(stderr, "ERROR decryptFile: custos_destroyKeyRes failed\n"); */
    /*    } */

    /* ERROR_3: */
    /*    ret = custos_destroyKeyReq(&req); */
    /*    if(ret < 0) { */
    /*    fprintf(stderr, "ERROR decryptFile: custos_destroyKeyReq failed\n"); */
    /*    } */

 ERROR_2:

    if(fclose(plainFP)) {
        fprintf(stderr, "ERROR decryptFile: fclose(plainFP) failed\n");
        perror("ERROR decryptFile");
        ret = -errno;
    }

 ERROR_1:

    if(fclose(encFP)) {
        fprintf(stderr, "ERROR decryptFile: fclose(encFP) failed\n");
        perror("ERROR decryptFile");
        ret = -errno;
    }

 ERROR_0:

    return ret;

}

static int encryptFile(const char* plainPath, const char* encPath) {

    int ret;
    //int i;
    FILE* plainFP = NULL;
    FILE* encFP = NULL;
    //custosKeyReq_t* req = NULL;
    //custosKeyRes_t* res = NULL;
    //uuid_t uuid;
    char* key;

    plainFP = fopen(plainPath, "r");
    if(!plainFP) {
        fprintf(stderr, "ERROR encryptFile: fopen(%s) failed\n", plainPath);
        perror("ERROR encryptFile");
        ret = -errno;
        goto ERROR_0;
    }

    encFP = fopen(encPath, "w");
    if(!encFP) {
        fprintf(stderr, "ERROR encryptFile: fopen(%s) failed\n", encPath);
        perror("ERROR encryptFile");
        ret = -errno;
        goto ERROR_1;
    }

    /* /\* Create a new Custos request *\/ */
    /* uuid_generate(uuid); */
    /* req = custos_createKeyReq(uuid, "http://test.com"); */
    /* if(!req) { */
    /*     fprintf(stderr, "ERROR encryptFile: custos_createKeyReq failed\n"); */
    /*     ret = -errno; */
    /*     goto ERROR_2; */
    /* } */

    /* /\* Get Key - 1st Attempt *\/ */
    /* res = custos_getKeyRes(req); */
    /* if(!res) { */
    /*     fprintf(stderr, "ERROR encryptFile: custos_getKeyRes failed\n"); */
    /*     ret = -errno; */
    /*     goto ERROR_3; */
    /* } */

    /* if(res->resStat) { */
    /*     fprintf(stderr, "ERROR encryptFile: response error %d\n", res->resStat); */
    /*     ret = -errno; */
    /*     goto ERROR_4; */
    /* } */

    /* if(!(res->key)) { */

    /*     /\* Update Request *\/ */
    /*     for(i = 0; i < CUS_ATTRID_MAX; i++) { */
    /*         if(res->attrStat[i] == CUS_ATTRSTAT_REQ) { */
    /*             switch(i) { */
    /*             case CUS_ATTRID_PSK: */
    /*                 ret = custos_updateKeyReq(req, i, CUS_TEST_PSK, */
    /*                                           (strlen(CUS_TEST_PSK) + 1)); */
    /*                 if(ret < 0) { */
    /*                     fprintf(stderr, "ERROR encryptFile: custos_updateKeyReq failed\n"); */
    /*                     goto ERROR_4; */
    /*                 } */
    /*                 break; */
    /*             default: */
    /*                 fprintf(stderr, "ERROR encryptFile: Unknown Custos Attr %d required\n", i); */
    /*                 goto ERROR_4; */
    /*                 break; */
    /*             } */
    /*         } */
    /*         else { */
    /*             fprintf(stderr, "ERROR encryptFile: Custos Attr %d Error %d\n", */
    /*                     i, res->attrStat[i]); */
    /*             goto ERROR_4; */
    /*         } */
    /*     } */

    /*     /\* Free Response *\/ */
    /*     ret = custos_destroyKeyRes(&res); */
    /*     if(ret < 0) { */
    /*         fprintf(stderr, "ERROR encryptFile: custos_destroyKeyRes failed\n"); */
    /*         goto ERROR_3; */
    /*     } */

    /*     /\* Get Key - 2nd Attempt *\/ */
    /*     res = custos_getKeyRes(req); */
    /*     if(!res) { */
    /*         fprintf(stderr, "ERROR encryptFile: custos_getKeyRes failed\n"); */
    /*         ret = -errno; */
    /*         goto ERROR_3; */
    /*     } */

    /*     if(res->resStat) { */
    /*         fprintf(stderr, "ERROR encryptFile: response error %d\n", res->resStat); */
    /*         ret = -errno; */
    /*         goto ERROR_4; */
    /*     } */

    /*     if(!(res->key)) { */
    /*         fprintf(stderr, "ERROR encryptFile: request failed\n"); */
    /*         ret = RETURN_FAILURE; */
    /*         goto ERROR_4; */
    /*     } */
    /* } */

    //key = (char*)(res->key);
    key = TESTKEY;
    ret = crypt_encrypt(plainFP, encFP, key);
    if(ret < 0) {
        fprintf(stderr, "ERROR encryptFile: crypt_encrypt() failed\n");
        goto ERROR_2;
    }

    /* /\* Free Response *\/ */
    /* ret = custos_destroyKeyRes(&res); */
    /* if(ret < 0) { */
    /*     fprintf(stderr, "ERROR encryptFile: custos_destroyKeyRes failed\n"); */
    /*     return EXIT_FAILURE; */
    /* } */

    /* /\* Free Request *\/ */
    /* ret = custos_destroyKeyReq(&req); */
    /* if(ret < 0) { */
    /*     fprintf(stderr, "ERROR encryptFile: custos_destroyKeyReq failed\n"); */
    /*     return EXIT_FAILURE; */
    /* } */

    if(fclose(encFP)) {
        fprintf(stderr, "ERROR encryptFile: fclose(encFP) failed\n");
        perror("ERROR encryptFile");
        ret = -errno;
        goto ERROR_1;
    }

    if(fclose(plainFP)) {
        fprintf(stderr, "ERROR encryptFile: fclose(plainFP) failed\n");
        perror("ERROR encryptFile");
        ret = -errno;
        goto ERROR_0;
    }

    return RETURN_SUCCESS;

 /* ERROR_4: */
 /*    ret = custos_destroyKeyRes(&res); */
 /*    if(ret < 0) { */
 /*        fprintf(stderr, "ERROR encryptFile: custos_destroyKeyRes failed\n"); */
 /*    } */

 /* ERROR_3: */
 /*    ret = custos_destroyKeyReq(&req); */
 /*    if(ret < 0) { */
 /*        fprintf(stderr, "ERROR encryptFile: custos_destroyKeyReq failed\n"); */
 /*    } */

 ERROR_2:

    if(fclose(encFP)) {
        fprintf(stderr, "ERROR encryptFile: fclose(encFP) failed\n");
        perror("ERROR encryptFile");
        ret = -errno;
    }

 ERROR_1:

    if(fclose(plainFP)) {
        fprintf(stderr, "ERROR encryptFile: fclose(plainFP) failed\n");
        perror("ERROR encryptFile");
        ret = -errno;
    }

 ERROR_0:

    return ret;

}

static int enc_getattr(const char* path, stat_t* stbuf) {

    int ret;
    int exists;
    char fullPath[PATHBUFSIZE];
    char tempPath[PATHBUFSIZE];
    stat_t stTemp;

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_getattr: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = lstat(fullPath, stbuf);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_getattr: lstat(fullPath) failed\n");
	perror("ERROR enc_getattr");
	return -errno;
    }

    if(S_ISREG(stbuf->st_mode)) {

	ret = buildTempPath(fullPath, tempPath, sizeof(tempPath));
	if(ret < 0) {
	    fprintf(stderr, "ERROR enc_getattr: buildTempPath failed\n");
	    return ret;
	}

	if(access(tempPath, F_OK) < 0) {
	    exists = 0;
	}
	else {
	    exists = 1;
	}

	if(!exists) {
	    ret = decryptFile(fullPath, tempPath);
	    if(ret < 0) {
		fprintf(stderr, "ERROR enc_getattr: decryptFile failed\n");
		return ret;
	    }
	}

	ret = lstat(tempPath, &stTemp);
	if(ret < 0) {
	    fprintf(stderr, "ERROR enc_getattr: lstat(tempPath) failed\n");
	    perror("ERROR enc_getattr");
	    return -errno;
	}

	/* Copy over select fields */
	stbuf->st_size = stTemp.st_size;
	stbuf->st_blksize = stTemp.st_blksize;
	stbuf->st_blocks = stTemp.st_blocks;

	if(!exists) {
	    ret = removeFile(tempPath);
	    if(ret < 0) {
		fprintf(stderr, "ERROR enc_getattr: removeFile failed\n");
		return ret;
	    }
	}
    }

    return RETURN_SUCCESS;

}

static int enc_fgetattr(const char* path, stat_t* stbuf,
			fuse_file_info_t* fi) {

    (void) path;

    int ret;
    enc_fhs_t* fhs;
    stat_t stTemp;

    fhs = get_fhs(fi->fh);

    ret = fstat(fhs->encFH, stbuf);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_fgetattr: fstat(encFH) failed");
	perror("ERROR enc_fgetattr");
	return -errno;
    }

    ret = fstat(fhs->clearFH, &stTemp);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_fgetattr: fstat(clearFH) failed");
	perror("ERROR enc_fgetattr");
	return -errno;
    }

    /* Copy over select fields */
    stbuf->st_size = stTemp.st_size;
    stbuf->st_blksize = stTemp.st_blksize;
    stbuf->st_blocks = stTemp.st_blocks;

    return RETURN_SUCCESS;

}

static int enc_access(const char* path, int mask) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_access: buildPath failed\n");
	return ret;
    }
    path = NULL;

    /* Operating on the encrypted file should be fine here since we
       are only checking permissions */

    ret = access(fullPath, mask);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_access: access failed\n");
	perror("ERROR enc_access");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_readlink(const char* path, char* buf, size_t size) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_readlink: buildPath failed\n");
	return ret;
    }
    path = NULL;

    /* ToDo: Should this operate on the plain or encrypted file? */

    ret = readlink(fullPath, buf, (size-1));
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_readlink: readlink failed\n");
	perror("ERROR enc_readlink");
	return -errno;
    }

    buf[ret] = '\0';

    return RETURN_SUCCESS;

}

static int enc_opendir(const char* path, fuse_file_info_t* fi) {

    int ret;
    enc_dirp_t* d = NULL;
    char fullPath[PATHBUFSIZE];

    d = malloc(sizeof(*d));
    if(d == NULL) {
	fprintf(stderr, "ERROR enc_opendir: malloc failed\n");
	perror("ERROR enc_opendir");
	return -errno;
    }

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_opendir: buildPath failed\n");
	return ret;
    }
    path = NULL;

    d->dp = opendir(fullPath);
    if(d->dp == NULL) {
	fprintf(stderr, "ERROR enc_opendir: opendir failed\n");
	perror("ERROR enc_opendir");
	ret = -errno;
	free(d);
	return ret;
    }
    d->offset = 0;
    d->entry = NULL;

    fi->fh = (unsigned long) d;

    return RETURN_SUCCESS;

}

static int enc_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
		       off_t offset, fuse_file_info_t* fi) {

    (void) path;

    enc_dirp_t* d = NULL;

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

    (void) path;

    enc_dirp_t* d = NULL;

    d = get_dirp(fi);
    closedir(d->dp);
    free(d);

    return RETURN_SUCCESS;

}

static int enc_mknod(const char* path, mode_t mode, dev_t rdev) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_mknod: buildPath failed\n");
	return ret;
    }
    path = NULL;

    if(S_ISFIFO(mode)) {
	ret = mkfifo(fullPath, mode);
    }
    else {
	ret = mknod(fullPath, mode, rdev);
    }
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_mknod: mkfifo/mknode failed\n");
	perror("ERROR enc_mknod");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_mkdir(const char* path, mode_t mode) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_mkdir: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = mkdir(fullPath, mode);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_mkdir: mkdir failed\n");
	perror("ERROR enc_mkdir");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_unlink(const char* path) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_unlink: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = unlink(fullPath);
    if(ret < 0){
	fprintf(stderr, "ERROR enc_unlink: unlink failed\n");
	perror("ERROR enc_unlink");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_rmdir(const char* path) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_rmdir: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = rmdir(fullPath);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_rmdir: rmdir failed\n");
	perror("ERROR enc_rmdir");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_symlink(const char* from, const char* to) {

    /* ToDo: Are both from and to in the fuse FS? */

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

static int enc_link(const char* from, const char* to) {

    /* ToDo: Are both from and to in the fuse FS? */

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

static int enc_rename(const char* from, const char* to) {

    int ret;
    char fullFrom[PATHBUFSIZE];
    char fullTo[PATHBUFSIZE];

    ret = buildPath(from, fullFrom, sizeof(fullFrom));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_rename: buildPath(from) failed\n");
	return ret;
    }
    from = NULL;

    ret = buildPath(to, fullTo, sizeof(fullTo));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_rename: buildPath(to) failed\n");
	return ret;
    }
    to = NULL;

    ret = rename(fullFrom, fullTo);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_rename: rename failed\n");
	perror("ERROR enc_rename");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_chmod(const char* path, mode_t mode) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_chmod: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = chmod(fullPath, mode);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_chmod: chmod failed\n");
	perror("ERROR enc_chmod");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_chown(const char* path, uid_t uid, gid_t gid) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_chown: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = lchown(fullPath, uid, gid);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_chown: lchown failed\n");
	perror("ERROR enc_chown");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_truncate(const char* path, off_t size) {

    int ret;
    int exists;
    char fullPath[PATHBUFSIZE];
    char tempPath[PATHBUFSIZE];

#ifdef DEBUG
    fprintf(stderr, "INFO enc_truncate: funcation called\n");
#endif

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_truncate: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = buildTempPath(fullPath, tempPath, sizeof(tempPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_truncate: buildTempPath failed\n");
	return ret;
    }

    if(access(tempPath, F_OK) < 0) {
	exists = 0;
    }
    else {
	exists = 1;
    }

    if(!exists) {

	ret = decryptFile(fullPath, tempPath);
	if(ret < 0) {
	    fprintf(stderr, "ERROR enc_getattr: decryptFile failed\n");
	    return ret;

	}

    }

    ret = truncate(tempPath, size);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_truncate: truncate(tempPath) failed\n");
	perror("ERROR enc_truncate");
	return -errno;
    }

    ret = encryptFile(tempPath, fullPath);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_truncate: decryptFile failed\n");
	return ret;
    }

    if(!exists) {

	ret = removeFile(tempPath);
	if(ret < 0) {
	    fprintf(stderr, "ERROR enc_truncate: removeFile failed\n");
	    return ret;

	}

    }

#ifdef DEBUG
    fprintf(stderr, "INFO enc_truncate: funcation completed\n");
#endif

    return RETURN_SUCCESS;

}

static int enc_ftruncate(const char* path, off_t size,
			 fuse_file_info_t* fi) {

    (void) path;

    int ret;
    enc_fhs_t* fhs;

    fhs = get_fhs(fi->fh);

    ret = ftruncate(fhs->clearFH, size);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_ftruncate: ftruncate failed\n");
	perror("ERROR enc_ftruncate");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_utimens(const char* path, const timespec_t ts[2]) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_utimens: buildPath failed\n");
	return ret;
    }
    path = NULL;

    /* don't use utime/utimes since they follow symlinks */
    ret = utimensat(0, fullPath, ts, AT_SYMLINK_NOFOLLOW);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_utimens: utimensat failed\n");
	perror("ERROR enc_utimens");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_create(const char* path, mode_t mode, fuse_file_info_t* fi) {

    int ret;
    int newFlags;
    enc_fhs_t* fhs;
    char fullPath[PATHBUFSIZE];
    char tempPath[PATHBUFSIZE];

#ifdef DEBUG
    fprintf(stderr, "INFO enc_create: function called\n");
#endif

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_create: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = buildTempPath(fullPath, tempPath, sizeof(tempPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_create: buildTempPath failed\n");
	return ret;
    }

    fhs = createFilePair(fullPath, tempPath, fi->flags, mode);
    if(!fhs) {
	fprintf(stderr, "ERROR enc_create: createFilePair failed\n");
	return RETURN_FAILURE;
    }

    ret = closeFilePair(fhs);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_create: closeFilePair failed\n");
	return ret;
    }

    ret = encryptFile(tempPath, fullPath);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_create: encryptFile failed\n");
	return ret;
    }

    ret = removeFile(tempPath);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_create: removeFile failed\n");
	return ret;
    }

    ret = decryptFile(fullPath, tempPath);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_create: decryptFile failed\n");
	return ret;
    }

    newFlags = fi->flags & (~O_CREAT & ~O_EXCL);
    fhs = openFilePair(fullPath, tempPath, newFlags);
    if(!fhs) {
	fprintf(stderr, "ERROR enc_create: openFilePair failed\n");
	return RETURN_FAILURE;
    }

    fhs->dirty = FHS_CLEAN;
    fi->fh = put_fhs(fhs);

#ifdef DEBUG
    fprintf(stderr, "INFO enc_create: returning successfully\n");
#endif

    return RETURN_SUCCESS;

}

static int enc_open(const char* path, fuse_file_info_t* fi) {

    int ret;
    enc_fhs_t* fhs;
    char fullPath[PATHBUFSIZE];
    char tempPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_open: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = buildTempPath(fullPath, tempPath, sizeof(tempPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_open: buildTempPath failed\n");
	return ret;
    }

    ret = decryptFile(fullPath, tempPath);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_open: decryptFile failed\n");
	return ret;
    }

    fhs = openFilePair(fullPath, tempPath, fi->flags);
    if(!fhs) {
	fprintf(stderr, "ERROR enc_open: openFilePair failed\n");
	return RETURN_FAILURE;
    }

    fhs->dirty = FHS_CLEAN;
    fi->fh = put_fhs(fhs);

    return RETURN_SUCCESS;

}

static int enc_read(const char* path, char* buf, size_t size, off_t offset,
		    fuse_file_info_t* fi) {

    (void) path;

    int ret;
    enc_fhs_t* fhs;

    fhs = get_fhs(fi->fh);

    ret = pread(fhs->clearFH, buf, size, offset);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_read: pread failed\n");
	perror("ERROR enc_read");
	ret = -errno;
    }

    return ret;

}

static int enc_write(const char* path, const char* buf, size_t size,
		     off_t offset, fuse_file_info_t* fi) {

    (void) path;

    int ret;
    enc_fhs_t* fhs;

    fhs = get_fhs(fi->fh);
    fhs->dirty = FHS_DIRTY;

    ret = pwrite(fhs->clearFH, buf, size, offset);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_write: pwrite failed\n");
	perror("ERROR enc_write");
	ret = -errno;
    }

    return ret;

}

static int enc_statfs(const char* path, statvfs_t* stbuf) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_statfs: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = statvfs(fullPath, stbuf);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_statfs: statvfs failed\n");
	perror("ERROR enc_statfs");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_flush(const char* path, fuse_file_info_t* fi) {

    /* This is called from every close on an open file, so call the
       close on the underlying filesystem. But since flush may be
       called multiple times for an open file, this must not really
       close the file.  This is important if used on a network
       filesystem like NFS which flush the data/metadata on close() */

    int ret;
    enc_fhs_t* fhs;
    char fullPath[PATHBUFSIZE];
    char tempPath[PATHBUFSIZE];

    if(!path) {
	fprintf(stderr, "ERROR enc_flush: path is NULL");
	return -EINVAL;
    }

    if(!fi) {
	fprintf(stderr, "ERROR enc_flush: fi is NULL");
	return -EINVAL;
    }

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_flush: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = buildTempPath(fullPath, tempPath, sizeof(tempPath));
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_flush: buildTempPath failed\n");
	return ret;
    }

    fhs = get_fhs(fi->fh);

    ret = dup(fhs->clearFH);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_flush: dup(clearFH) failed\n");
	perror("ERROR enc_flush");
	return -errno;
    }
    ret = close(ret);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_flush: close(dup(clearFH)) failed\n");
	perror("ERROR enc_flush");
	return -errno;
    }

    ret = dup(fhs->encFH);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_flush: dup(encFH) failed\n");
	perror("ERROR enc_flush");
	return -errno;
    }
    ret = close(ret);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_flush: close(dup(encFH)) failed\n");
	perror("ERROR enc_flush");
	return -errno;
    }

    if(fhs->dirty == FHS_DIRTY) {

	ret = encryptFile(tempPath, fullPath);
	if(ret < 0) {
	    fprintf(stderr, "ERROR enc_flush: encryptFile failed\n");
	    return ret;
	}

	fhs->dirty = FHS_CLEAN;

    }

    return RETURN_SUCCESS;

}

static int enc_fsync(const char* path, int isdatasync,
		     fuse_file_info_t* fi) {

    int ret;
    enc_fhs_t* fhs;
    char fullPath[PATHBUFSIZE];
    char tempPath[PATHBUFSIZE];

    if(!path) {
	fprintf(stderr, "ERROR enc_fsync: path is NULL");
	return -EINVAL;
    }

    if(!fi) {
	fprintf(stderr, "ERROR enc_fsync: fi is NULL");
	return -EINVAL;
    }

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_fsync: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = buildTempPath(fullPath, tempPath, sizeof(tempPath));
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_fsync: buildTempPath failed\n");
	return ret;
    }

    fhs = get_fhs(fi->fh);

    if(fhs->dirty == FHS_DIRTY) {

	ret = encryptFile(tempPath, fullPath);
	if(ret < 0) {
	    fprintf(stderr, "ERROR enc_fsync: encryptFile failed\n");
	    return ret;
	}

	fhs->dirty = FHS_CLEAN;

    }

    if(isdatasync) {
	ret = fdatasync(fhs->encFH);
    }
    else {
	ret = fsync(fhs->encFH);
    }

    if(ret < 0) {
	fprintf(stderr, "ERROR enc_fsync: fdatasync/fsync failed\n");
	perror("ERROR enc_fsync");
	return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_release(const char* path, fuse_file_info_t* fi) {

    int ret;
    int dirty;
    enc_fhs_t* fhs;
    char fullPath[PATHBUFSIZE];
    char tempPath[PATHBUFSIZE];

#ifdef DEBUG
    fprintf(stderr, "INFO enc_release: function called\n");
#endif

    if(!path) {
	fprintf(stderr, "ERROR enc_release: path is NULL");
	return -EINVAL;
    }

    if(!fi) {
	fprintf(stderr, "ERROR enc_release: fi is NULL");
	return -EINVAL;
    }

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_release: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = buildTempPath(fullPath, tempPath, sizeof(tempPath));
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_release: buildTempPath failed\n");
	return ret;
    }

    fhs = get_fhs(fi->fh);
    dirty = fhs->dirty;

    ret = closeFilePair(fhs);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_release: closeFilePair failed\n");
	return ret;
    }

    if(dirty == FHS_DIRTY) {

	ret = encryptFile(tempPath, fullPath);
	if(ret < 0) {
	    fprintf(stderr, "ERROR enc_release: encryptFile failed\n");
	    return ret;
	}

    }

    ret = removeFile(tempPath);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_release: removeFile failed\n");
	return ret;
    }

#ifdef DEBUG
    fprintf(stderr, "INFO enc_release: returning successfully\n");
#endif

    return RETURN_SUCCESS;

}

static int enc_lock(const char* path, fuse_file_info_t* fi, int cmd,
		    flock_t* lock) {

    (void) path;

    int ret;
    enc_fhs_t* fhs;

    fhs = get_fhs(fi->fh);

    ret = ulockmgr_op(fhs->clearFH, cmd, lock, &fi->lock_owner,
		      sizeof(fi->lock_owner));
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_lock: ulockmgr_op failed\n");
	return ret;
    }

    return ret;

}

static int enc_flock(const char* path, fuse_file_info_t* fi, int op) {

    (void) path;

    int ret;
    enc_fhs_t* fhs;

    fhs = get_fhs(fi->fh);

    ret = flock(fhs->clearFH, op);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_flock: flock failed\n");
	perror("ERROR enc_flock");
        return -errno;
    }

    return RETURN_SUCCESS;

}

/* xattr operations are optional and can safely be left unimplemented */
static int enc_setxattr(const char* path, const char* name, const char* value,
			size_t size, int flags) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_setxattr: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = lsetxattr(fullPath, name, value, size, flags);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_setxattr: lsetxattr failed\n");
	perror("ERROR enc_setxattr");
        return -errno;
    }

    return RETURN_SUCCESS;

}

static int enc_getxattr(const char* path, const char* name, char* value,
			size_t size) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_getxattr: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = lgetxattr(fullPath, name, value, size);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_getxattr: lgetxattr failed\n");
	perror("ERROR enc_getxattr");
        return -errno;
    }

    return ret;

}

static int enc_listxattr(const char* path, char* list, size_t size) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_listxattr: buildPath failed\n");
	return ret;
    }
    path = NULL;

    ret = llistxattr(fullPath, list, size);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_listxattr: llistxattr failed\n");
	perror("ERROR enc_listxattr");
        return -errno;
    }

    return ret;

}

static int enc_removexattr(const char* path, const char* name) {

    int ret;
    char fullPath[PATHBUFSIZE];

    ret = buildPath(path, fullPath, sizeof(fullPath));
    if(ret < 0){
	fprintf(stderr, "ERROR enc_removexattr: buildPath failed\n");
	return RETURN_FAILURE;
    }
    path = NULL;

    ret = lremovexattr(fullPath, name);
    if(ret < 0) {
	fprintf(stderr, "ERROR enc_removexattr: lremovexattr failed\n");
	perror("ERROR enc_removexattr");
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
