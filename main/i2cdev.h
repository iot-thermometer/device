#ifndef __I2CDEV_H__
#define __I2CDEV_H__

#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_err.h>
#include <esp_idf_lib_helpers.h>

#if HELPER_TARGET_IS_ESP8266

#define I2CDEV_MAX_STRETCH_TIME 0xffffffff

#else

#include <soc/i2c_reg.h>

#if defined(I2C_TIME_OUT_VALUE_V)
#define I2CDEV_MAX_STRETCH_TIME I2C_TIME_OUT_VALUE_V
#elif defined(I2C_TIME_OUT_REG_V)
#define I2CDEV_MAX_STRETCH_TIME I2C_TIME_OUT_REG_V
#else
#define I2CDEV_MAX_STRETCH_TIME 0x00ffffff
#endif

#endif

typedef struct {
    i2c_port_t port;
    i2c_config_t cfg;
    uint8_t addr;
    SemaphoreHandle_t mutex;
    uint32_t timeout_ticks;
} i2c_dev_t;

typedef enum {
    I2C_DEV_WRITE = 0,
    I2C_DEV_READ
} i2c_dev_type_t;

esp_err_t i2cdev_init();

esp_err_t i2cdev_done();

esp_err_t i2c_dev_create_mutex(i2c_dev_t *dev);

esp_err_t i2c_dev_delete_mutex(i2c_dev_t *dev);

esp_err_t i2c_dev_take_mutex(i2c_dev_t *dev);

esp_err_t i2c_dev_give_mutex(i2c_dev_t *dev);

esp_err_t i2c_dev_probe(const i2c_dev_t *dev, i2c_dev_type_t operation_type);

esp_err_t i2c_dev_read(const i2c_dev_t *dev, const void *out_data,
                       size_t out_size, void *in_data, size_t in_size);

esp_err_t i2c_dev_write(const i2c_dev_t *dev, const void *out_reg,
                        size_t out_reg_size, const void *out_data, size_t out_size);

esp_err_t i2c_dev_read_reg(const i2c_dev_t *dev, uint8_t reg,
                           void *in_data, size_t in_size);

esp_err_t i2c_dev_write_reg(const i2c_dev_t *dev, uint8_t reg,
                            const void *out_data, size_t out_size);

#define I2C_DEV_TAKE_MUTEX(dev)                 \
    do                                          \
    {                                           \
        esp_err_t __ = i2c_dev_take_mutex(dev); \
        if (__ != ESP_OK)                       \
            return __;                          \
    } while (0)

#define I2C_DEV_GIVE_MUTEX(dev)                 \
    do                                          \
    {                                           \
        esp_err_t __ = i2c_dev_give_mutex(dev); \
        if (__ != ESP_OK)                       \
            return __;                          \
    } while (0)

#define I2C_DEV_CHECK(dev, X)        \
    do                               \
    {                                \
        esp_err_t ___ = X;           \
        if (___ != ESP_OK)           \
        {                            \
            I2C_DEV_GIVE_MUTEX(dev); \
            return ___;              \
        }                            \
    } while (0)

#define I2C_DEV_CHECK_LOGE(dev, X, msg, ...)   \
    do                                         \
    {                                          \
        esp_err_t ___ = X;                     \
        if (___ != ESP_OK)                     \
        {                                      \
            I2C_DEV_GIVE_MUTEX(dev);           \
            ESP_LOGE(TAG, msg, ##__VA_ARGS__); \
            return ___;                        \
        }                                      \
    } while (0)

#endif
