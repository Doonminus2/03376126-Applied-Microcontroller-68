#pragma once

#include "esp_err.h"
#include "hal/adc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Call once before ldr_read_raw()/ldr_read_mv(). channel = ADC1 channel wired to the LDR divider.
esp_err_t ldr_init(adc_channel_t channel);

// Raw ADC counts.
esp_err_t ldr_read_raw(int *raw_out);

// Calibrated millivolts (falls back to raw-only if calibration unsupported on this chip).
esp_err_t ldr_read_mv(int *mv_out);

#ifdef __cplusplus
}
#endif
