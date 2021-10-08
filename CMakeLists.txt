cmake_minimum_required(VERSION 3.9)

# has to be before project
if(PICO_SDK_PATH)
	  include(${32BLIT_DIR}/32blit-pico/pico_sdk_import.cmake)
    endif()

project(32blit-games)

if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4244")
    endif()

    find_package (32BLIT CONFIG REQUIRED PATHS .)

set(PROJECT_DISTRIBS LICENSE README.md)

add_subdirectory (3dcube)
add_subdirectory (bitmapstuff)
add_subdirectory (carsprint)
add_subdirectory (dune2)
add_subdirectory (fracking)
add_subdirectory (isocars)
add_subdirectory (jetpac)
add_subdirectory (keypad)
add_subdirectory (knightlore)
add_subdirectory (magiceye)
add_subdirectory (missile)
add_subdirectory (sprite-browser)
add_subdirectory (starfield)
add_subdirectory (vector3d)
add_subdirectory (wakkawakka)
add_subdirectory (willy)
add_subdirectory (xmas)

# setup release packages
install (FILES ${PROJECT_DISTRIBS} DESTINATION .)
set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set (CPACK_GENERATOR "ZIP" "TGZ")
include (CPack)
