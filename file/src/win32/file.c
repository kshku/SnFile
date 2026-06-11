#include "snfile/snfile.h"

#if defined(SN_OS_WINDOWS)

    #include <string.h>
    #include <windows.h>
    #include <sncore/utf.h>

typedef struct SnFileWin32 {
    HANDLE handle;
} SnFileWin32;

    #define HDL(file) (((SnFileWin32 *)(file))->handle)

typedef struct SnDirWin32 {
    HANDLE handle;
    WIN32_FIND_DATAW data;
    bool first;
    char current_name[260 * 4];
} SnDirWin32;

    #define DHDL(dir) (((SnDirWin32 *)(dir))->handle)
    #define DDATA(dir) (((SnDirWin32 *)(dir))->data)
    #define DFIRST(dir) (((SnDirWin32 *)(dir))->first)
    #define DCURR_NAME(dir) (((SnDirWin32 *)(dir))->current_name)

SN_STATIC_ASSERT(sizeof(SnFileWin32) <= sizeof(SnFile), "SnFile size is not large enough!");
SN_STATIC_ASSERT(sizeof(SnDirWin32) <= sizeof(SnDir), "SnDir size is not large enough!");

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

    if (flags & SN_FILE_OPEN_FLAG_TRUNCATE) return TRUNCATE_EXISTING;

    return OPEN_EXISTING;
}

bool sn_file_open(const char *path, SnFileOpenFlag flags, SnFile *file) {
    wchar_t wpath[4096];
    if (sn_utf8_to_utf16(path, wpath, SN_ARRAY_LENGTH(wpath)) == (size_t)-1) return false;

    HDL(file) = CreateFileW(wpath, file_access(flags), FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                            file_creation(flags), FILE_ATTRIBUTE_NORMAL, NULL);

    if (HDL(file) == INVALID_HANDLE_VALUE) return false;

    if (flags & SN_FILE_OPEN_FLAG_APPEND) {
        LARGE_INTEGER zero = {0};
        SetFilePointerEx(HDL(file), zero, NULL, FILE_END);
    }

    return true;
}

void sn_file_close(SnFile *file) {
    if (HDL(file) && HDL(file) != INVALID_HANDLE_VALUE) {
        CloseHandle(HDL(file));
        HDL(file) = INVALID_HANDLE_VALUE;
    }
}

int64_t sn_file_read(SnFile *file, void *buffer, uint64_t size) {
    DWORD read1 = 0;
    DWORD read2 = 0;
    DWORD size2 = 0;
    if (size > 0xffffffff) size2 = size - 0xffffffff;
    DWORD size1 = size - size2;

    if (!ReadFile(HDL(file), buffer, size1, &read1, NULL)) return -1;
    if (size2 && !ReadFile(HDL(file), (void *)(((char *)buffer) + size1), size2, &read2, NULL))
        return -1;

    return (int64_t)read1 + read2;
}

int64_t sn_file_write(SnFile *file, const void *buffer, uint64_t size) {
    DWORD written1 = 0;
    DWORD written2 = 0;
    DWORD size2 = 0;
    if (size > 0xffffffff) size2 = size - 0xffffffff;
    DWORD size1 = size - size2;

    if (!WriteFile(HDL(file), buffer, size1, &written1, NULL)) return -1;
    if (size2 && !WriteFile(HDL(file), (const void *)(((char *)buffer) + size1), size2, &written2, NULL))
        return -1;

    return (int64_t)written1 + written2;
}

bool sn_file_seek(SnFile *file, int64_t offset, SnFileSeekOrigin origin) {
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

uint64_t sn_file_tell(SnFile *file) {
    LARGE_INTEGER zero = {0};
    LARGE_INTEGER pos;
    SetFilePointerEx(HDL(file), zero, &pos, FILE_CURRENT);
    return (uint64_t)pos.QuadPart;
}

bool sn_file_flush(SnFile *file) {
    return FlushFileBuffers(HDL(file));
}

uint64_t sn_file_size(SnFile *file) {
    LARGE_INTEGER size;
    if (!GetFileSizeEx(HDL(file), &size)) return 0;
    return (uint64_t)size.QuadPart;
}

bool sn_dir_open(const char *path, SnDir *dir) {
    wchar_t wpath[4096];
    size_t written = sn_utf8_to_utf16(path, wpath, SN_ARRAY_LENGTH(wpath) - 2);
    if (written == (size_t)-1) return false;

    wpath[written] = L'\\';
    wpath[written + 1] = L'*';
    wpath[written + 2] = 0;

    DHDL(dir) = FindFirstFileW(wpath, &DDATA(dir));
    if (DHDL(dir) == INVALID_HANDLE_VALUE) return false;

    DFIRST(dir) = true;

    return true;
}

bool sn_dir_read(SnDir *dir, SnDirEntry *entry) {
    WIN32_FIND_DATAW *data = &DDATA(dir);

    if (DFIRST(dir)) DFIRST(dir) = false;
    else if (!FindNextFileW(DHDL(dir), data)) return false;

    if (sn_utf16_to_utf8(data->cFileName, DCURR_NAME(dir), 260 * 4) == (size_t)-1)
        return false;

    *entry = (SnDirEntry){
        .name = DCURR_NAME(dir),
        .is_directory = (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0,
        .is_symlink = (data->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0,
    };
    entry->is_file = !entry->is_directory;

    return true;
}

void sn_dir_close(SnDir *dir) {
    if (DHDL(dir) && DHDL(dir) != INVALID_HANDLE_VALUE) {
        FindClose(DHDL(dir));
        DHDL(dir) = INVALID_HANDLE_VALUE;
    }
}

bool sn_path_exists(const char *path) {
    wchar_t wpath[4096];
    if (sn_utf8_to_utf16(path, wpath, SN_ARRAY_LENGTH(wpath)) == (size_t)-1) return false;
    return GetFileAttributesW(wpath) != INVALID_FILE_ATTRIBUTES;
}

bool sn_path_is_file(const char *path) {
    wchar_t wpath[4096];
    if (sn_utf8_to_utf16(path, wpath, SN_ARRAY_LENGTH(wpath)) == (size_t)-1) return false;
    DWORD attr = GetFileAttributesW(wpath);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool sn_path_is_directory(const char *path) {
    wchar_t wpath[4096];
    if (sn_utf8_to_utf16(path, wpath, SN_ARRAY_LENGTH(wpath)) == (size_t)-1) return false;
    DWORD attr = GetFileAttributesW(wpath);
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool sn_file_delete(const char *path) {
    wchar_t wpath[4096];
    if (sn_utf8_to_utf16(path, wpath, SN_ARRAY_LENGTH(wpath)) == (size_t)-1) return false;
    return DeleteFileW(wpath);
}

bool sn_dir_create(const char *path, bool recursive) {
    wchar_t wpath[4096];
    if (sn_utf8_to_utf16(path, wpath, SN_ARRAY_LENGTH(wpath)) == (size_t)-1) return false;

    if (!recursive) return CreateDirectoryW(wpath, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;

    for (size_t i = 0; wpath[i]; ++i) {
        if (wpath[i] == L'\\' || wpath[i] == L'/') {
            wchar_t saved = wpath[i];
            wpath[i] = 0;
            if (!CreateDirectoryW(wpath, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
                return false;
            wpath[i] = L'\\';
        }
    }

    return CreateDirectoryW(wpath, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool sn_dir_delete(const char *path) {
    wchar_t wpath[4096];
    if (sn_utf8_to_utf16(path, wpath, SN_ARRAY_LENGTH(wpath)) == (size_t)-1) return false;
    return RemoveDirectoryW(wpath);
}

bool sn_file_copy(const char *src, const char *dst, bool overwrite) {
    wchar_t wsrc[4096];
    if (sn_utf8_to_utf16(src, wsrc, SN_ARRAY_LENGTH(wsrc)) == (size_t)-1) return false;

    wchar_t wdst[4096];
    if (sn_utf8_to_utf16(dst, wdst, SN_ARRAY_LENGTH(wdst)) == (size_t)-1) return false;

    return CopyFileW(wsrc, wdst, !overwrite);
}

bool sn_file_move(const char *src, const char *dst, bool overwrite) {
    wchar_t wsrc[4096];
    if (sn_utf8_to_utf16(src, wsrc, SN_ARRAY_LENGTH(wsrc)) == (size_t)-1) return false;

    wchar_t wdst[4096];
    if (sn_utf8_to_utf16(dst, wdst, SN_ARRAY_LENGTH(wdst)) == (size_t)-1) return false;

    return MoveFileExW(wsrc, wdst, (overwrite ? MOVEFILE_REPLACE_EXISTING : 0));
}

bool sn_file_stat(const char *path, SnFileInfo *info) {
    wchar_t wpath[4096];
    if (sn_utf8_to_utf16(path, wpath, SN_ARRAY_LENGTH(wpath)) == (size_t)-1) return false;

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExW(wpath, GetFileExInfoStandard, &data)) return false;

    ULARGE_INTEGER size;
    size.HighPart = data.nFileSizeHigh;
    size.LowPart = data.nFileSizeLow;

    *info = (SnFileInfo){
        .size = size.QuadPart,

        .is_directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0,
        .is_symlink = (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0,

        .accessed_time
        = (((uint64_t)data.ftLastAccessTime.dwHighDateTime) << 32) | data.ftLastAccessTime.dwLowDateTime,

        .modified_time
        = (((uint64_t)data.ftLastWriteTime.dwHighDateTime) << 32) | data.ftLastWriteTime.dwLowDateTime,

        .change_time = (((uint64_t)data.ftCreationTime.dwHighDateTime) << 32) | data.ftCreationTime.dwLowDateTime};
    info->is_file = !info->is_directory;

    return true;
}

#endif
