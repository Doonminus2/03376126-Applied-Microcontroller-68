#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define I2C_SCL_IO GPIO_NUM_22
#define I2C_SDA_IO GPIO_NUM_21
#define I2C_MASTER_FREQ_HZ 100000
#define LCD_SLAVE_ADDR 0x27

static const char *TAG = "I2C_PROJECT";

void app_main(void) {
  ESP_LOGI(TAG, "Initializing Modern I2C Master Bus...");

  // 1. Define Bus Configuration Structure
  i2c_master_bus_config_t bus_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = I2C_NUM_0,
      .scl_io_num = I2C_SCL_IO,
      .sda_io_num = I2C_SDA_IO,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };

  // 2. Instantiate Master Bus Handle
  i2c_master_bus_handle_t bus_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

  // 3. Register Slave Device Configuration
  i2c_device_config_t dev_config = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = LCD_SLAVE_ADDR,
      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
  };

  i2c_master_dev_handle_t dev_handle;
  ESP_ERROR_CHECK(
      i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

  ESP_LOGI(TAG, "I2C bus registered successfully. Probing slave device at "
                "address 0x27...");

  // 4. Perform Data Transmission Probe
  uint8_t init_cmd[] = {0x08}; // Backlight control / clear command frame
  esp_err_t err = i2c_master_transmit(dev_handle, init_cmd, sizeof(init_cmd),
                                      1000 / portTICK_PERIOD_MS);

  if (err == ESP_OK) {
    ESP_LOGI(TAG,
             "Communication success! Peripheral acknowledged transfer (ACK).");
  } else {
    ESP_LOGE(TAG, "Communication failed: %s", esp_err_to_name(err));
  }
}