#include "ldr.h"

#include <stddef.h>
#include <stdbool.h>
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

#define LDR_ATTEN    ADC_ATTEN_DB_12
#define LDR_BITWIDTH ADC_BITWIDTH_DEFAULT

static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
static adc_cali_handle_t s_cali_handle = NULL;
static bool s_cali_ok = false;
static adc_channel_t s_channel;

esp_err_t ldr_init(adc_channel_t channel)
{
    s_channel = channel;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    esp_err_t err = adc_oneshot_new_unit(&init_config, &s_adc1_handle);
    if (err != ESP_OK) return err;

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = LDR_ATTEN,
        .bitwidth = LDR_BITWIDTH,
    };
    err = adc_oneshot_config_channel(s_adc1_handle, s_channel, &channel_config);
    if (err != ESP_OK) return err;

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = LDR_ATTEN,
        .bitwidth = LDR_BITWIDTH,
#if CONFIG_IDF_TARGET_ESP32
        .default_vref = 1100,
#endif
    };
    s_cali_ok = (adc_cali_create_scheme_line_fitting(&cali_config, &s_cali_handle) == ESP_OK);
#endif

    return ESP_OK;
}

esp_err_t ldr_read_raw(int *raw_out)
{
    if (s_adc1_handle == NULL || raw_out == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return adc_oneshot_read(s_adc1_handle, s_channel, raw_out);
}

esp_err_t ldr_read_mv(int *mv_out)
{
    if (mv_out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int raw = 0;
    esp_err_t err = ldr_read_raw(&raw);
    if (err != ESP_OK) return err;

    if (s_cali_ok) {
        return adc_cali_raw_to_voltage(s_cali_handle, raw, mv_out);
    }

    *mv_out = raw;
    return ESP_ERR_NOT_SUPPORTED;
}
