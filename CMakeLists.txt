cmake_minimum_required(VERSION 3.16)
project(ControlSystemsSuite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Gui Widgets Svg REQUIRED)
endif()

find_package(Eigen3 REQUIRED NO_MODULE)

add_executable(control_systems_suite
    main.cpp
    src/ComplexPlaneCanvas.cpp
    src/TimeResponseCanvas.cpp
    src/DesignParametersDialog.cpp
    src/RootLocusTab.cpp
    src/PIDTimeResponseTab.cpp
    src/ZPlaneFilterTab.cpp
    src/DiscreteTimeResponseTab.cpp
    src/ScientificCalculatorTab.cpp
    src/FormulaCheatSheetTab.cpp
    src/BlockDiagramCanvas.cpp
    src/BlockDiagramStudioTab.cpp
    src/BodeNyquistCanvas.cpp
    src/FrequencyResponseTab.cpp
    src/RouthHurwitzTab.cpp
    src/AppSettingsDialog.cpp
    src/UnifiedControlSuiteWindow.cpp
    include/CommonIncludes.h
    include/CoreAndWidgets.h
    include/ComplexPlaneCanvas.h
    include/TimeResponseCanvas.h
    include/DesignParametersDialog.h
    include/RootLocusTab.h
    include/PIDTimeResponseTab.h
    include/ZPlaneFilterTab.h
    include/DiscreteTimeResponseTab.h
    include/ScientificCalculatorTab.h
    include/FormulaCheatSheetTab.h
    include/BlockDiagramCanvas.h
    include/BlockDiagramStudioTab.h
    include/BodeNyquistCanvas.h
    include/FrequencyResponseTab.h
    include/RouthHurwitzTab.h
    include/AppSettingsDialog.h
    include/UnifiedControlSuiteWindow.h
)

target_include_directories(control_systems_suite PRIVATE include)

if(Qt6_FOUND)
    target_link_libraries(control_systems_suite PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Eigen3::Eigen
)
else()
    target_link_libraries(control_systems_suite PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Eigen3::Eigen)
endif()

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
