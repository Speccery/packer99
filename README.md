# packer99
packer99 is a command line tool to merge multiple TMS9900 tagged hex files to one binary image. It is intended initially to build simple boot ROMs for my TMS9995 breadboard project. The board has a memory paging unit implemented on an FPGA, and the ROM size in this example case is 256 kbytes. The tool supports per tagfile offsets, allowing many files to be merged and located on page boundaries as necessary.

At this point the tool only supports a handful of the hex file tags, more will be added as necessary.
This program is still fresh out of the oven and probably contains some bugs.

Examples:
`./packer99 rom.bin 32 boot.obj`
Creates a 32 kbyte binary rom file "rom.bin" from tagged hex file boot.obj. The ROM size is 32 kilobytes.
  
`./packer99 rom.bin 32 boot.obj -i -o -27136 modded-basic.obj`
The same as above, but also merges the file "modded-basic.obj" to the same image. It is possible to merge more than two files, just by adding more filenames. The current offset can be changed before each file, to build the ROM layout as necessary.

After some further development, now also raw binary images can be merged, and a.out format files too. The offsets can now be given as hexadecimal numbers.

`./packer99 rom7.bin 42 boot1.obj -i -t basic.obj -b -o 0x8000 forth.bin -a -o 0xa000 ../bb95/ledit99.out`

## General usage:
```  
Ruutu:packer99 epiehl$ ./packer99
packer99.exe written by Erik Piehl (C) 2016
Usage: packer99 rom-file rom-size [-i] [[-o offset] ti-file]
	rom-file	Name of output binary file.
	rom-size	Size of the ROM image in kilobytes
	-i		Ignore writes outside of ROM
	-o offset	Optional offset to add to all addresses from source
	-t		Next source file in tagged TI format
	-b		Next source file is a pure binary file
	-a		Next file in a.out format (16 byte header)
	filename	Name of source file
It is possible to have multiple files and offsets to write to the same ROM image.
Offset can be decimal or hexadecimal, a minus sign can be prepended.
Examples: -o 1430 -o -300 -o 0x8000 -o -0x100
Ruutu:packer99 epiehl$
```
