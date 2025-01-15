#pragma once

#include "sdkconfig.h"
#include "soc/soc_caps.h"

#if !SOC_IEEE802154_SUPPORTED || !CONFIG_ZB_ENABLED
#error SOC must support IEEE802154 and ZigBee must be enabled.
#endif

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_zigbee_core.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// See https://github.com/espressif/arduino-esp32/issues/9745#issuecomment-2165478493.
#ifdef __cplusplus
extern "C" {
#endif
#include "zboss_api.h"
#ifdef __cplusplus
};
#endif

#define LOG_TAG(v) [[maybe_unused]] static const char* TAG = #v

#ifdef NDEBUG
#define ESP_ERROR_ASSERT(x) \
    do {                    \
        (void)sizeof((x));  \
    } while (0)
#elif defined(CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT)
#define ESP_ERROR_ASSERT(x)   \
    do {                      \
        if (unlikely(!(x))) { \
            abort();          \
        }                     \
    } while (0)
#else
#define ESP_ERROR_ASSERT(x)                                                                                    \
    do {                                                                                                       \
        if (unlikely(!(x))) {                                                                                  \
            printf("ESP_ERROR_ASSERT failed");                                                                 \
            printf(" at %p\n", __builtin_return_address(0));                                                   \
            printf("file: \"%s\" line %d\nfunc: %s\nexpression: %s\n", __FILE__, __LINE__, __ASSERT_FUNC, #x); \
            abort();                                                                                           \
        }                                                                                                      \
    } while (0)
#endif

class ZbLockGuard {
public:
    ZbLockGuard() { esp_zb_lock_acquire(portMAX_DELAY); }

    ~ZbLockGuard() { esp_zb_lock_release(); }

    ZbLockGuard(const ZbLockGuard&) = delete;
    ZbLockGuard& operator=(const ZbLockGuard&) = delete;
    ZbLockGuard(ZbLockGuard&&) = delete;
    ZbLockGuard& operator=(ZbLockGuard&&) = delete;
};
