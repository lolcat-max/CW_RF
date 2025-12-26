#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_phy_init.h"

// Try different function names based on your SDK version
extern "C" {
    void phy_tx_contin_en(bool enable);
}

void app_main(void) {
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    // 3. Configure power BEFORE starting
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(80)); // 20dBm
    
    // 4. Set channel BEFORE starting
    int channel = 1;
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
    
    // 5. Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    vTaskDelay(500 / portTICK_PERIOD_MS); // Let WiFi stabilize

    printf("=== CW GENERATOR STARTED ===\n");
    printf("Frequency: 2412 MHz (Channel 1)\n");
    printf("Power: 20dBm (MAX)\n");

    // 6. Enable CW mode
    phy_tx_contin_en(true);
    printf("CW transmission enabled!\n");

    // 7. Main loop with channel hopping
    while (1) {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        
        phy_tx_contin_en(false);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        
        channel = (channel % 14) + 1;
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
        phy_tx_contin_en(true);
        
        printf("CH%d (2%d MHz)\n", channel, 412 + (channel-1)*5);
    }
}
