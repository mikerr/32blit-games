
Setup
=====

PC build:
---------

cd build
cmake .. -D32BLIT_DIR="~/32blit-beta/"
make


32blit build:
-------------

cd build.stm32
cmake .. -D32BLIT_DIR="~/32blit-beta/" -DCMAKE_TOOLCHAIN_FILE="~/32blit-beta/32blit.toolchain"
make 
