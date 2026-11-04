cmake_minimum_required(VERSION 3.16)
project(control_systems_suite VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Older macOS support
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version")
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build Universal Binary")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)
find_package(Eigen3 REQUIRED NO_MODULE)

file(GLOB MODULE_SOURCES "src/*.cpp")
file(GLOB MODULE_HEADERS "include/*.h")

add_executable(control_systems_suite
    main.cpp
    ${MODULE_SOURCES}
    ${MODULE_HEADERS}
)

if(APPLE)
    set_target_properties(control_systems_suite PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.jadenfoote.controlsystemssuite"
        MACOSX_BUNDLE_BUNDLE_NAME "Control Systems Suite"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

target_include_directories(control_systems_suite PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(control_systems_suite PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Eigen3::Eigen
    )

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(control_systems_suite PRIVATE -O3 -Wall -Wextra)
endif()

# Only run on Linux (not when cross-compiling for Windows or building on macOS)
if(UNIX AND NOT APPLE)
    # 1. Also emit a non-PIE executable so double-clicking the raw binary in Dolphin never crashes it
    target_link_options(control_systems_suite PRIVATE -no-pie)

    # 2. Automatically generate ControlSystemsSuite.desktop with the exact current paths
    set(DESKTOP_FILE_CONTENT "[Desktop Entry]
Type=Application
Name=Control Systems Suite
Comment=Interactive Control Systems & DSP Engineering Suite
Exec=${CMAKE_CURRENT_BINARY_DIR}/control_systems_suite
Path=${CMAKE_CURRENT_BINARY_DIR}
Icon=utilities-system-monitor
Terminal=false
Categories=Science;Engineering;Education;
")

    # Write the .desktop file into both the project folder and the build/ folder
    file(GENERATE OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/ControlSystemsSuite.desktop" CONTENT "${DESKTOP_FILE_CONTENT}")
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ControlSystemsSuite.desktop" CONTENT "${DESKTOP_FILE_CONTENT}")

    # 3. After every build, mark the .desktop files executable and update the KDE App Menu entry
    add_custom_command(TARGET control_systems_suite POST_BUILD
        COMMAND chmod +x "${CMAKE_CURRENT_SOURCE_DIR}/ControlSystemsSuite.desktop"
        COMMAND chmod +x "${CMAKE_CURRENT_BINARY_DIR}/ControlSystemsSuite.desktop"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/.local/share/applications"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/ControlSystemsSuite.desktop"
            "$ENV{HOME}/.local/share/applications/ControlSystemsSuite.desktop"
        COMMENT "Updating ControlSystemsSuite.desktop launcher..."
    )
endif()
