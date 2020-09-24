cmake_minimum_required(VERSION 3.5)

project(Klive LANGUAGES CXX)

add_subdirectory(ext/AdvancedDocking)
add_compile_options(-Wall)

include_directories(ext/AdvancedDocking/src)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES resources/Klive_fr_FR.ts)

add_executable(Klive
    ${ads_SRCS}
    main.cpp
    mainwindow.cpp
    mainwindow.h

    common/observer.h

    core/handler.cpp
    core/handler.h

    core/hardware/computer.cpp
    core/hardware/computer.h
    core/hardware/memory.cpp
    core/hardware/memory.h
    core/hardware/cpu.cpp
    core/hardware/cpu.h
    core/hardware/z80.cpp
    core/hardware/z80.h

    handlers/project/projecthandler.cpp
    handlers/project/projecthandler.h
    handlers/project/project.ui

    handlers/screen/screenhandler.cpp
    handlers/screen/screenhandler.h
    handlers/screen/screen.cpp
    handlers/screen/screen.h
    handlers/screen/linebouncer.cpp
    handlers/screen/linebouncer.h

    handlers/memory/memoryhandler.cpp
    handlers/memory/memoryhandler.h
    handlers/memory/memory.ui
    handlers/memory/memoryviewer.cpp
    handlers/memory/memoryviewer.h

    handlers/gadgets/clock/clock.cpp
    handlers/gadgets/clock/clock.h
    handlers/gadgets/zx/spectrum/SpectrumScreen.cpp
    handlers/gadgets/zx/spectrum/SpectrumScreen.h

    resources/klive.qrc
    ${TS_FILES}
)


target_link_libraries(Klive PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(Klive PRIVATE qtadvanceddocking)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
