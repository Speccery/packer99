# Mini cortex rom builder script
# EP 2016-04-30
# I am using W27C512-45Z EEPROM
# There are two things to take note of:
# CPU A14 is connected to A15 of the ROM (pin 1)
# ROM A14 is treated as /WR and connected to 5V.
#
# Thus there are two things to note:
# 1. Obviously only 32K of the ROM is available.
# 2. Since A15 and A14 are swapped and A14 is tied to 5V, the memory space is 
#    going to be in two blocks. The ROM file has to be written accordingly.
#
# 0000..3FFF Unused (A14 tied to 1)
# 4000..7FFF The CPU sees this as 0000..3FFF
# 8000..BFFF Unused (A14 tied to 1)
# C000..FFFF The CPU sees this as 4000..7FFF

# Create 64K image, only low 32K in use (packer fills the rest with FF)
./packer99 minic1.bin 64 boot1.obj -i -t basic.obj
# Create 16K FF file
dd if=minic1.bin of=ff-16k.bin skip=32768 bs=1 count=16384
# Pick up the two 16K pages from the file
dd if=minic1.bin of=minic-lo16k.bin bs=1 count=16384
dd if=minic1.bin of=minic-hi16k.bin skip=16384 bs=1 count=16384
# Then merge the whole thing to a 64K ROM file.
cat ff-16k.bin minic-lo16k.bin ff-16k.bin minic-hi16k.bin > minic.bin
## ./packer99 rom7.bin 42 boot1.obj -i -t basic.obj -b -o 0x8000 forth.bin -a -o 0xa000 ../bb95/ledit99.out
