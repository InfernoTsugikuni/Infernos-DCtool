cmake_minimum_required(VERSION 3.16)
project(Infernos_DCTool VERSION 0.1 LANGUAGES CXX)

# Enable Qt6 automatic features
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Find Qt6 Network
find_package(Qt6 REQUIRED COMPONENTS Network)

# Automatically collect all source files
file(GLOB_RECURSE SOURCE_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Automatically collect all header files (for IDE visibility)
file(GLOB_RECURSE HEADER_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

# Automatically collect all Qt resource files
file(GLOB_RECURSE RESOURCE_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/*.qrc"
)

# Platform-specific icon setup
set(APP_ICON_RESOURCE "")
if(WIN32)
    # Create a Windows resource file for the icon
    set(ICON_RC "${CMAKE_CURRENT_BINARY_DIR}/icon.rc")
    file(WRITE ${ICON_RC} "IDI_ICON1 ICON DISCARDABLE \"${CMAKE_CURRENT_SOURCE_DIR}/resources/images/icon.ico\"\n")
    set(APP_ICON_RESOURCE ${ICON_RC})
endif()

# Define the executable with sources, headers (IDE), resources, and icon
qt_add_executable(Infernos_DCTool
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${RESOURCE_FILES}
    ${APP_ICON_RESOURCE}
)

# Include directories for compiler search path
target_include_directories(Infernos_DCTool
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Link against Qt6 Widgets
target_link_libraries(Infernos_DCTool
    PRIVATE Qt6::Widgets
)

# Link against Qt6 Network
target_link_libraries(Infernos_DCTool
    PRIVATE Qt6::Network
)

# Application bundle/executable properties
set_target_properties(Infernos_DCTool PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Finalize for Qt6 (handles remaining Qt-specific setup)
qt_finalize_executable(Infernos_DCTool)
