#include "snfile/snfile.h"

#if defined(SN_OS_WINDOWS)

#include <windows.h>
#include <string.h>
 
typedef struct snFileWin32 {
    HANDLE handle;
} snFileWin32;

#define HDL(file) (((snFileWin32 *)(file))->handle)

typedef struct snDirWin32 {
    HANDLE handle;
    WIN32_FIND_DATAA data;
    bool first;
} snDirWin32;

#define DHDL(dir) (((snDirWin32 *)(dir))->handle)
#define DDATA(dir) (((snDirWin32 *)(dir))->data)
#define DFIRST(dir) (((snDirWin32 *)(dir))->first)

SN_STATIC_ASSERT(sizeof(snFileWin32) <= sizeof(snFile), "snFile size is not large enough!");
SN_STATIC_ASSERT(sizeof(snDirWin32) <= sizeof(snDir), "snDir size is not large enough!");

static DWORD file_access(int flags) {
    DWORD access = 0;
    if (flags & SN_FILE_OPEN_FLAG_READ) access |= GENERIC_READ;
    if (flags & SN_FILE_OPEN_FLAG_WRITE) access |= GENERIC_WRITE;
    return access;
}

static DWORD file_creation(int flags) {
    if ((flags & SN_FILE_OPEN_FLAG_CREATE) && (flags & SN_FILE_OPEN_FLAG_TRUNCATE))
        return CREATE_ALWAYS;

    if (flags & SN_FILE_OPEN_FLAG_CREATE) return OPEN_ALWAYS;

    if (flags & SN_FILE_OPEN_FLAG_TRUNCATE)
        return TRUNCATE_EXISTING;

    return OPEN_EXISTING;
}

bool sn_file_open(const char *path, snFileOpenFlag flags, snFile *file) {
    HDL(file) = CreateFileA(path, file_access(flags), FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, file_creation(flags), FILE_ATTRIBUTE_NORMAL, NULL);

    if (HDL(file) == INVALID_HANDLE_VALUE) return false;

    if (flags & SN_FILE_OPEN_FLAG_APPEND) {
        LARGE_INTEGER zero = {0};
        SetFilePointerEx(HDL(file), zero, NULL, FILE_END);
    }

    return true;
}

void sn_file_close(snFile *file) {
    if (HDL(file) && HDL(file) != INVALID_HANDLE_VALUE) {
        CloseHandle(HDL(file));
        HDL(file) = INVALID_HANDLE_VALUE;
    }
}

int64_t sn_file_read(snFile *file, void *buffer, uint64_t size) {
    DWORD read1 = 0;
    DWORD read2 = 0;
    DWORD size2 = 0;
    if (size > 0xffffffff) size2 = size - 0xffffffff;
    DWORD size1 = size - size2;

    if (!ReadFile(HDL(file), buffer, size1, &read1, NULL)) return -1;
    if (size2 && !ReadFile(HDL(file), buffer, size2, &read2, NULL)) return -1;

    return (int64_t)read1 + read2;
}

int64_t sn_file_write(snFile *file, const void *buffer, uint64_t size) {
    DWORD written1 = 0;
    DWORD written2 = 0;
    DWORD size2 = 0;
    if (size > 0xffffffff) size2 = size - 0xffffffff;
    DWORD size1 = size - size2;

    if (!WriteFile(HDL(file), buffer, size1, &written1, NULL)) return -1;
    if (size2 && !WriteFile(HDL(file), buffer, size2, &written2, NULL)) return -1;

    return (int64_t)written1 + written2;
}

bool sn_file_seek(snFile *file, int64_t offset, snFileSeekOrigin origin) {
    DWORD move = 0;
    switch (origin) {
        case SN_FILE_SEEK_ORIGIN_BEGIN:
            move = FILE_BEGIN;
            break;
        case SN_FILE_SEEK_ORIGIN_END:
            move = FILE_END;
            break;
        case SN_FILE_SEEK_ORIGIN_CURRENT:
        default:
            move = FILE_CURRENT;
            break;
    }

    LARGE_INTEGER off;
    off.QuadPart = offset;

    return SetFilePointerEx(HDL(file), off, NULL, move);
}

uint64_t sn_file_tell(snFile *file) {
    LARGE_INTEGER zero = {0};
    LARGE_INTEGER pos;
    SetFilePointerEx(HDL(file), zero, &pos, FILE_CURRENT);
    return (uint64_t)pos.QuadPart;
}

bool sn_file_flush(snFile *file) {
    return FlushFileBuffers(HDL(file));
}

uint64_t sn_file_size(snFile *file) {
    LARGE_INTEGER size;
    if (!GetFileSizeEx(HDL(file), &size)) return 0;
    return (uint64_t)size.QuadPart;
}

bool sn_dir_open(const char* path, snDir *dir) {
    char pattern[MAX_PATH];
    snprintf(pattern, MAX_PATH, "%s\\*", path);
    DHDL(dir) = FindFirstFileA(pattern, &DDATA(dir));
    if (DHDL(dir) == INVALID_HANDLE_VALUE) return false;

    DFIRST(dir) = true;

    return true;
}

bool sn_dir_read(snDir* dir, snDirEntry* entry) {
    WIN32_FIND_DATAA *data = &DDATA(dir);

    if (DFIRST(dir)) DFIRST(dir) = false;
    else if (!FindNextFileA(DHDL(dir), data)) return false;

    *entry = (snDirEntry) {
        .name = data->cFileName;
        .is_directory = (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        .is_file = !entry->is_dir;
        .is_symlink = (data->dwFileAttributes & FILE_ATTRIBUTE_REPRSE_POINT) != 0;
    };

    return true;
}

void sn_dir_close(snDir* dir) {
    if (DHDL(dir) && DHDL(dir) != INVALID_HANDLE_VALUE) {
        FindClose(DHDL(dir));
        DHDL(dir) = INVALID_HANDLE_VALUE;
    }
}

bool sn_path_exists(const char* path) {
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

bool sn_path_is_file(const char* path) {
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool sn_path_is_directory(const char* path) {
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool sn_file_delete(const char* path) {
    return DeleteFileA(path);
}

bool sn_dir_create(const char* path, bool recursive) {
    if (!recursive) return CreateDirectoryA(path, NULL);

    char buffer[1024] = {0};
    for (size_t i = 0; path[i]; ++i) {
        SN_ASSERT(i < SN_ARRAY_LENGTH(buffer));
        buffer[i] = path[i];
        if (path[i] == '\\' || path[i] == '/') {
            buffer[i] = 0;
            if (!CreateDirectoryA(buffer, NULL)) return false;
            buffer[i] = SN_PATH_SEPARATOR;
        }
    }

    return CreateDirectoryA(buffer, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool sn_dir_delete(const char* path) {
    return RemoveDirectoryA(path);
}

bool sn_file_copy(const char* src, const char* dst, bool overwrite) {
    return CopyFileA(src, dst, !overwrite);
}

bool sn_file_move(const char* src, const char* dst, bool overwrite) {
    return MoveFileExA(src, dst, (overwrite ? MOVEFILE_REPLACE_EXISTING : 0));
}

bool sn_file_stat(const char* path, snFileInfo* info) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data)) return false;

    ULARGE_INTEGER size;
    size.HighPart = data.nFileSizeHigh;
    size.LowPart = data.nFileSizeLow;

    *info = (snFileInfo) {
        .size = size.QuadPart,

        .is_directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0,
        .is_file = !info->is_directory,
        .is_symlink = (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0,

        .accessed_time = (((uint64_t)data.ftLastAccessTime.dwHighDateTime) << 32)
            | data.ftLastAccessTime.dwLowDateTime,

        .modified_time = (((uint64_t)data.ftLastWriteTime.dwHighDateTime) << 32)
            | data.ftLastWriteTime.dwLowDateTime,

        .change_time = (((uint64_t)data.ftCreationTime.dwHighDateTime) << 32)
            | data.ftCreationTime.dwLowDateTime
    };

    return true;
}

#endif
