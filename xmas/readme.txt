
Setup
=====

PC build:
---------

cd build
cmake .. -D32BLITPATH="~/32blit-beta/"
make
./xmas


32blit build:
-------------

cd build.stm32
cmake .. -D32BLITPATH="~/32blit-beta/" -DCMAKE_TOOLCHAIN_FILE="~/32blit-beta/32blit.toolchain"
make 
