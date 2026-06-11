#include "snfile/snfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_ASSERT(x)                                                     \
    do {                                                                   \
        if (!(x)) {                                                        \
            fprintf(stderr, "FAIL [%s:%d]: %s\n", __FILE__, __LINE__, #x); \
            abort();                                                       \
        }                                                                  \
    } while (0)

#define TEST_DIR "snfile_test_dir"
#define TEST_SUBDIR "snfile_test_dir/sub"
#define TEST_FILE "snfile_test_dir/test.txt"
#define TEST_FILE_COPY "snfile_test_dir/test_copy.txt"
#define TEST_FILE_MOVE "snfile_test_dir/test_moved.txt"

static void test_path_utils(void) {
    char buffer[256];

    TEST_ASSERT(sn_path_join(buffer, sizeof(buffer), "a/b", "c/d"));
    sn_path_normalize(buffer);
    TEST_ASSERT(strcmp(buffer, "a" SN_PATH_SEPARATOR_STR "b" SN_PATH_SEPARATOR_STR "c" SN_PATH_SEPARATOR_STR "d") == 0);

    TEST_ASSERT(strcmp(sn_path_filename("/a/b/c.txt"), "c.txt") == 0);
    TEST_ASSERT(strcmp(sn_path_extension("/a/b/c.txt"), "txt") == 0);
    TEST_ASSERT(sn_path_extension("/a/b/c") == NULL);

    char p2[256] = "a/.//b/../c/d";
    sn_path_normalize(p2);
    TEST_ASSERT(strcmp(p2, "a" SN_PATH_SEPARATOR_STR SN_PATH_SEPARATOR_STR "c" SN_PATH_SEPARATOR_STR "d") == 0);

    printf("[OK] path utils\n");
}

static void test_directory_ops(void) {
    TEST_ASSERT(sn_dir_create(TEST_DIR, true));
    TEST_ASSERT(sn_dir_create(TEST_SUBDIR, true));

    TEST_ASSERT(sn_path_exists(TEST_DIR));
    TEST_ASSERT(sn_path_is_directory(TEST_DIR));
    TEST_ASSERT(sn_path_is_directory(TEST_SUBDIR));

    SnDir dir;
    TEST_ASSERT(sn_dir_open(TEST_DIR, &dir));

    SnDirEntry entry;
    int seen = 0;
    while (sn_dir_read(&dir, &entry)) {
        if (strcmp(entry.name, ".") == 0) continue;
        if (strcmp(entry.name, "..") == 0) continue;
        ++seen;
    }

    sn_dir_close(&dir);
    TEST_ASSERT(seen >= 1);

    printf("[OK] directory ops\n");
}

static void test_file_io(void) {
    const char *msg = "Hello from SnFile!\n";
    char buffer[128];

    SnFile file;
    TEST_ASSERT(sn_file_open(
        TEST_FILE, SN_FILE_OPEN_FLAG_CREATE | SN_FILE_OPEN_FLAG_WRITE | SN_FILE_OPEN_FLAG_TRUNCATE, &file));

    TEST_ASSERT(sn_file_write(&file, msg, strlen(msg)) == (int64_t)strlen(msg));
    TEST_ASSERT(sn_file_flush(&file));
    sn_file_close(&file);

    TEST_ASSERT(sn_file_open(TEST_FILE, SN_FILE_OPEN_FLAG_READ, &file));
    int64_t r = sn_file_read(&file, buffer, sizeof(buffer));
    TEST_ASSERT(r > 0);
    buffer[r] = 0;
    TEST_ASSERT(strcmp(buffer, msg) == 0);
    sn_file_close(&file);

    printf("[OK] file read/write\n");
}

static void test_seek_and_size(void) {
    SnFile file;
    TEST_ASSERT(sn_file_open(TEST_FILE, SN_FILE_OPEN_FLAG_READ, &file));

    uint64_t size = sn_file_size(&file);
    TEST_ASSERT(size > 0);

    TEST_ASSERT(sn_file_seek(&file, 0, SN_FILE_SEEK_ORIGIN_END));
    TEST_ASSERT(sn_file_tell(&file) == size);

    TEST_ASSERT(sn_file_seek(&file, -1, SN_FILE_SEEK_ORIGIN_END));
    TEST_ASSERT(sn_file_tell(&file) == size - 1);

    sn_file_close(&file);

    printf("[OK] seek / tell / size\n");
}

static void test_copy_move_stat(void) {
    SnFileInfo info;

    TEST_ASSERT(sn_file_copy(TEST_FILE, TEST_FILE_COPY, true));
    TEST_ASSERT(sn_path_exists(TEST_FILE_COPY));
    TEST_ASSERT(sn_file_stat(TEST_FILE_COPY, &info));
    TEST_ASSERT(info.is_file);
    TEST_ASSERT(info.size > 0);

    TEST_ASSERT(sn_file_move(TEST_FILE_COPY, TEST_FILE_MOVE, true));
    TEST_ASSERT(!sn_path_exists(TEST_FILE_COPY));
    TEST_ASSERT(sn_path_exists(TEST_FILE_MOVE));

    printf("[OK] copy / move / stat\n");
}

static void test_cleanup(void) {
    TEST_ASSERT(sn_file_delete(TEST_FILE));
    TEST_ASSERT(sn_file_delete(TEST_FILE_MOVE));
    TEST_ASSERT(sn_dir_delete(TEST_SUBDIR));
    TEST_ASSERT(sn_dir_delete(TEST_DIR));

    printf("[OK] cleanup\n");
}

int main(void) {
    printf("==== SnFile Test ====\n");

    test_path_utils();
    test_directory_ops();
    test_file_io();
    test_seek_and_size();
    test_copy_move_stat();
    test_cleanup();

    printf("==== ALL TESTS PASSED ====\n");
    return 0;
}

