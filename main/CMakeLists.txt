idf_component_register(
    SRCS
        "RobotEye.cpp"
        "CameraEngine.cpp"
        "EyeEngine.cpp"
        "Graphics.cpp"
        "SH1106.cpp"
        "FaceTracker.cpp"
        "VisionEngine.cpp"
        "VisionTask.cpp"
        "ServoEngine.cpp"
        "RobotEyeNetwork.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        esp32-camera
        human_face_detect
        human_face_recognition
        esp_http_server
        esp_wifi
        esp_netif
        esp_event
        mdns
        nvs_flash
)
