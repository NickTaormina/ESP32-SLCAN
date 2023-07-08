#include "flash_handler.h"

void spiffs_init(void)
{
    ESP_LOGI("spiffs", "Initializing SPIFFS");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err != ESP_OK) {
        if (err == ESP_FAIL) {
            ESP_LOGE("spiffs", "Failed to mount or format filesystem");
        } else if (err == ESP_ERR_NOT_FOUND) {
            ESP_LOGE("spiffs", "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE("spiffs", "Failed to initialize SPIFFS (%s)", esp_err_to_name(err));
        }
        return;
    }
    ESP_LOGI("spiffs", "SPIFFS initialized");
}

char** get_spiffs_file_list()
{
    char** files = (char**)malloc(sizeof(char*) * MAX_SPIFFS_FILES);
    memset(files, 0, sizeof(char*) * MAX_SPIFFS_FILES);

    DIR* dir = opendir("/spiffs");
    if (dir == NULL) {
        perror("Failed to open directory");
        return NULL;
    }

    struct dirent* entry;
    int i = 0;
    while ((entry = readdir(dir)) != NULL && i < MAX_SPIFFS_FILES) {
        if (entry->d_type == DT_REG) {
            files[i] = (char*)malloc(strlen(entry->d_name) + 1);
            //printf("Found file: %s\n", entry->d_name);
            strcpy(files[i], entry->d_name);
            i++;
        }
    }

    closedir(dir);
    free (entry);
    return files;
}

void spiffs_deinit()
{
    esp_vfs_spiffs_unregister(NULL);
}

void write_spiffs_file(const char *path, char *data)
{
    FILE* f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE("spiffs", "Failed to open file for writing");
        return;
    }
    fprintf(f, "%s", data);
    fclose(f);
}

char *read_spiffs_file_to_buffer(const char *path)
{
    ESP_LOGI("spiffs", "Reading file %s", path);
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        ESP_LOGE("spiffs", "Failed to open file for reading");
        return "";
    }

    // Allocate a buffer to hold the file contents
    const size_t block_size = 1024; // Read 1 KB at a time
    size_t content_size = 0;
    char *content = (char *) malloc(block_size);
    if (content == NULL) {
        ESP_LOGE("spiffs", "Failed to allocate memory for file contents");
        fclose(f);
        return "";
    }

    // Read the file in blocks
    size_t bytes_read = 0;
    while ((bytes_read = fread(content + content_size, 1, block_size, f)) > 0) {
        content_size += bytes_read;
        char *new_content = (char *) realloc(content, content_size + block_size);
        if (new_content == NULL) {
            ESP_LOGE("spiffs", "Failed to allocate memory for file contents");
            fclose(f);
            free(content);
            return "";
        }
        content = new_content;
    }

    // Add a null terminator to the end of the file contents
    char *new_content = (char *) realloc(content, content_size + 1);
    if (new_content == NULL) {
        ESP_LOGE("spiffs", "Failed to allocate memory for file contents");
        fclose(f);
        free(content);
        return "";
    }
    content = new_content;
    content[content_size] = '\0';
    fclose(f);
    //free (new_content);

    
    return content;

    
}

bool file_exists(const char *path)
{
   FILE *file = fopen(path, "r");
    if (file) {
        fclose(file);
        return true;
    } else {
        return false;
    }

}

void delete_spiffs_file(const char *path)
{
    ESP_LOGI("spiffs", "Deleting file %s", path);
    
    if (remove(path) == 0) {
        ESP_LOGI("spiffs", "Deleted successfully");
    } else {
        ESP_LOGE("spiffs", "Unable to delete the file");
    }
}

