cmake_minimum_required(VERSION 3.16)

project(Quahag VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------
# ASAN option
# ------------------------------------------------------------------
option(ENABLE_ASAN "Build with AddressSanitizer" OFF)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g -O1)
    add_link_options(-fsanitize=address,undefined)
    message(STATUS "AddressSanitizer + UBSan enabled")
endif()

# ------------------------------------------------------------------
# Platform detection
# ------------------------------------------------------------------
if(NOT DEFINED CMAKE_PREFIX_PATH)
    # Default to system Qt6 on Linux
    set(CMAKE_PREFIX_PATH "/usr" CACHE PATH "Qt installation prefix")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# ------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------
set(PROJECT_SOURCES
    main.cpp
    core/gameloop.cpp
    core/gameloop.h
    core/inputstate.h
    core/tickcontext.h
    core/entitytypes.h
    entities/actoritem.cpp
    entities/actoritem.h
    entities/player.cpp
    entities/player.h
    entities/particle.cpp
    entities/particle.h
    entities/claweffect.cpp
    entities/claweffect.h
    entities/bullet.h
    entities/bullet.cpp
    entities/throne.h
    entities/throne.cpp
    entities/hachimi.h
    entities/hachimi.cpp
    entities/trailghost.h
    entities/trailghost.cpp
    entities/chutty.h
    entities/chutty.cpp
    entities/healcross.h
    entities/healcross.cpp
    entities/enemy.h
    entities/enemy.cpp
    graphics/camera2d.cpp
    graphics/camera2d.h
    graphics/parallaxbackground.cpp
    graphics/parallaxbackground.h
    graphics/tilelayeritem.cpp
    graphics/tilelayeritem.h
    ui/gameview.cpp
    ui/gameview.h
    world/gameworld.h
    world/gameworld.cpp
    world/gamescene.cpp
    world/gamescene.h
    world/levelbuilder.cpp
    world/levelbuilder.h
    world/tilemap.cpp
    world/tilemap.h
    resources.qrc
)

if(WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} resources.rc)
endif()

# ------------------------------------------------------------------
# Executable
# ------------------------------------------------------------------
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Quahag
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(Quahag ${PROJECT_SOURCES})
endif()

target_link_libraries(Quahag PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(Quahag PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ------------------------------------------------------------------
# Platform-specific properties
# ------------------------------------------------------------------
set_target_properties(Quahag PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# ------------------------------------------------------------------
# Install
# ------------------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS Quahag
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Quahag)
endif()
