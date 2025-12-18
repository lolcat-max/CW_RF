/*
 * ESP32-C3 Continuous Wave (CW) Generator
 * Mode: Pure RF Tone (Unmodulated Carrier)
 * Power: Max Hardware Limit (~20dBm)
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_phy_init.h" 

// IMPORTANT: This function is in the ROM/PHY lib but often not in public headers.
// We declare it manually to link against the blob.
extern "C" void esp_phy_tx_contin_en(bool contin_en);

void app_main(void)
{
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize Wi-Fi (Required to wake up the RF hardware)
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 3. Configure High Power
    // Disable power saving to prevent RF duty cycling
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    // Set max power (Index 80 = ~20dBm)
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(80));

    // 4. Set Target Frequency
    // Channel 1 = 2412 MHz. Channel 14 = 2484 MHz.
    int channel = 1; 
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));

    printf("CW GENERATOR STARTED.\n");
    printf("Frequency: %d MHz (Channel %d)\n", 2412 + (channel-1)*5, channel);
    printf("Power: MAX\n");
    
    // 5. ENABLE CONTINUOUS WAVE MODE
    // This stops the Wi-Fi MAC and hands control to the PHY test engine.
    // The radio will now output a pure sine wave at the center frequency.
    esp_phy_tx_contin_en(true);

    // 6. Infinite Loop to keep main task alive
    // The radio is handled by hardware now; CPU can do whatever or sleep.
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        // Optional: Hop channels periodically?
        // Note: You might need to disable CW, switch channel, then re-enable CW.

        esp_phy_tx_contin_en(false);
        channel = (channel % 14) + 1;
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        esp_phy_tx_contin_en(true);

    }
}
