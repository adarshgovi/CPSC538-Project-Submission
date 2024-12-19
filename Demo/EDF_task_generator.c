#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>
#include <stdio.h>
#include "EDF_task_generator.h"
#include <pico/stdio.h>
#include <stdio.h>
#include <pico/stdlib.h>


// /* Structure to hold EDF task parameters */
// typedef struct EDFTaskParameters
// {
//     TickType_t xExecutionTime; // Job execution time (in ticks)
//     uint32_t   uxGPIO;         // GPIO pin to toggle (for visual output)
// } EDFTaskParameters_t;

void vTaskSwitchedOutHook(void)
{
    UBaseType_t gpio = uxTaskGPIOGet();
    gpio_put(gpio, 0); // Turn off GPIO for the task
}

// Hook called when a task is switched in (optional)
void vTaskSwitchedInHook(void)
{
    UBaseType_t gpio = uxTaskGPIOGet();
    gpio_put(gpio, 1); // Turn off GPIO for the task
}

/* Generic EDF task function */
static void vGenericEDFTask(void *pvParameters)
{
    EDFTaskParameters_t *pxTaskParams = (EDFTaskParameters_t *)pvParameters;
    TickType_t xStartTime;

    for (;;)
    {
        const uint32_t NopPerTick = 25000; // value gotten from testing
        uint32_t NopPerTask = NopPerTick * pxTaskParams->xExecutionTime;
        xStartTime = xTaskGetTickCount();

        while (NopPerTask > 0)
        {
            NopPerTask--;
            __asm volatile("nop"); // Prevent compiler optimization
        }
        // printf("Task %s running, t: %u\n", pcTaskGetName(NULL), (unsigned int)xTaskGetTickCount());
        if (vTaskDelayUntilNextPeriod() == errDEADLINE_MISS){
            printf("Deadline missed for task %s\n", pcTaskGetName(NULL));
        }
    }
}

/* Function to create EDF tasks */
BaseType_t xCreateEDFTask(const char *pcTaskName,
                          TickType_t xExecutionTime,
                          TickType_t xPeriod,
                          TickType_t xDeadline,
                          UBaseType_t uxPriority,
                          uint32_t uxGPIO,
                          TaskHandle_t *pxTaskHandle)
{
    EDFTaskParameters_t *pxTaskParams;

    // Allocate memory for task parameters
    pxTaskParams = (EDFTaskParameters_t *)pvPortMalloc(sizeof(EDFTaskParameters_t));
    if (pxTaskParams == NULL)
    {
        return pdFAIL; // Memory allocation failed
    }
    pxTaskParams->xExecutionTime = xExecutionTime;
    BaseType_t createResult = xTaskCreateEDF_OLD(vGenericEDFTask,      // Task function
                       pcTaskName,           // Task name
                       configMINIMAL_STACK_SIZE, // Stack size
                       (void *)pxTaskParams, // Task parameters
                       uxPriority,           // Task priority
                       pxTaskHandle,        // Return the task handle
                       xPeriod, xDeadline, xExecutionTime);        // Return the task handle
    if (createResult == errMAX_UTILIZATION){
        printf("Max system utilization reached, task %s not created\n", pcTaskName);
        return createResult;
    }
    vTaskGPIOSet(*pxTaskHandle, uxGPIO); // Set the GPIO for the task
    return createResult;
}

/* Initialization function for EDF Task Generator */
void vInitializeEDFTaskGenerator(void)
{
    // Placeholder for any EDF generator-specific initialization if needed
}
