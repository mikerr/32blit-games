
Setup
=====

PC build:
---------

cd build
cmake .. -D32BLIT_DIR=~/32blit-sdk/
make


32blit build:
-------------

cd build.stm32
cmake .. -D32BLIT_DIR=~/32blit-sdk/ -DCMAKE_TOOLCHAIN_FILE=~/32blit-sdk/32blit.toolchain
make 

Picostation build:
-------------

cd build.pico
cmake .. -D32BLIT_DIR=~/32blit-sdk -DPICO_SDK_PATH=~/pico-sdk -DPICO_BOARD=pimoroni_picosystem
make 
