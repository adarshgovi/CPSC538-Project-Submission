# EDF Scheduler Implementation in FreeRTOS

This project extends FreeRTOS by implementing an **Earliest Deadline First (EDF)** scheduling algorithm. In this implementation, all EDF tasks are held at a single user-defined priority level. At each tick, the scheduler checks the global highest priority task available (may not be an EDF task). If there are no tasks at a higher priority than the EDF priority, our scheduler kicks in and checks the deadline of all tasks in the EDF_priority ready list. The task with the earliest deadline from this list is then executed.  

## Key Features

### Modifications to Task Control Block (TCB)
- **New Fields**:
  - `TaskDeadline`: The deadline for the task’s current job.
  - `TaskPeriod`: The period of the task.
  - `CurrentJobDeadline`: The absolute deadline of the current job.
  - `ActivationTime`: The activation time for the task.

### New EDF Task Creation Method
- A dedicated task creation method initializes tasks with EDF-specific parameters.
- Automatically manages task activation times and deadlines based on the task period.

### Admission Control
- Admission control ensures system utilization does not exceed 1 at task creation time. Tasks are admitted only if the system has sufficient capacity to schedule them.

### Deadline Monitoring
- Tasks can detect when deadlines are missed.
- Provides flexibility for users to define how to handle missed deadlines (e.g., logging, aborting, or reattempting the task).

### Demo Tasks
- Demo tasks with busy loops are included for testing the EDF scheduler.
- These tasks execute `NOP` operations on the Pico to simulate task workloads and ensure execution times meet specified parameters.

## Configuration and Usage

### Enabling the EDF Scheduler
- Ensure the configuration macro `configUSE_EDF_SCHEDULER` is enabled in your FreeRTOSConfig.h file:
  ```c
  #define configUSE_EDF_SCHEDULER 1
  #define configEDF_PRIORITY_LEVEL <Priority Level for all EDF Tasks>


### Creating EDF Tasks
EDF Task Creation Function

The `xTaskCreateEDF` function creates a task with EDF (Earliest Deadline First) scheduling. The scheduler uses the task's period and deadline to prioritize execution.
```c
    void xTaskCreateEDF(
        TaskFunction_t pxTaskCode,
        const char * const pcName,
        const uint16_t usStackDepth,
        void * const pvParameters,
        TaskHandle_t * const pxCreatedTask,
        const TickType_t xTaskPeriod,
        const TickType_t xTaskDeadline
    );
```

#### Parameters
- `pxTaskCode`:	Pointer to the task function to execute.
- `pcName`: A descriptive name for the task (useful for debugging and monitoring).
- `usStackDepth`: The size of the task's stack in words.
- `pvParameters`: Pointer to the parameters to be passed to the task function.
- `pxCreatedTask`: Pointer to the handle of the created task.
- `xTaskPeriod`: Period of the task (in ticks). Defines how often the task repeats.
- `xTaskDeadline`: Relative deadline (in ticks) for the task.

Below is an example demonstrating how to create a simple EDF-scheduled task:
``` c
void vUserTaskLogic(void *pvParameters)
{
    // Simulate some work using a busy loop
    const uint32_t NopPerTick = 25000; // Calibration value for testing
    uint32_t NopPerTask = NopPerTick * (uint32_t)pvParameters;

    while (NopPerTask > 0)
    {
        NopPerTask--;
        __asm volatile("nop"); // Prevent compiler optimization
    }

    printf("Task logic executed for task %s\n", pcTaskGetName(NULL));
}

int main(void)
{
    TaskHandle_t xTaskHandle;

    // Create an EDF task
    xTaskCreateEDF(
        vUserTaskLogic,          // Task function
        "EDF_Task",              // Task name
        configMINIMAL_STACK_SIZE,// Stack size
        (void *)20,              // Task parameters (e.g., execution time in ms)
        &xTaskHandle,            // Task handle
        pdMS_TO_TICKS(100),      // Task period: 100 ms
        pdMS_TO_TICKS(100)       // Task deadline: 100 ms
    );
    // Start the scheduler
    vTaskStartScheduler();
}
```

## Testing the EDF Scheduler
### Demo Tasks
Included demo tasks simulate real-time workloads with specified execution times, periods, and deadlines.
Use these tasks to validate EDF functionality:
- Task execution follows a periodic schedule.
- Deadlines are enforced.
- System utilization is checked. 

This is implemented in `Demo/EDF_task_generator.c`

# SRP (Stack Resource Policy) Additions to FreeRTOS

This project also starts to implement **Stack Resource Policy (SRP)** mechanisms into FreeRTOS for managing resource access in systems with priority inheritance requirements. These changes enable using Binary Semaphores with SRP in FreeRTOS. Files relevant to this part of the project are seen in `FreeRTOS-Kernel/SRP.c`

## Key Features

### System Ceiling Management
- **`systemCeilingStack`**: A stack to track active system ceilings, implemented as an array with a configurable depth (`configSRP_CEILING_DEPTH`).
- **`currentSystemCeiling`**: The currently active system ceiling.

### `SRP_BinarySemaphore_t` Structure
Defines a custom binary semaphore with SRP properties:
- **`semaphore`**: The underlying FreeRTOS semaphore.
- **`priorityCeiling`**: The current priority ceiling of the semaphore.
- **`maxPriority`**: The maximum priority of tasks that can take this semaphore.

### Functions

#### `UBaseType_t SRP_GetCurrentCeiling(void)`
- Returns the current system ceiling for use in scheduling and resource access decisions.

#### `SRP_BinarySemaphore_t * xSRPCreateBinary(UBaseType_t maxPriority)`
- Creates a binary semaphore with SRP properties.
- Parameters:
  - `maxPriority`: The maximum priority level of tasks allowed to take the semaphore.
- Returns a pointer to an `SRP_BinarySemaphore_t` instance.

#### `BaseType_t xSRPSemaphoreGive(SRP_BinarySemaphore_t* xSemaphore)`
- Releases an SRP semaphore.
- Updates the system ceiling after release.
- Parameters:
  - `xSemaphore`: Pointer to the SRP semaphore to release.
- Returns `pdTRUE` on success, `pdFALSE` otherwise.

#### `BaseType_t xSRPSemaphoreTake(SRP_BinarySemaphore_t* xSemaphore)`
- Acquires an SRP semaphore.
- Updates the system ceiling before acquisition.
- Parameters:
  - `xSemaphore`: Pointer to the SRP semaphore to acquire.
- Returns `pdTRUE` on success.

## Key Logic

### Semaphore Take
- Retrieves the current task's preemption level using `uxTaskPremptionLvlGet`.
- Updates the semaphore's priority ceiling to the maximum priority allowed.
- Adjusts the `systemCeilingStack` and `currentSystemCeiling` for SRP compliance.
- Blocks on the underlying FreeRTOS semaphore until it is available.

### Semaphore Give
- Resets the semaphore's priority ceiling to `0`.
- Restores the previous system ceiling from the `systemCeilingStack`.

### Benefits of SRP Integration
- Prevents **priority inversion** by ensuring tasks cannot access resources above the current ceiling.
- Simplifies system design by enforcing resource access rules through the kernel.
- Supports deterministic resource sharing in real-time systems.

## Configuration
- **`configSRP_CEILING_DEPTH`**: Configurable stack depth for tracking system ceilings. Define this in `FreeRTOSConfig.h` as per system requirements.

## Example Usage
```c
SRP_BinarySemaphore_t* mySemaphore = xSRPCreateBinary(3);

// Task attempting to acquire the semaphore
if (xSRPSemaphoreTake(mySemaphore) == pdTRUE) {
    // Critical section
    xSRPSemaphoreGive(mySemaphore);
}