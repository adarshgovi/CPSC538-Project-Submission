#ifndef INC_SRP_H
#define INC_SRP_H

#include "FreeRTOS.h"

typedef struct SRP_BinarySemaphore SRP_BinarySemaphoreHandle_t;
SRP_BinarySemaphoreHandle_t* xSRPCreateBinary(UBaseType_t priorityCeiling);
UBaseType_t SRP_GetCurrentCeiling(void);
BaseType_t xSRPSemaphoreGive(SRP_BinarySemaphoreHandle_t * xSemaphore);
BaseType_t xSRPSemaphoreTake(SRP_BinarySemaphoreHandle_t * xSemaphore);

#endif /* INC_SRP_H */