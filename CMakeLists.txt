cmake_minimum_required(VERSION 3.16)
project(indi_sitech)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

find_package(INDI QUIET)
find_package(Nova QUIET)
find_package(Threads REQUIRED)

# If INDI package not found, try to find it manually
if(NOT INDI_FOUND)
    find_path(INDI_INCLUDE_DIR basedevice.h
        PATHS /usr/include/libindi /usr/local/include/libindi /usr/include /usr/local/include
    )
    find_library(INDI_LIBRARIES indidriver
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu
    )
    
    if(INDI_INCLUDE_DIR AND INDI_LIBRARIES)
        set(INDI_FOUND TRUE)
        message(STATUS "Found INDI manually: ${INDI_LIBRARIES}")
        message(STATUS "INDI headers: ${INDI_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "INDI library not found")
    endif()
endif()

# If Nova package not found, try to find it manually
if(NOT Nova_FOUND)
    find_path(NOVA_INCLUDE_DIR libnova/libnova.h
        PATHS /usr/include /usr/local/include
    )
    find_library(NOVA_LIBRARIES nova
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu
    )
    
    if(NOVA_INCLUDE_DIR AND NOVA_LIBRARIES)
        set(NOVA_FOUND TRUE)
        set(NOVA_INCLUDE_DIRS ${NOVA_INCLUDE_DIR})
        message(STATUS "Found Nova manually: ${NOVA_LIBRARIES}")
    else()
        message(FATAL_ERROR "Nova library not found")
    endif()
endif()

include_directories(${INDI_INCLUDE_DIR})
include_directories(${NOVA_INCLUDE_DIRS})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Source files
set(SITECH_SOURCES
    sitech_mount.cpp
)

# Headers
set(SITECH_HEADERS
    sitech_mount.h
)

# Create the driver executable
add_executable(indi_sitech_mount ${SITECH_SOURCES})

# Link libraries
target_link_libraries(indi_sitech_mount 
    ${INDI_LIBRARIES}
    ${NOVA_LIBRARIES}
    Threads::Threads
)

# Install targets
install(TARGETS indi_sitech_mount RUNTIME DESTINATION bin)
install(FILES indi_sitech.xml DESTINATION share/indi)

# Install udev rules if on Linux
if(UNIX AND NOT APPLE)
    install(FILES 99-sitech.rules DESTINATION /lib/udev/rules.d OPTIONAL)
endif()

# Create package
set(CPACK_PACKAGE_NAME "indi-sitech")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "INDI Driver for SiTech Mounts")
set(CPACK_PACKAGE_VENDOR "INDI Community")
set(CPACK_GENERATOR "DEB;RPM")

include(CPack)
