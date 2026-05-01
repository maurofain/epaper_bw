#ifndef CONDITIONAL_LOG_H
#define CONDITIONAL_LOG_H

#include <esp_log.h>

// Logging macro configuration - controlled by CMakeLists.txt
// Set ENABLE_LOG=1 in main/CMakeLists.txt to enable logging output
#ifndef ENABLE_LOG
#define ENABLE_LOG 0
#endif

// Use 0/1 comparison for consistency with other feature flags
#if ENABLE_LOG == 1
#define _LOGI(...) ESP_LOGI(TAG, __VA_ARGS__)
#define _LOGD(...) ESP_LOGD(TAG, __VA_ARGS__)
#define _LOGE(...) ESP_LOGE(TAG, __VA_ARGS__)
#define _LOGW(...) ESP_LOGW(TAG, __VA_ARGS__)
#else
#define _LOGI(...) do { } while(0)
#define _LOGD(...) do { } while(0)
#define _LOGE(...) do { } while(0)
#define _LOGW(...) do { } while(0)
#endif

#endif // CONDITIONAL_LOG_H
