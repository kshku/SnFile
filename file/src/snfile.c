#include "snfile/snfile.h"

bool sn_path_join(char *dst, size_t dst_size, const char *a, const char *b) {
    size_t i = 0;

    // Copy a
    while (i < dst_size - 1 && a[i]) {
        dst[i] = a[i];
        ++i;
    }

    // Add separator if not there
    if (i > 0 && (a[i - 1] != '/' && a[i - 1] != '\\') && i < dst_size - 1)
        dst[i++] = SN_PATH_SEPARATOR;

    if (i == dst_size - 1) return false;

    size_t b_start = i;

    // Copy b
    while (i < dst_size - 1 && b[i - b_start]) {
        dst[i] = b[i - b_start];
        ++i;
    }

    if (b[i - b_start]) return false;

    dst[i] = 0;
    return true;
}

void sn_path_normalize(char* path) {
    char *last_sep = path;
    char *last2_sep = path;
    size_t dots = 0;
    char *write = path;
    for (char *read = path; *read; ++read, ++write) {
        switch (*read) {
            case '\\':
            case '/':
                *write = SN_PATH_SEPARATOR;
                last2_sep = last_sep;
                last_sep = write;
                break;
            case '.':
                dots = 1;
                if (*(read + dots) == '.') ++dots;

                if (*(read + dots) == '\\' || *(read + dots) == '/') {
                    if (dots == 2) {
                        write = last2_sep;
                        last_sep = last2_sep;

                        while (last2_sep != path && *last2_sep != SN_PATH_SEPARATOR) --last2_sep;
                    } else {
                        write = last_sep;
                    }
                }

                dots = 0;
                break;
            default:
                *write = *read;
                break;
        }
    }
    *write = 0;
}

const char* sn_path_filename(const char* path) {
    const char *last = path;
    for (const char *p = path; *p; ++p)
        if (p == '\\' || p == '/') last = p + 1;

    return last;
}

const char* sn_path_extension(const char* path) {
    const char *name = sn_path_filename(path);
    const char *dot = NULL;

    for (const char *p = name; *p; ++p)
        if (p == '.') dot = p;

    return dot ? dot + 1 : NULL;
}

