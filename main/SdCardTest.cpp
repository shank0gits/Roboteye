#include "SdCardTest.h"

#include "driver/gpio.h"
#include "driver/sdmmc_default_configs.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <dirent.h>

static const char* TAG = "RobotEyeSD";

// Common ESP32-S3-EYE 1-bit SDMMC wiring.
static constexpr gpio_num_t kSdClk = GPIO_NUM_39;
static constexpr gpio_num_t kSdCmd = GPIO_NUM_38;
static constexpr gpio_num_t kSdD0 = GPIO_NUM_40;

bool RobotEyeSdCardTest() {
    ESP_LOGI(TAG, "Starting SD-card test (1-bit SDMMC, CLK=%d CMD=%d D0=%d)", kSdClk, kSdCmd, kSdD0);

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    slot_config.clk = kSdClk;
    slot_config.cmd = kSdCmd;
    slot_config.d0 = kSdD0;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {};
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 2;
    mount_config.allocation_unit_size = 16 * 1024;

    sdmmc_card_t* card = nullptr;
    esp_err_t err = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SD-card mount failed: %s (0x%x)", esp_err_to_name(err), err);
        ESP_LOGE(TAG, "Check insertion, FAT format, and pins CLK=%d CMD=%d D0=%d", kSdClk, kSdCmd, kSdD0);
        return false;
    }

    sdmmc_card_print_info(stdout, card);
    ESP_LOGI(TAG, "SD-card mounted successfully at /sdcard");

    DIR* root = opendir("/sdcard");
    if (root == nullptr) {
        ESP_LOGE(TAG, "SD-card directory read failed: errno=%d (%s)", errno, std::strerror(errno));
    } else {
        int entries = 0;
        while (readdir(root) != nullptr) {
            ++entries;
        }
        closedir(root);
        ESP_LOGI(TAG, "SD-card directory read OK: %d entries", entries);
    }

    // Use an 8.3 FAT filename so the test also works when long filenames are
    // disabled in the ESP-IDF FATFS configuration.
    constexpr char kTestPath[] = "/sdcard/SDTEST.TXT";
    constexpr char kTestText[] = "RobotEye SD read/write test OK\n";

    FILE* file = std::fopen(kTestPath, "w");
    if (file == nullptr) {
        ESP_LOGE(TAG, "SD-card write test failed: cannot open %s; errno=%d (%s)",
                 kTestPath, errno, std::strerror(errno));
        return false;
    }
    const size_t expected = std::strlen(kTestText);
    const size_t written = std::fwrite(kTestText, 1, expected, file);
    std::fclose(file);
    if (written != expected) {
        ESP_LOGE(TAG, "SD-card write test failed: wrote %u/%u bytes", static_cast<unsigned>(written), static_cast<unsigned>(expected));
        return false;
    }

    char buffer[sizeof(kTestText)] = {};
    file = std::fopen(kTestPath, "r");
    if (file == nullptr) {
        ESP_LOGE(TAG, "SD-card read test failed: cannot reopen %s; errno=%d (%s)",
                 kTestPath, errno, std::strerror(errno));
        return false;
    }
    const size_t read = std::fread(buffer, 1, expected, file);
    std::fclose(file);
    if (read != expected || std::memcmp(buffer, kTestText, expected) != 0) {
        ESP_LOGE(TAG, "SD-card read/verify test failed");
        return false;
    }

    ESP_LOGI(TAG, "SD-card TEST PASSED: mount + write + read verified");
    return true;
}
