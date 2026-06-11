# SnFile

File I/O abstraction library written in C.

Provides cross-platform file and directory operations, path utilities,
and filesystem queries.

## API

### File API
- Open / close files
- Read / write files
- Seek / tell
- Flush
- File size

### Directory API
- Open / close directory
- Read the entries

### Path utilities

#### String-based path helpers
- Join path
- Normalize (resolves `.` and `..` lexically, converts `\` and `/` to `SN_PATH_SEPARATOR`)
- File name and extension

#### Filesystem queries
- Path exists
- Path is file
- Path is directory

#### Filesystem modification
- Create directory
- Delete file / directory
- Copy file
- Move file

#### File information
```c
sn_file_stat(const char *path, snFileInfo *info);
```
Provides:
- File size
- Access / modification / change times
- File type (file, directory, or symlink)

> **Note:** No thread-safety guarantees are provided.

## Usage

```c
#include <snfile/snfile.h>
#include <stdio.h>

int main(void) {
    // Write to a file
    snFile file;
    if (sn_file_open(&file, "hello.txt", SN_FILE_MODE_WRITE)) {
        sn_file_write(&file, "Hello, SnFile!", 14);
        sn_file_close(&file);
    }

    // Read from a file
    char buf[64];
    if (sn_file_open(&file, "hello.txt", SN_FILE_MODE_READ)) {
        uint64_t bytes = sn_file_read(&file, buf, sizeof(buf) - 1);
        buf[bytes] = '\0';
        printf("Read: %s\n", buf);
        sn_file_close(&file);
    }

    // Path joining
    char joined[256];
    sn_path_join(joined, sizeof(joined), "/base", "sub", "file.txt", NULL);
    printf("Joined: %s\n", joined);

    return 0;
}
```

## Adding to your project

```cmake
include(FetchContent)
FetchContent_Declare(snfile
    GIT_REPOSITORY https://github.com/kshku/SnFile.git
    GIT_TAG <tag>  # e.g., v0.1.0
)
FetchContent_MakeAvailable(snfile)

target_link_libraries(myapp PRIVATE snfile)
```

## Build

```sh
cmake -B build
cmake --build build
```

## Platform Support

| Platform | Backend |
|----------|---------|
| Linux | POSIX (`open`, `read`, `write`, `opendir`, etc.) |
| macOS | POSIX |
| Windows | Win32 (`CreateFileA`, `ReadFile`, `FindFirstFileA`, etc.) |

## Dependencies

- **SnCore** — fetched automatically via FetchContent
