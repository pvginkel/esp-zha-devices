#include "support.h"

#include "Attribute.h"

LOG_TAG(Attribute);

esp_zb_attribute_list_t* Attribute::getZbAttribute() {
    for (auto attr = _cluster->_cluster; attr; attr = attr->next) {
        if (attr->attribute.id == _attributeId) {
            return attr;
        }
    }

    return nullptr;
};

std::string AttributeString::getValue() {
    ZbLockGuard zbLock;

    const auto attr = getZbAttribute();

    if (!attr) {
        ESP_LOGE(TAG, "Attribute not found");
        return "";
    }
    if (!attr->attribute.data_p) {
        ESP_LOGE(TAG, "Data is null");
        return "";
    }

    ZigBeeStream stream((uint8_t*)attr->attribute.data_p);

    return stream.readString();
}

void AttributeString::setValue(const char* value) {
    ZbLockGuard zbLock;

    const auto len = strlen(value);
    const auto buffer = (uint8_t*)malloc(len + 1);
    buffer[0] = (uint8_t)len;
    memcpy(buffer + 1, value, len);

    ESP_LOGI(TAG, "Setting attribute %" PRIu16 " value %s", getAttributeId(), value);
    printf("attribute list cluster %" PRIu16 "\n", getCluster()->getClusterId());
    for (auto attr = getZbCluster(); attr; attr = attr->next) {
        printf("  cluster_id=%" PRIu16 " attr_id=%" PRIu16 " type=%" PRIu8 " access=%" PRIu8 " data=%" PRIu32 "\n",
               attr->cluster_id, attr->attribute.id, attr->attribute.type, attr->attribute.access,
               (uint32_t)attr->attribute.data_p);
    }

    ESP_ERROR_CHECK(esp_zb_cluster_update_attr(getZbCluster(), getAttributeId(), buffer));

    free(buffer);
}

esp_err_t AttributeString::createValue() {
    ZbLockGuard zbLock;

    uint8_t value[1] = {0};
    const auto err = esp_zb_custom_cluster_add_custom_attr(
        getZbCluster(), getAttributeId(), getDataType(),
        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, (void*)value);
    ESP_LOGI(TAG, "Added attribute %" PRIu16, getAttributeId());
    printf("attribute list cluster %" PRIu16 "\n", getCluster()->getClusterId());
    for (auto attr = getZbCluster(); attr; attr = attr->next) {
        printf("  cluster_id=%" PRIu16 " attr_id=%" PRIu16 " type=%" PRIu8 " access=%" PRIu8 " data=%" PRIu32 "\n",
               attr->cluster_id, attr->attribute.id, attr->attribute.type, attr->attribute.access,
               (uint32_t)attr->attribute.data_p);
    }
    return err;
}
uint8_t AttributeUInt8::getValue() {
    ZbLockGuard zbLock;

    const auto attr = getZbAttribute();

    if (!attr) {
        ESP_LOGE(TAG, "Attribute not found");
        return 0;
    }
    if (!attr->attribute.data_p) {
        ESP_LOGE(TAG, "Data is null");
        return 0;
    }

    ZigBeeStream stream((uint8_t*)attr->attribute.data_p);

    return stream.readUInt8();
}

void AttributeUInt8::setValue(uint8_t value) {
    ZbLockGuard zbLock;

    ESP_LOGI(TAG, "Setting attribute %" PRIu16 " value %" PRIu8, getAttributeId(), value);
    printf("attribute list cluster %" PRIu16 "\n", getCluster()->getClusterId());
    for (auto attr = getZbCluster(); attr; attr = attr->next) {
        printf("  cluster_id=%" PRIu16 " attr_id=%" PRIu16 " type=%" PRIu8 " access=%" PRIu8 " data=%" PRIu32 "\n",
               attr->cluster_id, attr->attribute.id, attr->attribute.type, attr->attribute.access,
               (uint32_t)attr->attribute.data_p);
    }

    const auto err = esp_zb_cluster_update_attr(getZbCluster(), getAttributeId(), (void*)&value);
    if (err != ESP_OK) {
    }
    ESP_ERROR_CHECK(err);
}

esp_err_t AttributeUInt8::createValue() {
    ZbLockGuard zbLock;

    uint8_t value = 0;
    const auto err = esp_zb_custom_cluster_add_custom_attr(
        getZbCluster(), getAttributeId(), getDataType(),
        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, (void*)&value);
    ESP_LOGI(TAG, "Added attribute %" PRIu16, getAttributeId());
    printf("attribute list cluster %" PRIu16 "\n", getCluster()->getClusterId());
    for (auto attr = getZbCluster(); attr; attr = attr->next) {
        printf("  cluster_id=%" PRIu16 " attr_id=%" PRIu16 " type=%" PRIu8 " access=%" PRIu8 " data=%" PRIu32 "\n",
               attr->cluster_id, attr->attribute.id, attr->attribute.type, attr->attribute.access,
               (uint32_t)attr->attribute.data_p);
    }
    return err;
}

uint8_t AttributeUInt16::getValue() {
    ZbLockGuard zbLock;

    const auto attr = getZbAttribute();

    if (!attr) {
        ESP_LOGE(TAG, "Attribute not found");
        return 0;
    }
    if (!attr->attribute.data_p) {
        ESP_LOGE(TAG, "Data is null");
        return 0;
    }

    ZigBeeStream stream((uint8_t*)attr->attribute.data_p);

    return stream.readUInt16Le();
}

void AttributeUInt16::setValue(uint16_t value) {
    ZbLockGuard zbLock;

    ESP_LOGI(TAG, "Setting attribute %" PRIu16 " value %" PRIu16, getAttributeId(), value);
    printf("attribute list cluster %" PRIu16 "\n", getCluster()->getClusterId());
    for (auto attr = getZbCluster(); attr; attr = attr->next) {
        printf("  cluster_id=%" PRIu16 " attr_id=%" PRIu16 " type=%" PRIu8 " access=%" PRIu8 " data=%" PRIu32 "\n",
               attr->cluster_id, attr->attribute.id, attr->attribute.type, attr->attribute.access,
               (uint32_t)attr->attribute.data_p);
    }

    ESP_ERROR_CHECK(esp_zb_cluster_update_attr(getZbCluster(), getAttributeId(), (void*)&value));
}

esp_err_t AttributeUInt16::createValue() {
    ZbLockGuard zbLock;

    uint16_t value = 0;
    const auto err = esp_zb_custom_cluster_add_custom_attr(
        getZbCluster(), getAttributeId(), getDataType(),
        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, (void*)&value);
    ESP_LOGI(TAG, "Added attribute %" PRIu16, getAttributeId());
    printf("attribute list cluster %" PRIu16 "\n", getCluster()->getClusterId());
    for (auto attr = getZbCluster(); attr; attr = attr->next) {
        printf("  cluster_id=%" PRIu16 " attr_id=%" PRIu16 " type=%" PRIu8 " access=%" PRIu8 " data=%" PRIu32 "\n",
               attr->cluster_id, attr->attribute.id, attr->attribute.type, attr->attribute.access,
               (uint32_t)attr->attribute.data_p);
    }
    return err;
}
