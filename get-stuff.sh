#!/bin/bash
# EP 2016-04-02
echo "Copying basic.obj and boot1.obj, producing rom3.bin"
cp ~/Dropbox/Omat/Retro/TMS9900/Stuart/Modified\ Cortex\ BASIC/modded-basic.obj basic.obj
cp ~/Dropbox/Omat/trunk/projects/bb95/DisSource.obj boot1.obj
./packer99 rom3.bin 32 boot1.obj -i basic.obj
