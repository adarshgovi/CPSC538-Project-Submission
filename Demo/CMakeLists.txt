cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
set(PICO_SDK_PATH "C:/Users/Adarsh/Desktop/Classes/CPSC 538G/freertos-pico/pico-sdk")
set(FREERTOS_KERNEL_PATH "C:/Users/Adarsh/Desktop/Classes/CPSC 538G/freertos-pico/FreeRTOS-Kernel")
include("C:/Users/Adarsh/Desktop/Classes/CPSC 538G/freertos-pico/pico-sdk/external/pico_sdk_import.cmake")

# Pull in FreeRTOS
include("C:/Users/Adarsh/Desktop/Classes/CPSC 538G/freertos-pico/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake")

project(app C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add source files
add_executable(blink 
    main.c
    EDF_task_generator.c  # Add the EDF task generator source file
)

# Include directories
target_include_directories(blink PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/EDF_task_generator  # Include for the header file
)

# Pull in common dependencies
target_link_libraries(blink 
    pico_stdlib 
    FreeRTOS-Kernel 
    FreeRTOS-Kernel-Heap4
)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(blink)