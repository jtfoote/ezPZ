cmake_minimum_required(VERSION 3.16)
project(ezpz VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version")
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build Universal Binary")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)
find_package(Eigen3 REQUIRED NO_MODULE)

file(GLOB MODULE_SOURCES "src/*.cpp")
file(GLOB MODULE_HEADERS "include/*.h")

# IMPORTANT: WIN32 and MACOSX_BUNDLE must be present here!
add_executable(ezpz WIN32 MACOSX_BUNDLE
    main.cpp
    resources.qrc
    ${MODULE_SOURCES}
    ${MODULE_HEADERS}
)

if(APPLE)
    set_target_properties(ezpz PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.jadenfoote.ezpz"
        MACOSX_BUNDLE_BUNDLE_NAME "ezPZ"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

target_include_directories(ezpz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ezpz PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Eigen3::Eigen
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(ezpz PRIVATE -O3 -Wall -Wextra -Wno-unused-lambda-capture)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ezpz PRIVATE -O3 -Wall -Wextra)
elseif(MSVC)
    target_compile_options(ezpz PRIVATE /O2 /W3 /utf-8 /permissive-)
endif()

if(UNIX AND NOT APPLE)
    target_link_options(ezpz PRIVATE -no-pie)

    set(DESKTOP_FILE_CONTENT "[Desktop Entry]
Type=Application
Name=ezPZ
Comment=Interactive Control Systems & DSP Engineering Suite
Exec=${CMAKE_CURRENT_BINARY_DIR}/ezpz
Path=${CMAKE_CURRENT_BINARY_DIR}
Icon=$ENV{HOME}/.local/share/icons/hicolor/scalable/apps/ezpz.svg
Terminal=false
Categories=Science;Engineering;Education;
")

    file(GENERATE OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/ezPZ.desktop" CONTENT "${DESKTOP_FILE_CONTENT}")
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ezPZ.desktop" CONTENT "${DESKTOP_FILE_CONTENT}")

    add_custom_command(TARGET ezpz POST_BUILD
        COMMAND chmod +x "${CMAKE_CURRENT_SOURCE_DIR}/ezPZ.desktop"
        COMMAND chmod +x "${CMAKE_CURRENT_BINARY_DIR}/ezPZ.desktop"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/.local/share/applications"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/.local/share/icons/hicolor/scalable/apps"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/ezpz.svg"
            "$ENV{HOME}/.local/share/icons/hicolor/scalable/apps/ezpz.svg"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/ezPZ.desktop"
            "$ENV{HOME}/.local/share/applications/ezPZ.desktop"
        COMMENT "Updating ezPZ.desktop launcher and icon..."
    )
endif()