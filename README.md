# SnFile

File I/O abstraction library written in C.

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
#### Pure string-based path helpers:
- Join path
- Normalize (resolves `.` and `..` lexically, converts `\` and `/` to `SN_PATH_SEPARATOR`
    which is defined based on platform)
- File name and extension

#### Filesystem queries
- Path exists
- Path is file
- Path is directory

#### File system Modification
- Create directory
- Delete file / directory
- Copy file
- Move file

#### File information
`sn_file_stat(const char *path, snFileInfo *info);`
Provides
- File size
- Access / modification / change times
- File type (file, directory, or symlink)

NOTE: No thread-safety guarantees are provided

