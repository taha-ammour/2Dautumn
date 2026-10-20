cmake_minimum_required(VERSION 3.16)
project(AutumnHarvest C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2 SDL2_image SDL2_ttf SDL2_mixer)

file(GLOB GAME_SOURCES CONFIGURE_DEPENDS src/*.c)
add_executable(2Dautumn ${GAME_SOURCES})
target_include_directories(2Dautumn PRIVATE include include/headerfile)
target_link_libraries(2Dautumn PRIVATE PkgConfig::SDL2)
if(NOT MSVC)
	target_link_libraries(2Dautumn PRIVATE m)
	target_compile_options(2Dautumn PRIVATE -Wall -Wextra)
endif()

# the game loads res/ and fonts/ relative to the working directory
add_custom_command(TARGET 2Dautumn POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:2Dautumn>/res
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/fonts $<TARGET_FILE_DIR:2Dautumn>/fonts)
