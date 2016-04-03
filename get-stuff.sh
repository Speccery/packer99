#!/bin/bash
# EP 2016-04-02
echo "Copying basic.obj and boot1.obj, producing rom3.bin"
cp ~/Dropbox/Omat/Retro/TMS9900/Stuart/Modified\ Cortex\ BASIC/modded-basic.obj basic.obj
cp ~/Dropbox/Omat/trunk/projects/bb95/DisSource.obj boot1.obj
./packer99 rom3.bin 32 boot1.obj -i basic.obj
# Now we've got a functional ROM with EVMBUG and BASIC in the image "rom3.bin".
# Next merge Forth into this image, by appending on top of 32K.
# The Forth image starts at 0x1600 in the binary.
dd if=stuart-forth.bin of=forth.bin skip=5632 bs=1

# Calculate amount of padding needed. Create padding.bin of that size.
# Include some python code as a here document.
python << eof-python
import os
import math
st = os.stat("forth.bin")
# print st.st_size
# We want to PAD forth.bin to next multiple of 4K.
pages = math.ceil(st.st_size/4096.0)
pages = int(pages)
print pages, "Pages needed for forth.bin"
padding = pages*4096 - st.st_size
padbyte=b'\xff'	# 255
with open("padding.bin","wb") as file:
	for i in range(0, padding):
		file.write(padbyte)
file.close()
eof-python

# Finally package the whole thing.
cat rom3.bin forth.bin padding.bin >  rom4.bin
echo "Created rom4.bin"
