CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
PROJECT(caveblitz)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Winit-self -Woverloaded-virtual -Wuninitialized -Wmissing-declarations -pedantic")

IF(WIN32)
    SET(WIN32_LIBS ws2_32 winmm)
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    SET(WIN32_LIBS "")
ENDIF()

SET(SDL2PP_WITH_IMAGE ON)
SET(SDL2PP_WITH_MIXER ON)
SET(SDL2PP_WITH_TTF ON)
SET(SDL2PP_STATIC OFF)
ADD_SUBDIRECTORY(extlibs/libSDL2pp)

INCLUDE(FindPkgConfig REQUIRED)
INCLUDE(FindOpenGL REQUIRED)

FIND_PACKAGE(PNG 1.6 REQUIRED)
PKG_SEARCH_MODULE(ENET REQUIRED libenet)

INCLUDE_DIRECTORIES(
    ${SDL2PP_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${ENET_INCLUDE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/extlibs/GSL/include
    ${CMAKE_CURRENT_LIST_DIR}/extlibs/glm
    ${CMAKE_CURRENT_LIST_DIR}/extlibs/rapidjson/include
    ${CMAKE_CURRENT_LIST_DIR}/extlibs/CLI11/include
)

# Common

SET(
    COMMON_SOURCE_FILES
    src/Common/DataTransfer.h
    src/Common/Image.cpp src/Common/Image.h
    src/Common/Map.cpp src/Common/Map.h
    src/Common/ProjectileType.h
    src/Common/ClientSentData.h
    src/Common/ServerSentData.h
)

ADD_LIBRARY(caveblitz-common ${COMMON_SOURCE_FILES})

TARGET_LINK_LIBRARIES(
    caveblitz-common
    ${PNG_LIBRARIES}
)

# Server base

SET(
    SERVER_LIB_SOURCE_FILES
    src/Server/Server.cpp src/Server/Server.h
    src/Server/MatchServer.cpp src/Server/MatchServer.h
    src/Server/ServerPlayer.cpp src/Server/ServerPlayer.h
    src/Server/ServerProjectile.cpp src/Server/ServerProjectile.h
    src/Server/ServerPickup.cpp src/Server/ServerPickup.h
    src/Server/JoinError.cpp src/Server/JoinError.h
    src/Server/Weapon.cpp src/Server/Weapon.h
    src/Server/GameMode.h
    src/Server/FreeForAllGameMode.cpp src/Server/FreeForAllGameMode.h
    src/Server/Match.h
)

ADD_LIBRARY(caveblitz-server-lib ${SERVER_LIB_SOURCE_FILES})
TARGET_LINK_LIBRARIES(
    caveblitz-server-lib
    ${ENET_LIBRARIES}
    ${WIN32_LIBS}
    caveblitz-common
)
TARGET_COMPILE_DEFINITIONS(
    caveblitz-server-lib PRIVATE
    GLM_ENABLE_EXPERIMENTAL=1
)

# Server

SET(
    SERVER_SOURCE_FILES
    src/server_main.cpp
)

ADD_EXECUTABLE(caveblitz-server ${SERVER_SOURCE_FILES})
TARGET_LINK_LIBRARIES(
    caveblitz-server
    caveblitz-common
    caveblitz-server-lib
)
TARGET_COMPILE_DEFINITIONS(
    caveblitz-server PRIVATE
    GLM_ENABLE_EXPERIMENTAL=1
)

# Client

SET(
    CLIENT_SOURCE_FILES
    src/main.cpp
    src/Client/Game.cpp src/Client/Game.h
    src/Client/MatchScene.cpp src/Client/MatchScene.h
    src/Client/ServerConnection.cpp src/Client/ServerConnection.h
    src/Client/Ship.cpp src/Client/Ship.h
    src/Client/FPSManager.cpp src/Client/FPSManager.h
    src/Client/Projectile.cpp src/Client/Projectile.h
    src/Client/Scene.h
    src/Client/Pickup.cpp src/Client/Pickup.h
    src/Client/Explosion.cpp src/Client/Explosion.h
    src/Client/MenuScene.cpp src/Client/MenuScene.h
)

ADD_EXECUTABLE(caveblitz ${CLIENT_SOURCE_FILES})

TARGET_LINK_LIBRARIES(
    caveblitz
    ${SDL2PP_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ENET_LIBRARIES}
    ${WIN32_LIBS}
    caveblitz-common
    caveblitz-server-lib
)

TARGET_COMPILE_DEFINITIONS(
    caveblitz PRIVATE
    GLM_ENABLE_EXPERIMENTAL=1
)

# Install

ADD_CUSTOM_TARGET(
    assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:caveblitz>/assets
)

SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
SET(INSTALL_TARGET caveblitz)
INSTALL(TARGETS caveblitz RUNTIME DESTINATION ${INSTALL_TARGET})
INSTALL(TARGETS caveblitz-server RUNTIME DESTINATION ${INSTALL_TARGET})
INSTALL(FILES $<TARGET_FILE_DIR:caveblitz>/assets DESTINATION ${INSTALL_TARGET})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/README.md DESTINATION ${INSTALL_TARGET})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/COPYING DESTINATION ${INSTALL_TARGET})

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
        ${LIB_COPY_PATH}/libenet-7.dll
        ${LIB_COPY_PATH}/SDL2_ttf.dll
        ${LIB_COPY_PATH}/libfreetype-6.dll
        ${LIB_COPY_PATH}/libharfbuzz-0.dll
        ${LIB_COPY_PATH}/libbz2-1.dll
        ${LIB_COPY_PATH}/libgcc_s_seh-1.dll
        ${LIB_COPY_PATH}/libglib-2.0-0.dll
        ${LIB_COPY_PATH}/libgraphite2.dll
        ${LIB_COPY_PATH}/libintl-8.dll
        ${LIB_COPY_PATH}/libpcre-1.dll
        ${LIB_COPY_PATH}/libiconv-2.dll
        ${LIB_COPY_PATH}/libstdc++-6.dll
        ${LIB_COPY_PATH}/libwinpthread-1.dll
        DESTINATION ${INSTALL_TARGET}
    )

    SET(CPACK_GENERATOR ZIP)
ELSE()
    SET(CPACK_GENERATOR TGZ)
ENDIF()

INCLUDE(CPack)
