# packer99
packer99 is a command line tool to merge multiple TMS9900 tagged hex files to one binary image.

At this point the tool only supports a handful of tagged file tags, more will be added as necessary.

Examples:
  ./packer99 rom.bin 32 boot.obj
  Creates a 32 kbyte binary rom file "rom.bin" from tagged hex file boot.obj. The ROM size is 32 kilobytes.
  
  ./packer99 rom.bin 32 boot.obj -o -32768 modded-basic.obj
  The same as above, but also merges the file "modded-basic.obj" to the same image.
  
