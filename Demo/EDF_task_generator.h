/* EDF_task_generator.h */
#ifndef EDF_TASK_GENERATOR_H
#define EDF_TASK_GENERATOR_H

#include "FreeRTOS.h"
#include "task.h"

/* Function to create EDF tasks */
BaseType_t xCreateEDFTask(const char *pcTaskName,
                          TickType_t xExecutionTime,
                          TickType_t xPeriod,
                          TickType_t xDeadline,
                          UBaseType_t uxPriority,
                          uint32_t uxGPIO,
                          TaskHandle_t *pxTaskHandle);

/* Initialization function for EDF Task Generator */
void vInitializeEDFTaskGenerator(void);


void vTaskSwitchedOutHook(void);
void vTaskSwitchedInHook(void);

#endif /* EDF_TASK_GENERATOR_H */
