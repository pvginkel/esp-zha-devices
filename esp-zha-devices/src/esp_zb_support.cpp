#include "esp_zb_support.h"
#include "support.h"

extern "C" esp_err_t esp_zb_basic_cluster_add_string_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id,
                                                          const char *value_p) {
    auto const len = strlen(value_p);
    ESP_ERROR_ASSERT(len < 256);
    auto copy = (char *)malloc(len + 2);
    copy[0] = len;
    memcpy(copy + 1, value_p, len + 1);

    return esp_zb_basic_cluster_add_attr(attr_list, attr_id, copy);
}
