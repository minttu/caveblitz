cmake_minimum_required(VERSION 3.8)
project(caveblitz)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Winit-self -Woverloaded-virtual -Wuninitialized -Wmissing-declarations -pedantic")

SET(SDL2PP_WITH_IMAGE ON)
SET(SDL2PP_WITH_MIXER ON)
SET(SDL2PP_WITH_TTF ON)
ADD_SUBDIRECTORY(extlibs/libSDL2pp)

INCLUDE(FindPkgConfig)
INCLUDE(FindOpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLM REQUIRED)

include_directories(
        ${SDL2PP_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/extlibs/GSL/include
)
link_directories(${SDL2_LIBRARY_DIRS})

set(SOURCE_FILES src/main.cpp src/Server/Server.cpp src/Server/Server.h src/Server/ServerPlayer.cpp src/Server/ServerPlayer.h src/Client/Game.cpp src/Client/Game.h src/Client/Ship.cpp src/Client/Ship.h src/Client/FPSManager.cpp src/Client/FPSManager.h)
add_executable(caveblitz ${SOURCE_FILES})
target_link_libraries(
        caveblitz
        ${SDL2PP_LIBRARIES}
        ${OPENGL_LIBRARIES}
)

add_custom_command(
        TARGET caveblitz PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:caveblitz>/assets
)
