cmake_minimum_required(VERSION 3.5)

project(Klive LANGUAGES CXX)

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
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    core/handler.cpp
    core/handler.h

    handlers/project/projecthandler.cpp
    handlers/project/projecthandler.h
    handlers/project/project.ui

    handlers/screen/screenhandler.cpp
    handlers/screen/screenhandler.h
    handlers/screen/screen.cpp
    handlers/screen/screen.h

    handlers/memory/memoryhandler.cpp
    handlers/memory/memoryhandler.h
    handlers/memory/memory.ui
    handlers/memory/memoryviewer.cpp
    handlers/memory/memoryviewer.h

    resources/klive.qrc
    ${TS_FILES}
)


target_link_libraries(Klive PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
