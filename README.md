# packer99
packer99 is a command line tool to merge multiple TMS9900 tagged hex files to one binary image. It is intended initially to build simple boot ROMs for my TMS9995 breadboard project. The board has a memory paging unit implemented on an FPGA, and the ROM size in this example case is 256 kbytes. The tool supports per tagfile offsets, allowing many files to be merged and located on page boundaries as necessary.

At this point the tool only supports a handful of the hex file tags, more will be added as necessary.
This program is still fresh out of the oven and probably contains some bugs.

Examples:
`./packer99 rom.bin 32 boot.obj`
Creates a 32 kbyte binary rom file "rom.bin" from tagged hex file boot.obj. The ROM size is 32 kilobytes.
  
`./packer99 rom.bin 32 boot.obj -i -o -27136 modded-basic.obj`
The same as above, but also merges the file "modded-basic.obj" to the same image. It is possible to merge more than two files, just by adding more filenames. The current offset can be changed before each file, to build the ROM layout as necessary.

## General usage:
```  
Ruutu:packer99 epiehl$ ./packer99
packer99.exe written by Erik Piehl (C) 2016
Usage: packer99 rom-file rom-size [-i] [[-o offset] ti-file]
	rom-file	Name of output binary file.
	rom-size	Size of the ROM image in kilobytes
	-i		    Ignore writes outside of ROM
	-o offset	Optional offset to add to all addresses from source
	ti-file		Source file in tagged TI format
It is possible to have multiple t-files and offsets to write to the same ROM image.
Ruutu:packer99 epiehl$
```
