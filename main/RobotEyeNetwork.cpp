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
#include <cctype>
#include <cstring>
#include <atomic>
#include <cstdio>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

#define ROBOTEYE_WIFI_SSID CONFIG_ROBOTEYE_WIFI_SSID
#define ROBOTEYE_WIFI_PASSWORD CONFIG_ROBOTEYE_WIFI_PASSWORD

static const char *TAG = "RobotEyeNet";
static httpd_handle_t server = nullptr;
static bool mdns_started = false;
static std::atomic_bool face_present{false};

// Defined by RobotEye.cpp. The HTTP endpoint must use the same synchronized
// camera path as the face-tracking task.
extern CameraEngine camera;

static bool get_safe_upload_name(httpd_req_t* req, char* name, size_t name_size) {
    char query[160] = {};
    char value[96] = {};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK ||
        httpd_query_key_value(query, "name", value, sizeof(value)) != ESP_OK) {
        return false;
    }

    // Accept only a simple filename; never allow a path from the browser.
    const char* base = std::strrchr(value, '/');
    if (base == nullptr) base = std::strrchr(value, '\\');
    base = base == nullptr ? value : base + 1;
    if (*base == '\0' || std::strcmp(base, ".") == 0 || std::strcmp(base, "..") == 0) {
        return false;
    }
    size_t output_length = 0;
    for (const char* p = base; *p != '\0'; ++p) {
        if (output_length + 1 >= name_size) return false;
        const unsigned char character = static_cast<unsigned char>(*p);
        name[output_length++] =
            (std::isalnum(character) || *p == '.' || *p == '_' || *p == '-') ? *p : '_';
    }
    name[output_length] = '\0';
    if (output_length == 0 || std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0) {
        return false;
    }
    return true;
}

static esp_err_t web_app_handler(httpd_req_t *req) {
    static const char page[] = R"HTML(<!doctype html>
<html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>RobotEye Dashboard</title><style>
body{font:16px system-ui,sans-serif;max-width:860px;margin:auto;padding:18px;background:#10131a;color:#edf2f7}
.card{background:#1b2230;border:1px solid #2d3748;border-radius:16px;padding:16px;margin:12px 0}
button{padding:9px 14px;border:0;border-radius:9px;background:#22c55e;font-weight:700;cursor:pointer;margin:4px}
img{display:block;max-width:100%;max-height:480px;border-radius:12px;background:#080a0e;margin-top:10px}
#files div{padding:7px 0;border-bottom:1px solid #30394a}
</style></head><body>
<h2>RobotEye Dashboard</h2>
<div class="card"><h3>Camera</h3><button onclick="capture()">Capture image</button><img id="camera" alt="Capture will appear here"></div>
<div class="card"><h3>SD Card</h3>
<input id="file" type="file"><button id="button" onclick="upload()">Upload</button>
<progress id="progress" value="0" max="100" style="width:100%;display:none"></progress>
<pre id="status">Select a file.</pre>
<h3>Files</h3><div id="files">Loading...</div></div>
<script>
function capture(){document.getElementById('camera').src='/capture?t='+Date.now();}
async function refreshFiles(){
  const target=document.getElementById('files');
  try{
    const response=await fetch('/files',{cache:'no-store'});
    if(!response.ok) throw new Error(await response.text());
    const data=await response.json();
    target.innerHTML='';
    if(!data.files.length){target.textContent='No files found.';return;}
    data.files.forEach(function(file){
      const row=document.createElement('div');
      row.textContent=file.name+' ('+file.size+' bytes) ';
      const button=document.createElement('button');
      button.textContent='Delete';
      button.onclick=async function(){
        if(!confirm('Delete '+file.name+'?')) return;
        const result=await fetch('/delete?name='+encodeURIComponent(file.name),{method:'POST'});
        document.getElementById('status').textContent=await result.text();
        refreshFiles();
      };
      row.appendChild(button); target.appendChild(row);
    });
  }catch(error){target.textContent='Unable to list files: '+error;}
}
function upload(){
  const f=document.getElementById('file').files[0];
  if(!f){document.getElementById('status').textContent='Select a file first.';return;}
  const button=document.getElementById('button');
  const progress=document.getElementById('progress');
  const status=document.getElementById('status');
  const request=new XMLHttpRequest();
  button.disabled=true;
  progress.value=0;
  progress.style.display='block';
  status.textContent='Uploading '+f.name+'... 0%';
  request.upload.onprogress=function(event){
    if(event.lengthComputable){
      const percent=Math.round(event.loaded*100/event.total);
      progress.value=percent;
      status.textContent='Uploading '+f.name+'... '+percent+'%';
    }
  };
  request.onload=function(){
    button.disabled=false;
    if(request.status>=200 && request.status<300){
      progress.value=100;
      status.textContent=request.responseText;
      refreshFiles();
    }else{
      status.textContent='Upload failed: '+request.responseText;
    }
  };
  request.onerror=function(){
    button.disabled=false;
    status.textContent='Upload failed: connection error';
  };
  request.open('POST','/upload?name='+encodeURIComponent(f.name),true);
  request.send(f);
}
refreshFiles();
</script></body></html>)HTML";
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_send(req, page, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t upload_handler(httpd_req_t *req) {
    char name[96] = {};
    if (!get_safe_upload_name(req, name, sizeof(name))) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid filename");
        return ESP_ERR_INVALID_ARG;
    }

    char path[128] = {};
    std::snprintf(path, sizeof(path), "/sdcard/%s", name);
    FILE* file = std::fopen(path, "wb");
    if (file == nullptr) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "cannot open SD file");
        return ESP_FAIL;
    }

    std::vector<char> buffer(4096);
    size_t remaining = req->content_len;
    bool failed = false;
    while (remaining > 0) {
        const int received = httpd_req_recv(req, buffer.data(), remaining > buffer.size() ? buffer.size() : remaining);
        if (received == HTTPD_SOCK_ERR_TIMEOUT) {
            // A slow phone-hotspot or browser can pause between TCP chunks.
            // Keep waiting; the request is still valid.
            continue;
        }
        if (received <= 0) {
            failed = true;
            break;
        }
        if (std::fwrite(buffer.data(), 1, received, file) != static_cast<size_t>(received)) {
            failed = true;
            break;
        }
        remaining -= static_cast<size_t>(received);
    }
    std::fclose(file);

    if (failed || remaining != 0) {
        std::remove(path);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "SD write failed");
        return ESP_FAIL;
    }

    char response[160] = {};
    std::snprintf(response, sizeof(response), "Uploaded %s (%u bytes)", name,
                  static_cast<unsigned>(req->content_len));
    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    ESP_LOGI(TAG, "Uploaded %s to SD card (%u bytes)", name, static_cast<unsigned>(req->content_len));
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

static const char* content_type_for_file(const char* name) {
    const char* extension = std::strrchr(name, '.');
    if (extension == nullptr) return "application/octet-stream";
    if (std::strcmp(extension, ".txt") == 0 || std::strcmp(extension, ".json") == 0) {
        return "text/plain; charset=utf-8";
    }
    if (std::strcmp(extension, ".mp3") == 0) return "audio/mpeg";
    if (std::strcmp(extension, ".wav") == 0) return "audio/wav";
    if (std::strcmp(extension, ".jpg") == 0 || std::strcmp(extension, ".jpeg") == 0) {
        return "image/jpeg";
    }
    return "application/octet-stream";
}

static esp_err_t file_handler(httpd_req_t *req) {
    char name[96] = {};
    if (!get_safe_upload_name(req, name, sizeof(name))) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid filename");
        return ESP_ERR_INVALID_ARG;
    }

    char path[128] = {};
    std::snprintf(path, sizeof(path), "/sdcard/%s", name);
    FILE* file = std::fopen(path, "rb");
    if (file == nullptr) {
        ESP_LOGW(TAG, "SD file not found: %s", path);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "file not found");
        return ESP_ERR_NOT_FOUND;
    }

    httpd_resp_set_type(req, content_type_for_file(name));
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment");

    std::vector<char> buffer(4096);
    while (true) {
        const size_t bytes_read = std::fread(buffer.data(), 1, buffer.size(), file);
        if (bytes_read == 0) {
            if (std::ferror(file)) {
                std::fclose(file);
                httpd_resp_send_chunk(req, nullptr, 0);
                return ESP_FAIL;
            }
            break;
        }
        if (httpd_resp_send_chunk(req, buffer.data(), bytes_read) != ESP_OK) {
            std::fclose(file);
            return ESP_FAIL;
        }
    }
    std::fclose(file);
    httpd_resp_send_chunk(req, nullptr, 0);
    ESP_LOGI(TAG, "SD file served: %s", name);
    return ESP_OK;
}

static esp_err_t files_handler(httpd_req_t *req) {
    DIR* directory = opendir("/sdcard");
    if (directory == nullptr) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "cannot open SD directory");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    if (httpd_resp_send_chunk(req, "{\"files\":[", HTTPD_RESP_USE_STRLEN) != ESP_OK) {
        closedir(directory);
        return ESP_FAIL;
    }

    bool first = true;
    struct dirent* entry = nullptr;
    char item[320] = {};
    while ((entry = readdir(directory)) != nullptr) {
        if (entry->d_name[0] == '.') continue;
        char path[280] = {};
        std::snprintf(path, sizeof(path), "/sdcard/%s", entry->d_name);
        struct stat file_stat = {};
        const long long size = stat(path, &file_stat) == 0 ? static_cast<long long>(file_stat.st_size) : 0;
        std::snprintf(item, sizeof(item), "%s{\"name\":\"%s\",\"size\":%lld}",
                      first ? "" : ",", entry->d_name, size);
        first = false;
        if (httpd_resp_send_chunk(req, item, HTTPD_RESP_USE_STRLEN) != ESP_OK) {
            closedir(directory);
            return ESP_FAIL;
        }
    }
    closedir(directory);
    httpd_resp_send_chunk(req, "]}", HTTPD_RESP_USE_STRLEN);
    return httpd_resp_send_chunk(req, nullptr, 0);
}

static esp_err_t delete_handler(httpd_req_t *req) {
    char name[96] = {};
    if (!get_safe_upload_name(req, name, sizeof(name))) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid filename");
        return ESP_ERR_INVALID_ARG;
    }

    char path[128] = {};
    std::snprintf(path, sizeof(path), "/sdcard/%s", name);
    if (std::remove(path) != 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "file not found or delete failed");
        return ESP_ERR_NOT_FOUND;
    }

    char response[128] = {};
    std::snprintf(response, sizeof(response), "Deleted %s", name);
    ESP_LOGI(TAG, "SD file deleted: %s", name);
    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

void RobotEyeNetworkSetFacePresent(bool present) {
    face_present.store(present, std::memory_order_relaxed);
}

static esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t *fb = camera.captureFrame();
    if (!fb) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "capture failed");
        return ESP_FAIL;
    }
    if (fb->format != PIXFORMAT_JPEG) {
        camera.releaseFrame(fb);
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
        camera.releaseFrame(fb);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "out of memory");
        return ESP_ERR_NO_MEM;
    }
    std::memcpy(jpeg_copy, fb->buf, jpeg_len);
    camera.releaseFrame(fb);

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
    config.recv_wait_timeout = 60;
    config.send_wait_timeout = 60;
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
    httpd_uri_t web_app = {};
    web_app.uri = "/";
    web_app.method = HTTP_GET;
    web_app.handler = web_app_handler;
    httpd_register_uri_handler(server, &web_app);
    httpd_uri_t upload = {};
    upload.uri = "/upload";
    upload.method = HTTP_POST;
    upload.handler = upload_handler;
    httpd_register_uri_handler(server, &upload);
    httpd_uri_t file = {};
    file.uri = "/file";
    file.method = HTTP_GET;
    file.handler = file_handler;
    httpd_register_uri_handler(server, &file);
    httpd_uri_t files = {};
    files.uri = "/files";
    files.method = HTTP_GET;
    files.handler = files_handler;
    httpd_register_uri_handler(server, &files);
    httpd_uri_t remove_file = {};
    remove_file.uri = "/delete";
    remove_file.method = HTTP_POST;
    remove_file.handler = delete_handler;
    httpd_register_uri_handler(server, &remove_file);
    ESP_LOGI(TAG, "Camera endpoint ready: http://roboteye.local/capture");
    ESP_LOGI(TAG, "SD upload page ready: http://roboteye.local/");
    ESP_LOGI(TAG, "SD file endpoint ready: http://roboteye.local/file?name=FILE");
    ESP_LOGI(TAG, "SD file list endpoint ready: http://roboteye.local/files");
    ESP_LOGI(TAG, "SD file delete endpoint ready: http://roboteye.local/delete?name=FILE");
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
