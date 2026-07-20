idf_component_register(
    SRCS
        "RobotEye.cpp"
        "CameraEngine.cpp"
        "EyeEngine.cpp"
        "Graphics.cpp"
        "SH1106.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        esp32-camera
)