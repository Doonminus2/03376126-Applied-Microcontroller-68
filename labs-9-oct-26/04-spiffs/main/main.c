#include "esp_log.h"
#include "esp_spiffs.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "STORAGE_PROJECT";

void app_main(void) {
  ESP_LOGI(TAG, "Initializing SPIFFS Virtual File System...");

  // 1. Configure VFS SPIFFS Mounting Structure
  esp_vfs_spiffs_conf_t conf = {.base_path = "/spiffs",
                                .partition_label = NULL,
                                .max_files = 5,
                                .format_if_mount_failed = true};

  // 2. Register and Mount VFS SPIFFS
  esp_err_t ret = esp_vfs_spiffs_register(&conf);
  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG, "Failed to find SPIFFS partition in partition table");
    } else {
      ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
    }
    return;
  }

  ESP_LOGI(TAG, "SPIFFS mounted successfully at /spiffs. Executing POSIX Write "
                "Operation...");

  // 3. Perform Standard POSIX File Write Call
  FILE *f = fopen("/spiffs/log.txt", "w");
  if (f == NULL) {
    ESP_LOGE(TAG, "Failed to open log.txt for writing");
    return;
  }
  fprintf(f, "Sensor Log Entry #104: Temp=24.5C, Humidity=60%%");
  fclose(f);
  ESP_LOGI(TAG, "File successfully written and flushed to flash.");

  // 4. Perform POSIX Read and Verification
  FILE *f_read = fopen("/spiffs/log.txt", "r");
  if (f_read == NULL) {
    ESP_LOGE(TAG, "Failed to open log.txt for reading");
    return;
  }
  char line[128];
  fgets(line, sizeof(line), f_read);
  fclose(f_read);

  ESP_LOGI(TAG, "Read back string from SPIFFS: %s", line);

  // 5. Unmount Filesystem Cleanly
  esp_vfs_spiffs_unregister(conf.partition_label);
  ESP_LOGI(TAG, "SPIFFS filesystem unmounted cleanly.");
}