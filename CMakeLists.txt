cmake_minimum_required(VERSION 3.21)

project(md2any
    VERSION 0.1.0
    DESCRIPTION "A Qt/QML desktop UI for Pandoc document conversion"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
)

qt_standard_project_setup(REQUIRES 6.2)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

add_library(md2any_core STATIC
    src/pandoc_runner.cpp
    src/settings_store.cpp
)

target_sources(md2any_core
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS src
        FILES
            src/conversion_result.h
            src/conversion_task.h
            src/pandoc_runner.h
            src/settings_store.h
)

target_include_directories(md2any_core
    PUBLIC
        src
)

target_link_libraries(md2any_core
    PUBLIC
        Qt6::Core
)

qt_add_executable(md2any
    src/app_controller.cpp
    src/main.cpp
)

target_sources(md2any
    PRIVATE
        src/app_controller.h
)

qt_add_qml_module(md2any
    URI Md2Any
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
)

target_link_libraries(md2any
    PRIVATE
        md2any_core
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)

include(CTest)

if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()
