/*
 * ESP32-C3 Continuous Wave (CW) Generator - Alternative Method
 * Uses esp_wifi_80211_tx for continuous transmission
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_phy_init.h"
#include "esp_wifi_types.h"

static const char *TAG = "CW_GEN";

// Minimal WiFi frame for continuous transmission
static const uint8_t cw_packet[] = {
    0x08, 0x00,             // Frame Control
    0x00, 0x00,             // Duration
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // Destination
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,  // Source
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,  // BSSID
    0x00, 0x00              // Sequence Control
};

extern "C" void app_main(void)
{
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 3. Configure High Power
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(80)); // Max power

    // 4. Set Target Channel
    int channel = 1;
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));

    ESP_LOGI(TAG, "CW Generator Started");
    ESP_LOGI(TAG, "Frequency: %d MHz (Channel %d)", 2412 + (channel-1)*5, channel);
    ESP_LOGI(TAG, "Power: MAX (20dBm)");

    // 5. Continuous transmission using 802.11 frame injection
    while (1) {
        // Transmit frames continuously with minimal delay
        // This creates a near-continuous RF output
        esp_wifi_80211_tx(WIFI_IF_STA, cw_packet, sizeof(cw_packet), false);
        
        // Minimal delay to flood the air
        vTaskDelay(1 / portTICK_PERIOD_MS);
        
        // Optional: Channel hopping (uncomment to enable)
        
        static int hop_counter = 0;
        if (++hop_counter > 1000) {
            hop_counter = 0;
            channel = (channel % 14) + 1;
            esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            ESP_LOGI(TAG, "Hopped to channel %d", channel);
        }
        
    }
}