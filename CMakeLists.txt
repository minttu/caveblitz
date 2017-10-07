CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
PROJECT(caveblitz)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Winit-self -Woverloaded-virtual -Wuninitialized -Wmissing-declarations -pedantic")

IF(WIN32)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
ENDIF()

SET(SDL2PP_WITH_IMAGE ON)
SET(SDL2PP_WITH_MIXER ON)
SET(SDL2PP_WITH_TTF ON)
SET(SDL2PP_STATIC OFF)
ADD_SUBDIRECTORY(extlibs/libSDL2pp)

INCLUDE(FindPkgConfig)
INCLUDE(FindOpenGL REQUIRED)

FIND_PACKAGE(PNG 1.6 REQUIRED)

INCLUDE_DIRECTORIES(
    ${SDL2PP_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/extlibs/GSL/include
    ${CMAKE_CURRENT_LIST_DIR}/extlibs/glm
)

SET(
    SOURCE_FILES
    src/main.cpp
    src/Server/Server.cpp src/Server/Server.h
    src/Server/ServerPlayer.cpp src/Server/ServerPlayer.h
    src/Server/ServerProjectile.cpp src/Server/ServerProjectile.h
    src/Client/Game.cpp src/Client/Game.h
    src/Client/MatchScene.cpp src/Client/MatchScene.h
    src/Client/Ship.cpp src/Client/Ship.h
    src/Client/FPSManager.cpp src/Client/FPSManager.h
    src/Client/Projectile.cpp src/Client/Projectile.h
    src/Common/DataTransfer.h
    src/Common/Image.cpp src/Common/Image.h
)

ADD_EXECUTABLE(caveblitz ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(
    caveblitz
    ${SDL2PP_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${PNG_LIBRARIES}
)

TARGET_COMPILE_DEFINITIONS(
    caveblitz PRIVATE
    GLM_ENABLE_EXPERIMENTAL=1
)

ADD_CUSTOM_TARGET(
    TARGET caveblitz PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:caveblitz>/assets
)

INSTALL(TARGETS caveblitz RUNTIME DESTINATION /)
INSTALL(FILES $<TARGET_FILE_DIR:caveblitz>/assets DESTINATION /)

IF(WIN32)
    SET(LIB_COPY_PATH c:/msys64/mingw64/bin)
    INSTALL(
        FILES
        ${LIB_COPY_PATH}/SDL2.dll
        ${LIB_COPY_PATH}/SDL2_image.dll
        ${LIB_COPY_PATH}/libpng16-16.dll
        ${LIB_COPY_PATH}/libjpeg-8.dll
        ${LIB_COPY_PATH}/libtiff-5.dll
        ${LIB_COPY_PATH}/libwebp-7.dll
        ${LIB_COPY_PATH}/zlib1.dll
        ${LIB_COPY_PATH}/liblzma-5.dll
        DESTINATION /
    )

    SET(CPACK_GENERATOR ZIP)
ELSE()
    SET(CPACK_GENERATOR TGZ)
ENDIF()

INCLUDE(CPack)
