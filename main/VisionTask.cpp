#include "VisionTask.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"


static const char* TAG = "VisionTask";


//----------------------------------------------------
// Vision Task Configuration
//----------------------------------------------------

// Face detection processing interval
// 100 ms = maximum ~10 detection cycles per second
#define VISION_TASK_DELAY_MS    100

// Task stack size
#define VISION_TASK_STACK_SIZE  8192

// Task priority
#define VISION_TASK_PRIORITY    5

// Run vision processing on Core 1
#define VISION_TASK_CORE        1


//----------------------------------------------------
// Constructor
//----------------------------------------------------

VisionTask::VisionTask(
    VisionEngine& visionRef
)
    : vision(visionRef)
{
    taskHandle = nullptr;
}


//----------------------------------------------------
// Start Vision Task
//----------------------------------------------------

bool VisionTask::begin()
{
    //-----------------------------------------------
    // Prevent Duplicate Task
    //-----------------------------------------------

    if (taskHandle != nullptr)
    {
        ESP_LOGW(
            TAG,
            "Vision Task already running"
        );

        return false;
    }


    //-----------------------------------------------
    // Check Vision Engine
    //-----------------------------------------------

    if (!vision.isReady())
    {
        ESP_LOGE(
            TAG,
            "Vision Engine is not ready"
        );

        return false;
    }


    //-----------------------------------------------
    // Create FreeRTOS Task
    //-----------------------------------------------

    BaseType_t result =
        xTaskCreatePinnedToCore(
            taskEntry,
            "VisionTask",
            VISION_TASK_STACK_SIZE,
            this,
            VISION_TASK_PRIORITY,
            &taskHandle,
            VISION_TASK_CORE
        );


    //-----------------------------------------------
    // Check Task Creation
    //-----------------------------------------------

    if (result != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "Failed to create Vision Task"
        );

        taskHandle = nullptr;

        return false;
    }


    //-----------------------------------------------
    // Success
    //-----------------------------------------------

    ESP_LOGI(
        TAG,
        "Vision Task Started"
    );

    ESP_LOGI(
        TAG,
        "Vision Interval: %d ms",
        VISION_TASK_DELAY_MS
    );

    return true;
}


//----------------------------------------------------
// Task Entry
//----------------------------------------------------

void VisionTask::taskEntry(
    void* parameter
)
{
    //-----------------------------------------------
    // Convert Parameter
    //-----------------------------------------------

    VisionTask* self =
        static_cast<VisionTask*>(
            parameter
        );


    //-----------------------------------------------
    // Validate Parameter
    //-----------------------------------------------

    if (self == nullptr)
    {
        ESP_LOGE(
            TAG,
            "Invalid Vision Task parameter"
        );

        vTaskDelete(nullptr);

        return;
    }


    //-----------------------------------------------
    // Run Vision Processing
    //-----------------------------------------------

    self->run();


    //-----------------------------------------------
    // Task Should Normally Never Reach Here
    //-----------------------------------------------

    self->taskHandle = nullptr;


    //-----------------------------------------------
    // Delete Task
    //-----------------------------------------------

    vTaskDelete(nullptr);
}


//----------------------------------------------------
// Vision Processing Loop
//----------------------------------------------------

void VisionTask::run()
{
    ESP_LOGI(
        TAG,
        "Vision Processing Started"
    );


    //-----------------------------------------------
    // Continuous Vision Processing
    //-----------------------------------------------

    while (true)
    {
        //-------------------------------------------
        // Camera Capture
        // JPEG Decode
        // Face Detection
        //-------------------------------------------

        vision.update();


        //-------------------------------------------
        // Allow Other Tasks To Run
        //-------------------------------------------
        //
        // This delay limits face detection to
        // approximately 10 processing cycles/sec.
        //
        // It also gives the camera and other
        // FreeRTOS tasks CPU time.
        //-------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(
                VISION_TASK_DELAY_MS
            )
        );
    }
}