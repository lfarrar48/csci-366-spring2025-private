#include "msulib/fs.h"

#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>

#else
#include <unistd.h>
#include <sys/stat.h>
#endif

fs_error_t fs_resolve_path(const msu_str_t *path, const msu_str_t **out) {
    if (!path || msu_str_is_empty(path)) {
        *out = EMPTY_STRING;
        return FS_ERROR_INVALID_PATH;
    }

    char resolved_path[PATH_MAX] = {0};
#ifdef _WIN32
    DWORD size = GetFullPathNameA(msu_str_data(path), PATH_MAX, resolved_path, NULL);
    if (!size) return FS_ERROR_PATH_RESOLVE;
#else
    if (!realpath(msu_str_data(path), resolved_path)) {
        return FS_ERROR_PATH_RESOLVE;
    }
#endif

    *out = msu_str_new(resolved_path);
    return FS_ERROR_NONE;
}

fs_error_t fs_path_within_dir(const msu_str_t *path, const msu_str_t *dir, bool *outwithin) {
    if (msu_str_is_empty(path) || msu_str_is_empty(dir)) {
        return FS_ERROR_INVALID_PATH;
    }

    fs_error_t err = FS_ERROR_NONE;

    const msu_str_t *abs_path = NULL, *abs_dir = NULL;
    err = fs_resolve_path(path, &abs_path);
    if (err) return err;

    err = fs_resolve_path(dir, &abs_dir);
    if (err) {
        msu_str_free(abs_path);
        return err;
    }

    *outwithin = msu_str_sw(abs_path, abs_dir);
    msu_str_free(abs_path);
    msu_str_free(abs_dir);
    return FS_ERROR_NONE;
}

fs_error_t fs_path_is_relative(const msu_str_t *path, bool *out_isrelative) {
    if (msu_str_is_empty(path)) {
        return FS_ERROR_INVALID_PATH;
    }

    const char *c_path = msu_str_data(path);

#ifdef _WIN32
    *out_isrelative = !msu_str_containss(path, ":") && !msu_str_sws(path, "/");
#else
    *out_isrelative = msu_str_at(path, 0) != '/';
#endif

    return FS_ERROR_NONE;
}

fs_error_t _fs_get_file_size(FILE *f, size_t *outsize) {
    if (0 != fseek(f, 0, SEEK_END)) {
        return FS_ERROR_SEEK;
    }

    size_t filesize = (size_t) ftell(f);
    rewind(f);
    if (filesize == -1 || filesize >= SIZE_MAX) {
        return FS_ERROR_TELL;
    }

    *outsize = filesize;
    return FS_ERROR_NONE;
}

fs_error_t fs_get_file_size(const msu_str_t *path, size_t *outsize) {
    FILE *f = fopen(msu_str_data(path), "rb");
    if (!f) return FS_ERROR_OPEN_READ;

    fs_error_t err;

    err = _fs_get_file_size(f, outsize);
    fclose(f);
    if (err) return err;

    return FS_ERROR_NONE;
}

fs_error_t fs_read_to_string(const msu_str_t *path, const msu_str_t **out) {
    if (msu_str_is_empty(path)) return FS_ERROR_INVALID_PATH;

    FILE *f = fopen(msu_str_data(path), "rb");
    if (!f) return FS_ERROR_OPEN_READ;

    fs_error_t err = FS_ERROR_NONE;
    size_t fsize;

    err = _fs_get_file_size(f, &fsize);
    if (err) {
        fclose(f);
        return err;
    }

    msu_str_builder_t sb = msu_str_builder_new();
    msu_str_builder_ensure_capacity(sb, fsize + 1);

    size_t read = fread(sb->src, 1, fsize, f);
    if (read == 0) {
        msu_str_builder_free(sb);
        fclose(f);
        return FS_ERROR_READ;
    }
    sb->len = read;
    fclose(f);

    sb->src[sb->len] = 0;
    *out = msu_str_builder_into_string_and_free(sb);
    return FS_ERROR_NONE;
}

fs_error_t fs_path_extension(const msu_str_t *path, const msu_str_t **out) {
    if (msu_str_is_empty(path)) return FS_ERROR_INVALID_PATH;

    const char *extptr;
    extptr = strchr(msu_str_data(path), '.');
    if (!extptr) extptr = msu_str_data(path) + msu_str_len(path);

    size_t start = extptr - msu_str_data(path);
    *out = msu_str_substring(path, start, msu_str_len(path));
    return FS_ERROR_NONE;
}

#ifdef _WIN32
time_t system_time_to_time_t(const SYSTEMTIME *st) {
    FILETIME ft;
    SystemTimeToFileTime(st, &ft);

    // Convert FILETIME to time_t (Windows epoch is 1601, Unix epoch is 1970)
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // The difference between the Windows and Unix epochs is 11644473600 seconds.
    const time_t EPOCH_DIFF = 11644473600;

    return (time_t)((uli.QuadPart / 10000000) - EPOCH_DIFF);
}
#endif

fs_error_t fs_get_stat(const msu_str_t *path, fs_stat_t *out) {
    if (path == NULL) {
        return FS_ERROR_INVALID_PATH;
    }

#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA file_info;
    if (GetFileAttributesExA(msu_str_data(path), GetFileExInfoStandard, &file_info) == 0) {
        return FS_ERROR_STAT;
    }

    out->is_file = !(file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    out->is_directory = !out->is_file;

    SYSTEMTIME creation_time, modification_time, access_time;
    FileTimeToSystemTime(&file_info.ftCreationTime, &creation_time);
    FileTimeToSystemTime(&file_info.ftLastWriteTime, &modification_time);
    FileTimeToSystemTime(&file_info.ftLastAccessTime, &access_time);

    out->creation_time = system_time_to_time_t(&creation_time);
    out->modification_time = system_time_to_time_t(&modification_time);
    out->access_time = system_time_to_time_t(&access_time);
#else
    struct stat file_stat;
    if (stat(msu_str_data(path), &file_stat) == -1) {
        return FS_ERROR_STAT;
    }

    out->is_file = S_ISREG(file_stat.st_mode);
    out->is_directory = S_ISDIR(file_stat.st_mode);

    out->creation_time = file_stat.st_ctime;
    out->modification_time = file_stat.st_mtime;
    out->access_time = file_stat.st_atime;
#endif

    return FS_ERROR_NONE;
}

fs_error_t get_working_directory(const msu_str_t **outpath) {
    char buffer[PATH_MAX];
#ifdef _WIN32
    if (_getcwd(buffer, sizeof(buffer)) == NULL) {
        return FS_ERROR_CWDIR;
    }
    *outpath = msu_str_new(buffer);
#else
    if (getcwd(buffer, sizeof(buffer)) == NULL) {
        return FS_ERROR_CWDIR;
    }
    *outpath = msu_str_new(buffer);
#endif

    return FS_ERROR_NONE;
}

fs_error_t set_working_directory(const msu_str_t *path) {
    if (msu_str_is_empty(path)) {
        return FS_ERROR_INVALID_PATH;
    }

#ifdef _WIN32
    if (_chdir(msu_str_data(path)) != 0) {
        return FS_ERROR_CHDIR;
    }
#else
    if (chdir(msu_str_data(path)) != 0) {
        return FS_ERROR_CHDIR;
    }
#endif

    return FS_ERROR_NONE;
}
