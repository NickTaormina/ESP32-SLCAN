#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#include "esp_spiffs.h"
#include "esp_log.h"
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#define SPIFFS_SIZE (1 * 512 * 1024); // 512K
#define MAX_SPIFFS_FILES 100
    void spiffs_init(void);
    char **get_spiffs_file_list();
    void spiffs_deinit();
    void write_spiffs_file(const char *path, char *data);
    char *read_spiffs_file_to_buffer(const char *path);
    bool file_exists(const char *path);
    void delete_spiffs_file(const char *path);

#ifdef __cplusplus
} /*extern "C"*/
#endif
