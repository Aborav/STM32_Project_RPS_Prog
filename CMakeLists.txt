cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32g431cbu6_dac_tl494)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

#####################################################################
# Add folders paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Libs/
    ${CMAKE_SOURCE_DIR}/Libs/MGL
    ${CMAKE_SOURCE_DIR}/Libs/MGL/st7735_driver
    ${CMAKE_SOURCE_DIR}/Libs/Encoder
    ${CMAKE_SOURCE_DIR}/Libs/INA226
)

# Inc files .c
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Src/ctrl.c 
    ${CMAKE_SOURCE_DIR}/Core/Src/dislpay.c
    ${CMAKE_SOURCE_DIR}/Core/Src/input.c
    ${CMAKE_SOURCE_DIR}/Core/Src/serv.c
    # Добавьте другие .c файлы из src если есть 
)

# Libs .c
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Libs/Encoder/M_ENC.c
    ${CMAKE_SOURCE_DIR}/Libs/FlashProm/M_Flashprom.c
    ${CMAKE_SOURCE_DIR}/Libs/INA226/M_INA226.c
    ${CMAKE_SOURCE_DIR}/Libs/MGL/MGL.c
    ${CMAKE_SOURCE_DIR}/Libs/MGL/st7735_driver/st7735.c
    ${CMAKE_SOURCE_DIR}/Libs/SignalFilter/M_Filter.c
)
#####################################################################


# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
