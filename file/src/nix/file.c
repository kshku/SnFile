#define _GNU_SOURCE
#include "snfile/snfile.h"

#if defined(SN_OS_LINUX) || defined(SN_OS_MAC)

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct snFilePosix {
    int fd;
} snFilePosix;

#define FD(file) (((snFilePosix *)(file))->fd)

typedef struct snDirPosix {
    DIR *dir;
} snDirPosix;

#define DIRECTORY(dir) (((snDirPosix *)(dir))->dir)

SN_STATIC_ASSERT(sizeof(snFilePosix) <= sizeof(snFile), "snFile size is not large enough!");
SN_STATIC_ASSERT(sizeof(snDirPosix) <= sizeof(snDir), "snDir size is not large enough!");

bool sn_file_open(const char *path, int flags, snFile *file) {
    int open_flags = 0;

    if (flags & SN_FILE_OPEN_FLAG_READ) open_flags |= O_RDONLY;
    if (flags & SN_FILE_OPEN_FLAG_WRITE) open_flags |= O_WRONLY;
    if ((flags & (SN_FILE_OPEN_FLAG_READ | SN_FILE_OPEN_FLAG_WRITE)) == 
        (SN_FILE_OPEN_FLAG_READ | SN_FILE_OPEN_FLAG_WRITE))
        open_flags |= O_RDWR;

    if (flags & SN_FILE_OPEN_FLAG_CREATE) open_flags |= O_CREAT;
    if (flags & SN_FILE_OPEN_FLAG_TRUNCATE) open_flags |= O_TRUNC;
    if (flags & SN_FILE_OPEN_FLAG_APPEND) open_flags |= O_APPEND;

    FD(file) = open(path, open_flags, 0644);
    if(FD(file) < 0) return false;

    return true;
}

void sn_file_close(snFile *file) {
    SN_ASSERT(close(FD(file)) == 0);
    FD(file) = -1;
}

int64_t sn_file_read(snFile *file, void *buffer, uint64_t size) {
    return (int64_t)read(FD(file), buffer, size);
}

int64_t sn_file_write(snFile *file, const void *buffer, uint64_t size) {
    return (int64_t)write(FD(file), buffer, size);
}

bool sn_file_seek(snFile *file, int64_t offset, snFileSeekOrigin origin) {
    int whence = 0;
    switch (origin) {
        case SN_FILE_SEEK_ORIGIN_BEGIN:
            whence = SEEK_SET;
            break;
        case SN_FILE_SEEK_ORIGIN_END:
            whence = SEEK_END;
            break;
        case SN_FILE_SEEK_ORIGIN_CURRENT:
        default:
            whence = SEEK_CUR;
            break;
    }

    return lseek(FD(file), offset, whence) >= 0;
}

uint64_t sn_file_tell(snFile *file) {
    return (uint64_t)lseek(FD(file), 0, SEEK_CUR);
}

bool sn_file_flush(snFile *file) {
    return fsync(FD(file)) == 0;
}

uint64_t sn_file_size(snFile *file) {
    struct stat st;
    if (fstat(FD(file), &st) != 0) return 0;
    return st.st_size;
}

bool sn_dir_open(const char* path, snDir *dir) {
    DIRECTORY(dir) = opendir(path);
    if (!DIRECTORY(dir)) return false;
    return true;
}

bool sn_dir_read(snDir* dir, snDirEntry* entry) {
    struct dirent *dirent = readdir(DIRECTORY(dir));

    if (!dirent) return false;

    *entry = (snDirEntry){
        .name = dirent->d_name,
        .is_file = dirent->d_type == DT_REG,
        .is_directory = dirent->d_type == DT_DIR,
        .is_symlink = dirent->d_type == DT_LNK
    };

    return true;
}

void sn_dir_close(snDir* dir) {
    SN_ASSERT(closedir(DIRECTORY(dir)) == 0);
}

bool sn_path_exists(const char* path) {
    struct stat st;
    SN_UNUSED(st);
    return stat(path, &st) == 0;
}

bool sn_path_is_file(const char* path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

bool sn_path_is_directory(const char* path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

bool sn_file_delete(const char* path) {
    return unlink(path) == 0;
}

bool sn_dir_create(const char* path, bool recursive) {
    if (!recursive) return mkdir(path, 0755) == 0 || errno == EEXIST;

    // Hopefully large enough buffer
    char buffer[1024] = {0};
    for (size_t i = 0; path[i]; ++i) {
        SN_ASSERT(i < SN_ARRAY_LENGTH(buffer));
        buffer[i] = path[i];
        if (path[i] == '\\' || path[i] == '/') {
            buffer[i] = 0;
            if (mkdir(buffer, 0755) != 0 && errno != EEXIST) return false;
            buffer[i] = SN_PATH_SEPARATOR;
        }
    }
    
    return mkdir(buffer, 0755) == 0 || errno == EEXIST;
}

bool sn_dir_delete(const char* path) {
    return rmdir(path) == 0;
}

bool sn_file_copy(const char* src, const char* dst, bool overwrite) {
    if (!overwrite && sn_path_exists(dst)) return false;

    snFile srcf, dstf;

    if (!sn_file_open(src, SN_FILE_OPEN_FLAG_READ, &srcf)) return false;

    int flags = SN_FILE_OPEN_FLAG_CREATE | SN_FILE_OPEN_FLAG_WRITE;
    if (!sn_file_open(dst, flags, &dstf)) return false;

    char buffer[4096];
    int64_t ret;
    while ((ret = sn_file_read(&srcf, buffer, SN_ARRAY_LENGTH(buffer))) > 0)
        sn_file_write(&dstf, buffer, ret);

    sn_file_close(&srcf);
    sn_file_close(&dstf);
    return true;
}

bool sn_file_move(const char* src, const char* dst, bool overwrite) {
    if (!overwrite && sn_path_exists(dst)) return false;
    return rename(src, dst) == 0;
}

bool sn_file_stat(const char* path, snFileInfo* info) {
    struct stat st;
    if (stat(path, &st) != 0)
        return false;

    *info = (snFileInfo){
        .size = st.st_size,

        .modified_time = st.st_mtime,
        .accessed_time = st.st_atime,
        .change_time =  st.st_ctime,

        .is_file = S_ISREG(st.st_mode),
        .is_directory = S_ISDIR(st.st_mode),
        .is_symlink = S_ISLNK(st.st_mode)
    };

    return true;
}

#endif
