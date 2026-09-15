#ifndef VISION_TASK_H
#define VISION_TASK_H

#include "VisionEngine.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


class VisionTask
{
public:

    //---------------------------------------
    // Constructor
    //---------------------------------------

    VisionTask(
        VisionEngine& vision
    );


    //---------------------------------------
    // Start Vision Task
    //---------------------------------------

    bool begin();


private:

    //---------------------------------------
    // Vision Engine Reference
    //---------------------------------------

    VisionEngine& vision;


    //---------------------------------------
    // FreeRTOS Task
    //---------------------------------------

    static void taskEntry(
        void* parameter
    );

    void run();


    //---------------------------------------
    // Task Handle
    //---------------------------------------

    TaskHandle_t taskHandle = nullptr;
};

#endif