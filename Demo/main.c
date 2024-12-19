#include "FreeRTOSConfig.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "EDF_task_generator.h"
#include "SRP.h"
#include <pico/stdio.h>
#include <pico/time.h>
#include <stdio.h>


// Debounce period in milliseconds
#define DEBOUNCE_PERIOD_MS 500

// Timestamp of the last button press
volatile TickType_t xLastButtonPressTime = 0;
volatile uint8_t currentGPIO = 7;

#define RED_LED 17
#define GREEN_LED 16
#define YELLOW_LED 15
#define BLUE_LED 14
#define T1_GPIO 4
#define T2_GPIO 5
#define T3_GPIO 6
#define T4_GPIO 7
#define T5_GPIO 8
#define T6_GPIO 9
#define T7_GPIO 10
#define T8_GPIO 11
#define T9_GPIO 12
#define T10_GPIO 13 
#define BUTTON 18
// create semaphores for SRP tests
SRP_BinarySemaphoreHandle_t *SRPSem1;
SRP_BinarySemaphoreHandle_t *SRPSem2;   
SRP_BinarySemaphoreHandle_t *SRPSem3;

void button_callback(uint gpio, uint32_t events)
{
    TickType_t xCurrentTime = xTaskGetTickCountFromISR();
    if ((xCurrentTime - xLastButtonPressTime) < pdMS_TO_TICKS(DEBOUNCE_PERIOD_MS))
    {
        return;
    }
    TaskHandle_t tempHandle;
    xLastButtonPressTime = xCurrentTime;
    static int taskCounter = 0;
    char taskName[20];
    snprintf(taskName, 20, "B_EDF_T_%d", currentGPIO-7);
    if (xCreateEDFTask(taskName,
                       pdMS_TO_TICKS(10),  // Execution time: 20 ms
                       pdMS_TO_TICKS(100), // Period: 100 ms
                       pdMS_TO_TICKS(100), // Deadline: 100 ms
                       configEDF_PRIORITY_LEVEL,                  // Priority
                      currentGPIO++,      // GPIO for task visualization
                       &tempHandle) == pdPASS)
    {
        // get something from the task handle
        UBaseType_t priority = uxTaskPriorityGet(tempHandle);
        printf("Task %s created successfully!, priority %d \n", taskName, priority);
    }
    else
    {
        printf("Failed to create task %s (admission control failed).\n", taskName);
    }
}

void init_task_gpios()
{
    gpio_init(RED_LED);
    gpio_init(GREEN_LED);
    gpio_init(YELLOW_LED);
    gpio_init(BLUE_LED);
    gpio_init(T1_GPIO);
    gpio_init(T2_GPIO);
    gpio_init(T3_GPIO);
    gpio_init(T4_GPIO);
    gpio_init(T5_GPIO);
    gpio_init(T6_GPIO);
    gpio_init(T7_GPIO);
    gpio_init(T8_GPIO);
    gpio_init(T9_GPIO);
    gpio_init(T10_GPIO);
    gpio_init(BUTTON);
    // gpio_init(T11_GPIO);


    gpio_set_dir(RED_LED, GPIO_OUT);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_set_dir(YELLOW_LED, GPIO_OUT);
    gpio_set_dir(BLUE_LED, GPIO_OUT);
    gpio_set_dir(T1_GPIO, GPIO_OUT);
    gpio_set_dir(T2_GPIO, GPIO_OUT);
    gpio_set_dir(T3_GPIO, GPIO_OUT);
    gpio_set_dir(T4_GPIO, GPIO_OUT);
    gpio_set_dir(T5_GPIO, GPIO_OUT);
    gpio_set_dir(T6_GPIO, GPIO_OUT);
    gpio_set_dir(T7_GPIO, GPIO_OUT);
    gpio_set_dir(T8_GPIO, GPIO_OUT);
    gpio_set_dir(T9_GPIO, GPIO_OUT);
    gpio_set_dir(T10_GPIO, GPIO_OUT);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_down(BUTTON); // Enable pull-down resistor (active-high button)
    // gpio_set_dir(T11_GPIO, GPIO_OUT);
}

void vTaskBusyWork(TickType_t ticks)
{
    TickType_t xTimeToWake = xTaskGetTickCount() + ticks;
    while (xTaskGetTickCount() < xTimeToWake)
    {
        __asm volatile("nop"); // Prevent compiler optimization
    }

}

void vTestSRP1(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1 second period

    // Initialize the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // printf("T1 running, t: %u\n", (unsigned int)xTaskGetTickCount());

        // acquire red semaphore
        xSRPSemaphoreTake(SRPSem1);
        gpio_put(RED_LED, 1);
        vTaskBusyWork(100);

        // acquire green semaphore
        xSRPSemaphoreTake(SRPSem3);
        gpio_put(GREEN_LED, 1);
        vTaskBusyWork(100);
        gpio_put(GREEN_LED, 0);
        // release green semaphore
        xSRPSemaphoreGive(SRPSem3);

        vTaskBusyWork(100);
        gpio_put(RED_LED, 0);
        xSRPSemaphoreGive(SRPSem1);

        vTaskDelayUntilNextPeriod();
    }
}

void vTestSRP2(void *pvParameters)
{
    for (;;)
    {
        // printf("T2 running, t: %u\n", (unsigned int)xTaskGetTickCount());\

        xSRPSemaphoreTake(SRPSem1); 
        gpio_put(RED_LED, 1);
        vTaskBusyWork(100);

        // acquire yellow semaphore
        xSRPSemaphoreTake(SRPSem2);
        gpio_put(YELLOW_LED, 1);
        vTaskDelay(100);
        gpio_put(YELLOW_LED, 0);
        xSRPSemaphoreGive(SRPSem2);
        // release yellow semaphore

        vTaskDelay(100);
        gpio_put(RED_LED, 0);
        xSRPSemaphoreGive(SRPSem1);
        // release red semaphore

        vTaskBusyWork(1000);

        // acquire green semaphore
        xSRPSemaphoreTake(SRPSem3);
        gpio_put(GREEN_LED, 1);
        vTaskDelay(100);
        gpio_put(GREEN_LED, 0);
        xSRPSemaphoreGive(SRPSem3);

        vTaskDelayUntilNextPeriod();
    }
}

void vTestSRP3(void *pvParameters)
{
    for (;;)
    {
        // Print a message to indicate the task is running.
        // printf("T3 running, t: %u\n", (unsigned int)xTaskGetTickCount());


        // acquire yellow semaphore
        xSRPSemaphoreTake(SRPSem2);
        gpio_put(YELLOW_LED, 1);
        vTaskDelay(100);

        // acquire green semaphore
        xSRPSemaphoreTake(SRPSem3);
        gpio_put(GREEN_LED, 1);
        vTaskDelay(100);
        gpio_put(GREEN_LED, 0);
        xSRPSemaphoreGive(SRPSem3);
        // release green semaphore
        
        vTaskDelay(100);
        gpio_put(YELLOW_LED, 0);
        vTaskDelay(100);
        xSRPSemaphoreGive(SRPSem2);
        // release yellow semaphore

        vTaskBusyWork(100);
        xSRPSemaphoreTake(SRPSem1);
        gpio_put(RED_LED, 1);
        vTaskBusyWork(100);
        gpio_put(RED_LED, 0);
        xSRPSemaphoreGive(SRPSem1);
        vTaskBusyWork(100);

        vTaskDelayUntilNextPeriod();
    }
}

void main() {
    init_task_gpios();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    stdio_init_all();

    // Set up an interrupt for the button press (rising edge)
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_RISE, true, &button_callback);
    printf("Test UART Msg \n");
    // create pointers to the tasksh
    TaskHandle_t xTaskHandle1;
    TaskHandle_t xTaskHandle2;
    TaskHandle_t xTaskHandle3;
    int test_num = 4;

    # if (configENABLE_EDF_TEST == 1)
    switch(test_num){
        case 0:
            xCreateEDFTask("EDF_Test_Task1", 
                            pdMS_TO_TICKS(20), // Execution time: 20 ms
                            pdMS_TO_TICKS(99), // Period: 50 ms
                            pdMS_TO_TICKS(33), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T1_GPIO,           // GPIO pin for visual output
                            &xTaskHandle1);
            xCreateEDFTask("EDF_Test_Task2", 
                            pdMS_TO_TICKS(20), // Execution time: 20 ms
                            pdMS_TO_TICKS(99), // Period: 50 ms
                            pdMS_TO_TICKS(66), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T2_GPIO,           // GPIO pin for visual output
                            &xTaskHandle2);
            xCreateEDFTask("EDF_Test_Task3",
                                pdMS_TO_TICKS(20), // Execution time: 20 ms
                                pdMS_TO_TICKS(99), // Period: 50 ms
                                pdMS_TO_TICKS(99), // Deadline: 50 ms
                                configEDF_PRIORITY_LEVEL,                 // Task priority
                                T3_GPIO,           // GPIO pin for visual output
                                &xTaskHandle3);
            break;
        case 1:
            xCreateEDFTask("EDF_Test_Task1", 
                            pdMS_TO_TICKS(20), // Execution time: 20 ms
                            pdMS_TO_TICKS(50), // Period: 50 ms
                            pdMS_TO_TICKS(50), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T1_GPIO,           // GPIO pin for visual output
                            &xTaskHandle1);
            xCreateEDFTask("EDF_Test_Task2", 
                            pdMS_TO_TICKS(20), // Execution time: 20 ms
                            pdMS_TO_TICKS(75), // Period: 50 ms
                            pdMS_TO_TICKS(75), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T2_GPIO,           // GPIO pin for visual output
                            &xTaskHandle2);
            xCreateEDFTask("EDF_Test_Task3",
                                pdMS_TO_TICKS(20), // Execution time: 20 ms
                                pdMS_TO_TICKS(100), // Period: 50 ms
                                pdMS_TO_TICKS(100), // Deadline: 50 ms
                                configEDF_PRIORITY_LEVEL,                 // Task priority
                                T3_GPIO,           // GPIO pin for visual output
                                &xTaskHandle3);

            break;

        case 2:
            xCreateEDFTask("EDF_Test_Task1", 
                            pdMS_TO_TICKS(30), // Execution time: 20 ms
                            pdMS_TO_TICKS(99), // Period: 50 ms
                            pdMS_TO_TICKS(99), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T1_GPIO,           // GPIO pin for visual output
                            &xTaskHandle1);
            xCreateEDFTask("EDF_Test_Task2", 
                            pdMS_TO_TICKS(30), // Execution time: 20 ms
                            pdMS_TO_TICKS(99), // Period: 50 ms
                            pdMS_TO_TICKS(99), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T2_GPIO,           // GPIO pin for visual output
                            &xTaskHandle2);
            xCreateEDFTask("EDF_Test_Task3",
                                pdMS_TO_TICKS(30), // Execution time: 20 ms
                                pdMS_TO_TICKS(99), // Period: 50 ms
                                pdMS_TO_TICKS(99), // Deadline: 50 ms
                                configEDF_PRIORITY_LEVEL,                 // Task priority
                                T3_GPIO,           // GPIO pin for visual output
                                &xTaskHandle3);

            break;    

        case 3:
            xCreateEDFTask("EDF_Test_Task1", 
                            pdMS_TO_TICKS(15), // Execution time: 20 ms
                            pdMS_TO_TICKS(100), // Period: 50 ms
                            pdMS_TO_TICKS(30), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T1_GPIO,           // GPIO pin for visual output
                            &xTaskHandle1);
            xCreateEDFTask("EDF_Test_Task2", 
                            pdMS_TO_TICKS(15), // Execution time: 20 ms
                            pdMS_TO_TICKS(100), // Period: 50 ms
                            pdMS_TO_TICKS(45), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T2_GPIO,           // GPIO pin for visual output
                            &xTaskHandle2);
            xCreateEDFTask("EDF_Test_Task3",
                                pdMS_TO_TICKS(15), // Execution time: 20 ms
                                pdMS_TO_TICKS(100), // Period: 50 ms
                                pdMS_TO_TICKS(60), // Deadline: 50 ms
                                configEDF_PRIORITY_LEVEL,                 // Task priority
                                T3_GPIO,           // GPIO pin for visual output
                                &xTaskHandle3);

            break;            
        case 4:
            xCreateEDFTask("EDF_Test_Task1", 
                            pdMS_TO_TICKS(33), // Execution time: 20 ms
                            pdMS_TO_TICKS(99), // Period: 50 ms
                            pdMS_TO_TICKS(99), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T1_GPIO,           // GPIO pin for visual output
                            &xTaskHandle1);
            xCreateEDFTask("EDF_Test_Task2", 
                            pdMS_TO_TICKS(33), // Execution time: 20 ms
                            pdMS_TO_TICKS(99), // Period: 50 ms
                            pdMS_TO_TICKS(99), // Deadline: 50 ms
                            configEDF_PRIORITY_LEVEL,                 // Task priority
                            T2_GPIO,           // GPIO pin for visual output
                            &xTaskHandle2);
            xCreateEDFTask("EDF_Test_Task3",
                                pdMS_TO_TICKS(33), // Execution time: 20 ms
                                pdMS_TO_TICKS(99), // Period: 50 ms
                                pdMS_TO_TICKS(99), // Deadline: 50 ms
                                configEDF_PRIORITY_LEVEL,                 // Task priority
                                T3_GPIO,           // GPIO pin for visual output
                                &xTaskHandle3);

            break;            

        default:
            break;


    }

    


    # elif (configENABLE_SRP_TEST == 1) /* configENABLE_EDF_TEST B*/
        SRPSem1 = xSRPCreateBinary(3);
        SRPSem2 = xSRPCreateBinary(2);
        SRPSem3 = xSRPCreateBinary(3);

        xTaskCreateEDF_OLD(vTestSRP1, "SRP_T1", configMINIMAL_STACK_SIZE, 
        NULL, configEDF_PRIORITY_LEVEL, &xTaskHandle1, pdMS_TO_TICKS(2000), 
        pdMS_TO_TICKS(5000), 0);
        xTaskCreateEDF_OLD(vTestSRP2, "SRP_T2", configMINIMAL_STACK_SIZE, 
        NULL, configEDF_PRIORITY_LEVEL, &xTaskHandle2, pdMS_TO_TICKS(3000), 
        pdMS_TO_TICKS(5000), 0);
        xTaskCreateEDF_OLD(vTestSRP3, "SRP_T3", configMINIMAL_STACK_SIZE, 
        NULL, configEDF_PRIORITY_LEVEL, &xTaskHandle3, pdMS_TO_TICKS(4000), 
        pdMS_TO_TICKS(5000), 0);
        vTaskGPIOSet(xTaskHandle1, T1_GPIO);
        vTaskGPIOSet(xTaskHandle2, T2_GPIO);
        vTaskGPIOSet(xTaskHandle3, T3_GPIO);


        #if configENABLE_SRP == 1
            vTaskPremptionSet(xTaskHandle1, 3);
            vTaskPremptionSet(xTaskHandle2, 2);
            vTaskPremptionSet(xTaskHandle3, 1);
        #endif
        
        printf("%s P Lvl %d GPIO %d \n", pcTaskGetName(xTaskHandle1), uxTaskPremptionLvlGet(xTaskHandle1), uxTaskGPIOGet(xTaskHandle1));
        printf("%s P Lvl %d GPIO %d \n ", pcTaskGetName(xTaskHandle2), uxTaskPremptionLvlGet(xTaskHandle2), uxTaskGPIOGet(xTaskHandle2));
        printf("%s P Lvl %d GPIO %d \n", pcTaskGetName(xTaskHandle3), uxTaskPremptionLvlGet(xTaskHandle3), uxTaskGPIOGet(xTaskHandle3));
    #endif /* configENABLE_EDF_TEST */
// 
    vTaskStartScheduler();

}
