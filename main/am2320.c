#include <i2cdev.c>
#include <math.h>
#include <esp_log.h>
#include <string.h>
#include <ets_sys.h>

#define AM2320_I2C_ADDR (0x5c)
#define I2C_FREQ_HZ (100000)
#define MODBUS_READ (0x03)
#define REG_RH_H (0x00)
#define REG_T_H (0x02)
#define REG_MODEL_H (0x08)
#define REG_VER (0x0a)
#define REG_DEV_ID_H (0x0b)

#define DELAY_T1_US (800 + 100)
#define DELAY_T2_US (1500 + 100)

static const char *AM2320_TAG = "AM2320";

// spec page 20
static esp_err_t read_reg_modbus(i2c_dev_t *dev, uint8_t reg, uint8_t len, uint8_t *buf) {
    uint8_t req[] = {MODBUS_READ, reg, len};
    uint8_t resp[len + 4];
    esp_err_t err = ESP_FAIL;

    I2C_DEV_TAKE_MUTEX(dev);

    err = i2c_dev_probe(dev, I2C_DEV_READ);
    if (err == ESP_FAIL) {
        ESP_LOGD(AM2320_TAG, "i2c_dev_probe: %s", esp_err_to_name(err));
    }
    ets_delay_us(DELAY_T1_US);

    err = i2c_dev_write(dev, NULL, 0, req, sizeof(req));
    if (err != ESP_OK) {
        ESP_LOGE(AM2320_TAG, "i2c_dev_write(): %s", esp_err_to_name(err));
        goto fail;
    }
    ets_delay_us(DELAY_T2_US);

    err = i2c_dev_read(dev, NULL, 0, resp, sizeof(resp));
    if (err != ESP_OK) {
        ESP_LOGE(AM2320_TAG, "i2c_dev_read(): %s", esp_err_to_name(err));
        goto fail;
    }

    if (resp[0] != MODBUS_READ) {
        ESP_LOGE(AM2320_TAG, "Invalid MODBUS reply (%d != 0x03)", resp[0]);
        err = ESP_ERR_INVALID_RESPONSE;
        goto fail;
    }
    if (resp[1] != len) {
        ESP_LOGE(AM2320_TAG, "Invalid MODBUS reply length (%d != %d)", resp[1], len);
        err = ESP_ERR_INVALID_RESPONSE;
        goto fail;
    }

    memcpy(buf, resp + 2, len);

    fail:
    I2C_DEV_GIVE_MUTEX(dev);
    return err;
}

static float convert_temperature(uint16_t raw) {
    if (raw == 0xffff)
        return NAN;
    float res = raw & 0x8000
                ? (float) -(int16_t)(raw & 0x7fff)
                : (float) (raw);
    return res / 10.0f;
}

static inline float convert_humidity(uint16_t raw) {
    return raw != 0xffff
           ? (float) raw / 10.0f
           : NAN;
}

esp_err_t am2320_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio) {
    dev->port = port;
    dev->addr = AM2320_I2C_ADDR;
    dev->cfg.sda_io_num = sda_gpio;
    dev->cfg.scl_io_num = scl_gpio;
    dev->cfg.master.clk_speed = I2C_FREQ_HZ;
    return i2c_dev_create_mutex(dev);
}

esp_err_t am2320_read(i2c_dev_t *dev, float *temperature, float *humidity) {
    uint8_t buf[4] = {0xff, 0xff, 0xff, 0xff};
    esp_err_t err = read_reg_modbus(dev, REG_RH_H, 4, buf);
    if (err != ESP_OK) {
        return err;
    }
    *humidity = convert_humidity(((uint16_t) buf[0] << 8) + buf[1]);
    *temperature = convert_temperature(((uint16_t) buf[2] << 8) + buf[3]);
    return ESP_OK;
}