#include "RobotEyeNetwork.h"

#include "CameraEngine.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "mdns.h"
#include "nvs_flash.h"
#include <cstring>
#include <atomic>

#define ROBOTEYE_WIFI_SSID CONFIG_ROBOTEYE_WIFI_SSID
#define ROBOTEYE_WIFI_PASSWORD CONFIG_ROBOTEYE_WIFI_PASSWORD

static const char *TAG = "RobotEyeNet";
static httpd_handle_t server = nullptr;
static bool mdns_started = false;
static std::atomic_bool face_present{false};

void RobotEyeNetworkSetFacePresent(bool present) {
    face_present.store(present, std::memory_order_relaxed);
}

static esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "capture failed");
        return ESP_FAIL;
    }
    if (fb->format != PIXFORMAT_JPEG) {
        esp_camera_fb_return(fb);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "camera is not JPEG");
        return ESP_FAIL;
    }

    // Copy the JPEG before sending it. Holding the camera frame buffer while
    // a Wi-Fi client reads the response can occupy both frame buffers and
    // cause cam_hal FB-OVF while face tracking is running.
    const size_t jpeg_len = fb->len;
    uint8_t *jpeg_copy = static_cast<uint8_t *>(
        heap_caps_malloc(jpeg_len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (jpeg_copy == nullptr) {
        esp_camera_fb_return(fb);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "out of memory");
        return ESP_ERR_NO_MEM;
    }
    std::memcpy(jpeg_copy, fb->buf, jpeg_len);
    esp_camera_fb_return(fb);

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    esp_err_t err = httpd_resp_send(
        req,
        reinterpret_cast<const char *>(jpeg_copy),
        jpeg_len);
    heap_caps_free(jpeg_copy);
    return err;
}

static esp_err_t presence_handler(httpd_req_t *req) {
    const char *body = face_present.load(std::memory_order_relaxed)
        ? "{\"face\":true}"
        : "{\"face\":false}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, body, HTTPD_RESP_USE_STRLEN);
}

static void start_server() {
    if (server != nullptr) {
        return;
    }
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "HTTP server start failed");
        return;
    }
    httpd_uri_t capture = {};
    capture.uri = "/capture";
    capture.method = HTTP_GET;
    capture.handler = capture_handler;
    httpd_register_uri_handler(server, &capture);
    httpd_uri_t presence = {};
    presence.uri = "/presence";
    presence.method = HTTP_GET;
    presence.handler = presence_handler;
    httpd_register_uri_handler(server, &presence);
    ESP_LOGI(TAG, "Camera endpoint ready: http://roboteye.local/capture");
}

static void on_wifi_event(void *, esp_event_base_t base, int32_t id, void *) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected; retrying");
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_ip_info_t info = {};
        esp_netif_get_ip_info(netif, &info);
        ESP_LOGI(TAG, "Wi-Fi connected; camera IP: " IPSTR, IP2STR(&info.ip));
        if (!mdns_started) {
            if (mdns_init() == ESP_OK) {
                mdns_hostname_set("roboteye");
                mdns_instance_name_set("Roboteye Camera");
                mdns_started = true;
            }
        }
        start_server();
    }
}

bool RobotEyeNetworkStart() {
    if (std::strlen(ROBOTEYE_WIFI_SSID) == 0) {
        ESP_LOGW(TAG, "Wi-Fi credentials are empty; set ROBOTEYE_WIFI_SSID/PASSWORD in menuconfig");
        return false;
    }
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES || nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_err);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi_event, nullptr));
    wifi_config_t config = {};
    std::strncpy(reinterpret_cast<char *>(config.sta.ssid), ROBOTEYE_WIFI_SSID, sizeof(config.sta.ssid) - 1);
    std::strncpy(reinterpret_cast<char *>(config.sta.password), ROBOTEYE_WIFI_PASSWORD, sizeof(config.sta.password) - 1);
    config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
    ESP_ERROR_CHECK(esp_wifi_start());
    return true;
}
