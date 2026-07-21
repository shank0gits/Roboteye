idf_component_register(
    SRCS
        "RobotEye.cpp"
        "CameraEngine.cpp"
        "EyeEngine.cpp"
        "Graphics.cpp"
        "SH1106.cpp"
        "FaceTracker.cpp"
        "VisionEngine.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        esp32-camera
        human_face_detect
        human_face_recognition
)