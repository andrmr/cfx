cmake_minimum_required(VERSION 3.17)

project (demo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include(${CMAKE_CURRENT_LIST_DIR}/cfx/CMakeLists.txt)

add_executable(${PROJECT_NAME} demo.cpp ${cfx_source})
file(TO_CMAKE_PATH "${CMAKE_CURRENT_LIST_DIR}/res" RES_DIR)
target_compile_definitions(${PROJECT_NAME} PRIVATE RES_DIR="${RES_DIR}")

# SDL2 --
find_package(SDL2 REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(${PROJECT_NAME} SDL2::SDL2 -lsdl2_image "-mconsole")
# -- SDL2

# {fmt} --
find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} fmt::fmt)
# -- {fmt}

# spdlog --
find_package(spdlog REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE SPDLOG_FMT_EXTERNAL SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog)
# -- spdlog

# cereal --
find_package(cereal REQUIRED)
# -- cereal
