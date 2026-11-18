#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"

static const uint8_t module_addresses[] = {
    0x3C, // OLED display
    0x62, // SCD41
    0x77 // BME680 sensor
};
static const int num_modules = sizeof(module_addresses) / sizeof(module_addresses[0]);


#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xff
uint8_t sensirion_common_generate_crc(const uint8_t* data, uint16_t count) {
    uint16_t current_byte;
    uint8_t crc = CRC8_INIT;
    uint8_t crc_bit;
    /* calculates 8-Bit checksum with given polynomial */
    for (current_byte = 0; current_byte < count; ++current_byte) {
        crc ^= (data[current_byte]);
        for (crc_bit = 8; crc_bit > 0; --crc_bit) {
            if (crc & 0x80)
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}
void app_main(void) {
    uint8_t test[] = {0xBE, 0xEF};
    assert(sensirion_common_generate_crc(test, 2) == 0x92);
    
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_4,
        .scl_io_num = GPIO_NUM_5,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true
        }
    };
    
    i2c_master_bus_handle_t bus;

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus));

    while (1) {
        for (int i = 0; i < num_modules; i++) {
            esp_err_t err = i2c_master_probe(bus, module_addresses[i], 1000);
            if (err == ESP_OK) {
                ESP_LOGI("I2C", "Module found at address 0x%02X", module_addresses[i]);
            }

        }
    }
}