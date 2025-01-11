
#ifndef MSU_FS_H
#define MSU_FS_H

#include <stdbool.h>
#include <msulib/str.h>

typedef struct fs_stat {
    bool is_file;
    bool is_directory;
    bool is_link;
    time_t creation_time;
    time_t modification_time;
    time_t access_time;
} fs_stat_t;

typedef enum fs_error {
    FS_ERROR_NONE = 0,
    FS_ERROR_INVALID_PATH,
    FS_ERROR_PATH_RESOLVE,
    FS_ERROR_OPEN_READ,
    FS_ERROR_READ,
    FS_ERROR_TELL,
    FS_ERROR_SEEK,
    FS_ERROR_STAT,
    FS_ERROR_CHDIR,
    FS_ERROR_CWDIR,
} fs_error_t;

fs_error_t fs_resolve_path(const msu_str_t *path, const msu_str_t **out);
fs_error_t fs_path_within_dir(const msu_str_t *path, const msu_str_t *dir, bool *outwithin);
fs_error_t fs_path_is_relative(const msu_str_t *path, bool *out_isrelative);
fs_error_t fs_get_file_size(const msu_str_t *path, size_t *outsize);
fs_error_t fs_read_to_string(const msu_str_t *path, const msu_str_t **out);
fs_error_t fs_path_extension(const msu_str_t *path, const msu_str_t **out);
fs_error_t fs_get_stat(const msu_str_t *path, fs_stat_t *out);

fs_error_t get_working_directory(const msu_str_t **outpath);
fs_error_t set_working_directory(const msu_str_t *path);

#endif