cmake_minimum_required(VERSION 3.9)
project (keypad)
find_package(32BLIT CONFIG REQUIRED $32BLIT_DIR)
blit_executable (keypad keypad.cpp)
blit_assets_yaml (keypad assets.yml)
blit_metadata (keypad metadata.yml)
