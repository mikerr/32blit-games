# Basic parameters; check that these match your project / environment
cmake_minimum_required(VERSION 3.9)
project(32blit-games)

if(PICO_SDK_PATH)
  include(${32BLIT_DIR}/32blit-pico/pico_sdk_import.cmake)
  include(${32BLIT_DIR}/32blit-pico/pico_extras_import.cmake)
endif()
enable_language( C CXX ASM )

set(PROJECT_DISTRIBS LICENSE README.md)

# Build configuration; approach this with caution!
if(MSVC)
  add_compile_options("/W4" "/wd4244" "/wd4324" "/wd4458" "/wd4100")
else()
  add_compile_options("-Wall" "-Wextra" "-Wdouble-promotion" "-Wno-unused-parameter")
endif()

find_package (32BLIT CONFIG REQUIRED PATHS ../32blit-sdk $ENV{PATH_32BLIT_SDK})

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
