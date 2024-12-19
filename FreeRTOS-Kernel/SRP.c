#include "FreeRTOS.h"
#include "SRP.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "FreeRTOSConfig.h"


static UBaseType_t systemCeilingStack[configSRP_CEILING_DEPTH];
static int ceilingStackTop = 0; // Top of the stack
static UBaseType_t currentSystemCeiling = 0; // Current active system ceiling

typedef struct SRP_BinarySemaphore {
    SemaphoreHandle_t semaphore;     // Underlying FreeRTOS semaphore
    UBaseType_t priorityCeiling;     // Priority ceiling of this semaphore
    UBaseType_t maxPriority;         // Maximum priority of tasks that can take this semaphore
} SRP_BinarySemaphore_t;

UBaseType_t SRP_GetCurrentCeiling(void) {
    return currentSystemCeiling;
}

/* Function to create an SRP binary semaphore */
SRP_BinarySemaphore_t * xSRPCreateBinary(UBaseType_t maxPriority) {
    SRP_BinarySemaphore_t* srpSemaphore = pvPortMalloc(sizeof(SRP_BinarySemaphore_t));
    srpSemaphore->priorityCeiling = 0;
    srpSemaphore->maxPriority = maxPriority;
    srpSemaphore->semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(srpSemaphore->semaphore);
    return srpSemaphore;
}

BaseType_t xSRPSemaphoreGive(SRP_BinarySemaphore_t* xSemaphore) {
    SRP_BinarySemaphore_t* srpSemaphore = (SRP_BinarySemaphore_t*) xSemaphore;
    BaseType_t result = xSemaphoreGive(srpSemaphore->semaphore);
    if (result == pdTRUE) {
        srpSemaphore->priorityCeiling = 0;
    }
    return result;
    systemCeilingStack[++ceilingStackTop] = currentSystemCeiling;
    currentSystemCeiling = srpSemaphore->priorityCeiling;
}

BaseType_t xSRPSemaphoreTake(SRP_BinarySemaphore_t* xSemaphore) {
    SRP_BinarySemaphore_t* srpSemaphore = (SRP_BinarySemaphore_t*) xSemaphore;
    UBaseType_t currentPremptionLvl = uxTaskPremptionLvlGet(NULL);
    srpSemaphore->priorityCeiling = srpSemaphore->maxPriority;
    if(ceilingStackTop == 0) {
        currentSystemCeiling = 0;
    }
    currentSystemCeiling = systemCeilingStack[ceilingStackTop--];
    systemCeilingStack[ceilingStackTop++] = srpSemaphore->priorityCeiling;
    xSemaphoreTake(srpSemaphore->semaphore, portMAX_DELAY);
    return pdTRUE;
}
