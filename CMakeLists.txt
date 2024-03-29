cmake_minimum_required(VERSION 3.5)

project(klive LANGUAGES CXX)

add_subdirectory(ext/Qt-Advanced-Docking-System)
add_compile_options(-Wall -g)

include_directories(ext/Qt-Advanced-Docking-System/src)
include_directories(core)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)
# set(TS_FILES resources/Klive_fr_FR.ts)

add_executable(klive
    ${ads_SRCS}
    core/main.cpp
    core/mainwindow.cpp
    core/mainwindow.h

    common/observer.h
    common/mapreader.h
    common/expr.h
    common/expr.cpp
    common/scopevar.h
    common/utils.cpp
    common/utils.h

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
    core/hardware/z80registers.cpp
    core/hardware/z80registers.h
    core/hardware/ioports.h
    core/hardware/checkpoints.cpp
    core/hardware/keyboard.cpp

    handlers/project/projecthandler.cpp
    handlers/project/projecthandler.h

    handlers/gadgets/console/console.h
    handlers/gadgets/console/console.cpp
    handlers/gadgets/z80/gens.cpp
    handlers/gadgets/z80/monsview.cpp
    handlers/gadgets/z80/monsview.h
    handlers/gadgets/z80/mons.cpp
    handlers/gadgets/z80/mons.h

    handlers/gadgets/gdb/minigdb.cpp
    handlers/gadgets/gdb/minigdb.h

    handlers/screen/screenhandler.cpp
    handlers/screen/screenhandler.h

    handlers/memory/memoryviewer.cpp
    handlers/memory/memoryviewer.h

    handlers/gadgets/zx/spectrum/SpectrumScreen.cpp
    handlers/gadgets/zx/spectrum/SpectrumScreen.h

    handlers/gadgets/clock/clock.cpp
    handlers/gadgets/clock/clock.h

    handlers/gadgets/lines/linebouncer.cpp
    handlers/gadgets/lines/linebouncer.h
    handlers/gadgets/lines/lines.cpp
    handlers/gadgets/lines/lines.h

    resources/klive.qrc
    ${TS_FILES}
)
# qt5_create_translation(QMLES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(klive PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(klive PRIVATE qtadvanceddocking)
