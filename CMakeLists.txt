cmake_minimum_required(VERSION 3.20)

project(recoil_gun
    VERSION 1.0.0
    DESCRIPTION "Jogo 2D de duelo por recuo, ricochetes e power-ups"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(raylib CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)

if(TARGET raylib)
    set(RECOIL_RAYLIB_TARGET raylib)
elseif(TARGET raylib::raylib)
    set(RECOIL_RAYLIB_TARGET raylib::raylib)
else()
    message(FATAL_ERROR "A instalacao da raylib foi encontrada, mas nenhum target CMake conhecido foi exportado.")
endif()

if(TARGET box2d::box2d)
    set(RECOIL_BOX2D_TARGET box2d::box2d)
elseif(TARGET box2d)
    set(RECOIL_BOX2D_TARGET box2d)
else()
    message(FATAL_ERROR "A instalacao do Box2D foi encontrada, mas nenhum target CMake conhecido foi exportado.")
endif()

add_executable(recoil-gun
    src/main.c
    src/jogo.c
    src/assets.c
    src/fases.c
)

target_include_directories(recoil-gun PRIVATE include)

target_link_libraries(recoil-gun PRIVATE
    ${RECOIL_RAYLIB_TARGET}
    ${RECOIL_BOX2D_TARGET}
)

if(UNIX)
    target_link_libraries(recoil-gun PRIVATE m)
endif()

target_compile_options(recoil-gun PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Eu copio os assets para junto do executavel para ele funcionar tanto
# a partir da raiz do projeto quanto de dentro da pasta build.
add_custom_command(TARGET recoil-gun POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:recoil-gun>/assets"
)
