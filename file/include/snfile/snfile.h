#pragma once

#include "snfile/defines.h"

#if defined(SN_OS_WINDOWS)
    #define SN_PATH_SEPARATOR '\\'
#else
    #define SN_PATH_SEPARATOR '/'
#endif

/**
 * @struct snFile
 * @brief Opaque file handle.
 */
typedef struct snFile {
    alignas(max_align_t) char buffer[8];
}snFile;

/**
 * @struct snDir
 * @brief Opaque dir handle.
 */
typedef struct snDir {
#ifdef SN_OS_WINDOWS
    alignas(max_align_t) char buffer[512];
#else
    alignas(max_align_t) char buffer[8];
#endif
} snDir;

/**
 * @brief File open flags.
 */
typedef enum snFileOpenFlag {
    SN_FILE_OPEN_FLAG_READ = SN_BIT_FLAG(0),
    SN_FILE_OPEN_FLAG_WRITE = SN_BIT_FLAG(1),
    SN_FILE_OPEN_FLAG_APPEND = SN_BIT_FLAG(2),
    SN_FILE_OPEN_FLAG_CREATE = SN_BIT_FLAG(3),
    SN_FILE_OPEN_FLAG_TRUNCATE = SN_BIT_FLAG(4),
    SN_FILE_OPEN_FLAG_BINARY = SN_BIT_FLAG(5), /**< Windows only, ignored in POSIX */
} snFileOpenFlag; 

/**
 * @brief File seeks.
 */
typedef enum snFileSeekOrigin {
    SN_FILE_SEEK_ORIGIN_BEGIN,
    SN_FILE_SEEK_ORIGIN_CURRENT,
    SN_FILE_SEEK_ORIGIN_END
} snFileSeekOrigin;

/**
 * @struct snFileInfo
 * @brief File info.
 */
typedef struct snFileInfo {
    uint64_t size;
    uint64_t change_time;
    uint64_t modified_time;
    uint64_t accessed_time;

    bool is_file;
    bool is_directory;
    bool is_symlink;
} snFileInfo;

/**
 * @struct snDirEntry
 * @brief The directory entry.
 */
typedef struct snDirEntry {
    const char *name;
    bool is_file;
    bool is_directory;
    bool is_symlink;
} snDirEntry;

/**
 * @brief Open a file.
 *
 * @param path The path to file.
 * @param flags Flags for opening.
 * @param allocator Allocator hooks.
 *
 * @return Returns true on success, false otherwise.
 */
SN_API bool sn_file_open(const char *path, int flags, snFile *file);

/**
 * @brief Closes the opened file.
 *
 * @param file File to close.
 */
SN_API void sn_file_close(snFile *file);

/**
 * @bief Read from file to buffer.
 *
 * Returns 0 if EOF, on error returns negetive number.
 *
 * @param file The file to read.
 * @param buffer The buffer to write.
 * @param size Size of the buffer (amount to read).
 *
 * @return Returns number of bytes actually read.
 *
 * @note Returns number of bytes read.
 */
SN_API int64_t sn_file_read(snFile *file, void *buffer, uint64_t size);

/**
 * @bief Write to file from buffer.
 *
 * On error returns negetive number.
 *
 * @param file The file to write to.
 * @param buffer The buffer to read.
 * @param size Size of the buffer (amount to write).
 *
 * @return Returns number of bytes actually written.
 *
 * @note Returns number of bytes written.
 */
SN_API int64_t sn_file_write(snFile *file, const void *buffer, uint64_t size);

/**
 * @brief Seek file.
 *
 * @param file The file.
 * @param offset Offset to seek.
 * @param origin The seek origin.
 */
SN_API bool sn_file_seek(snFile *file, int64_t offset, snFileSeekOrigin origin);

/**
 * @brief Get the current offset.
 *
 * @param file The file.
 *
 * @return Returns current offset.
 */
SN_API uint64_t sn_file_tell(snFile *file);

/**
 * @brief Flush the file.
 * 
 * @param file The file to flush.
 * 
 * @return Returns true on success, false otherwise.
 */
SN_API bool sn_file_flush(snFile *file);

/**
 * @brief Get file size.
 *
 * @param file The file to get size of.
 *
 * @return Size of the file.
 */
SN_API uint64_t sn_file_size(snFile *file);

/**
 * @brief Open a directory.
 *
 * @param path Path to directory.
 * @param allocator The allocator.
 *
 * @return Returns true on success, false otherwise.
 */
SN_API bool sn_dir_open(const char* path, snDir *dir);

/**
 * @brief Read the directory.
 *
 * @note Name in entry will be only valid until next call to sn_dir_read.
 *
 * @param dir The directory to read.
 * @param entry The entry to wirte to.
 *
 * @return Returns false when no more entries are there.
 */
SN_API bool sn_dir_read(snDir* dir, snDirEntry* entry);

/**
 * @brief Close the opened directory.
 *
 * @param dir The directory to close.
 */
SN_API void sn_dir_close(snDir* dir);

/**
 * @brief Join two paths.
 *
 * @param dst The buffer to write joined path.
 * @param dst_size The size to write.
 * @param a The first part of path to join.
 * @param b The second part of path to join.
 *
 * @return Returns true on success, false if size was not enough.
 */
SN_API bool sn_path_join(char* dst, size_t dst_size, const char* a, const char* b);

/**
 * @brief Normalize the path.
 *
 * Resolves `.` and `..`, and Fixes separators.
 *
 * @note Modifies the path passed as argument.
 * @note If '..' is found before any separator, it will be just discarded.
 *
 * @param path Pointer to path buffer.
 */
SN_API void sn_path_normalize(char* path);

/**
 * @brief Get the pointer to file name.
 *
 * @note Returns whatever is there after the last separator
 *
 * @param path The path.
 *
 * @param Pointer to file name in the buffer.
 */
SN_API const char* sn_path_filename(const char* path);

/**
 * @brief Get the pointer to file extension.
 *
 * @note Returns NULL if no extension found.
 *
 * @param path The path.
 *
 * @param Pointer to file extension in the buffer.
 */
SN_API const char* sn_path_extension(const char* path);

/**
 * @brief Check whether path exists.
 *
 * @param path The path.
 *
 * @return Returns true if path exists, false otherwise.
 */
SN_API bool sn_path_exists(const char* path);

/**
 * @brief Check whether path is a file.
 *
 * @param path The path.
 *
 * @return Returns true if path is a file, false otherwise.
 */
SN_API bool sn_path_is_file(const char* path);

/**
 * @brief Check whether path is a directory.
 *
 * @param path The path.
 *
 * @return Returns true if path is a directory, false otherwise.
 */
SN_API bool sn_path_is_directory(const char* path);

/**
 * @brief Delete a file.
 *
 * @param path The path.
 *
 * @return Returns true on success, false otherwise.
 */
SN_API bool sn_file_delete(const char* path);

/**
 * @brief Create a directory.
 *
 * @param path The path.
 * @param recursive Create recursively.
 *
 * @return Returns true on success, false otherwise.
 *
 * @note Returns true if directory already exists.
 */
SN_API bool sn_dir_create(const char* path, bool recursive);

/**
 * @brief Delete an empty directory.
 *
 * @param path The path.
 *
 * @return Returns true on success, false otherwise.
 */
SN_API bool sn_dir_delete(const char* path);

/**
 * @brief Copy file.
 *
 * @param src Path to copy from.
 * @param dst Path to copy to.
 * @param overwrite Overwrite if exists
 *
 * @return Returns true on success, false otherwise.
 */
SN_API bool sn_file_copy(const char* src, const char* dst, bool overwrite);

/**
 * @brief Move file.
 *
 * @param src Path to copy from.
 * @param dst Path to copy to.
 * @param overwrite Overwrite if exists
 *
 * @return Returns true on success, false otherwise.
 */
SN_API bool sn_file_move(const char* src, const char* dst, bool overwrite);

/**
 * @brief Get file info.
 *
 * @param path The file path.
 * @param info The info to write to.
 *
 * @return Returns true on success, false otherwise.
 */
SN_API bool sn_file_stat(const char* path, snFileInfo* info);

