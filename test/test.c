#include "snfile/snfile.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define TEST_DIR        "snfile_test_dir"
#define TEST_SUBDIR     "snfile_test_dir/sub"
#define TEST_FILE       "snfile_test_dir/test.txt"
#define TEST_FILE_COPY  "snfile_test_dir/test_copy.txt"
#define TEST_FILE_MOVE  "snfile_test_dir/test_moved.txt"

static void test_path_utils(void) {
    char buffer[256];

    assert(sn_path_join(buffer, sizeof(buffer), "a/b", "c/d"));
    sn_path_normalize(buffer);
    assert(strcmp(buffer, "a/b/c/d") == 0);

    assert(strcmp(sn_path_filename("/a/b/c.txt"), "c.txt") == 0);
    assert(strcmp(sn_path_extension("/a/b/c.txt"), "txt") == 0);
    assert(sn_path_extension("/a/b/c") == NULL);

    char p2[256] = "a/./b/../c//d";
    sn_path_normalize(p2);
    assert(strcmp(p2, "a/c//d") == 0);

    printf("[OK] path utils\n");
}

static void test_directory_ops(void) {
    assert(sn_dir_create(TEST_DIR, true));
    assert(sn_dir_create(TEST_SUBDIR, true));

    assert(sn_path_exists(TEST_DIR));
    assert(sn_path_is_directory(TEST_DIR));
    assert(sn_path_is_directory(TEST_SUBDIR));

    snDir dir;
    assert(sn_dir_open(TEST_DIR, &dir));

    snDirEntry entry;
    int seen = 0;
    while (sn_dir_read(&dir, &entry)) {
        if (strcmp(entry.name, ".") == 0) continue;
        if (strcmp(entry.name, "..") == 0) continue;
        ++seen;
    }

    sn_dir_close(&dir);
    assert(seen >= 1);

    printf("[OK] directory ops\n");
}

static void test_file_io(void) {
    const char *msg = "Hello from SnFile!\n";
    char buffer[128];

    snFile file;
    assert(sn_file_open(
        TEST_FILE,
        SN_FILE_OPEN_FLAG_CREATE |
        SN_FILE_OPEN_FLAG_WRITE |
        SN_FILE_OPEN_FLAG_TRUNCATE,
        &file
    ));

    assert(sn_file_write(&file, msg, strlen(msg)) == (int64_t)strlen(msg));
    assert(sn_file_flush(&file));
    sn_file_close(&file);

    assert(sn_file_open(TEST_FILE, SN_FILE_OPEN_FLAG_READ, &file));
    int64_t r = sn_file_read(&file, buffer, sizeof(buffer));
    assert(r > 0);
    buffer[r] = 0;
    assert(strcmp(buffer, msg) == 0);
    sn_file_close(&file);

    printf("[OK] file read/write\n");
}

static void test_seek_and_size(void) {
    snFile file;
    assert(sn_file_open(TEST_FILE, SN_FILE_OPEN_FLAG_READ, &file));

    uint64_t size = sn_file_size(&file);
    assert(size > 0);

    assert(sn_file_seek(&file, 0, SN_FILE_SEEK_ORIGIN_END));
    assert(sn_file_tell(&file) == size);

    assert(sn_file_seek(&file, -1, SN_FILE_SEEK_ORIGIN_END));
    assert(sn_file_tell(&file) == size - 1);

    sn_file_close(&file);

    printf("[OK] seek / tell / size\n");
}

static void test_copy_move_stat(void) {
    snFileInfo info;

    assert(sn_file_copy(TEST_FILE, TEST_FILE_COPY, true));
    assert(sn_path_exists(TEST_FILE_COPY));
    assert(sn_file_stat(TEST_FILE_COPY, &info));
    assert(info.is_file);
    assert(info.size > 0);

    assert(sn_file_move(TEST_FILE_COPY, TEST_FILE_MOVE, true));
    assert(!sn_path_exists(TEST_FILE_COPY));
    assert(sn_path_exists(TEST_FILE_MOVE));

    printf("[OK] copy / move / stat\n");
}

static void test_cleanup(void) {
    assert(sn_file_delete(TEST_FILE));
    assert(sn_file_delete(TEST_FILE_MOVE));
    assert(sn_dir_delete(TEST_SUBDIR));
    assert(sn_dir_delete(TEST_DIR));

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

