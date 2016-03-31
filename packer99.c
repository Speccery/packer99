// packer99.c
// Written by Erik Piehl
// 
// Started 2016-03-30
//
//

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#ifdef WIN32
#	include <malloc.h>
#endif
#include <stdlib.h>

unsigned char *image = NULL;

void set_min(int *current, int ref) {
    if (ref < *current) *current = ref;
}

void set_max(int *current, int ref) {
    if (ref > *current) *current = ref;
}

unsigned get_word(char **p) {
	// Convert 16-bit big endian word to hex, advance pointer by 4
	char s[5];
	int v = 0;
	s[0] = (*p)[0];
	s[1] = (*p)[1];
	s[2] = (*p)[2];
	s[3] = (*p)[3];
	s[4] = '\0';
	*p += 4;
	if(sscanf(s, "%x", &v) != 1) {
		fprintf(stderr, "Could not decode hex word %s\n", s);
	}
	return v;
}

int main(int argc, char *argv[]) {
  char *filename = NULL; // "modded-basic.obj";
  int rom_size = 64*1024;
  int min_addr_ref = 1 << 30;
  int max_addr_ref = 0;
  int min_addr_written = min_addr_ref;
  int max_addr_written = 0;
  unsigned org = 0;
  FILE *f = NULL;
  int line_nr = 0;
  int argi;
  int offset=0;
  int next_offset=0;

  printf("packer99.exe written by Erik Piehl\n");
  if (argc < 4) {
  	fprintf(stderr, 
  		"Usage: packer99 rom-file rom-size [[-o offset] ti-file]\n"
  		"\trom-file\tName of output binary file.\n"
  		"\trom-size\tSize of the ROM image in kilobytes\n"
  		"\toffset\tOptional offset to add to all addresses from source\n"
  		"\tti-file\tSource file in tagged TI format\n"
  		"It is possible to have multiple t-files and offsets to write to the same ROM image.\n"
  		);
  	return 1;
  }

  if(sscanf(argv[2], "%d", &rom_size) != 1) {
  	fprintf(stderr, "Unable to read rom-size\n");
  	return 2;
  }
  if(rom_size < 1 || rom_size > 4096) {
  	fprintf(stderr, "rom-size out of range\n");
  	return 2;
  }
  rom_size *= 1024;

  image = malloc(rom_size);
  if (!image) {
      perror("malloc failed...?");
      exit(5);
  }
  
  for(argi=3; argi<argc; argi++) {
  	if (next_offset) {
  		next_offset = 0;
  		if(sscanf(argv[argi], "%d", &offset) != 1) {
		  	fprintf(stderr, "Unable to decode offset: %s\n", argv[argi]);
		  	return 2;
  			
  		}
  		continue;
  	}
  	if (argv[argi][0] == '-' && argv[argi][1] == 'o') {
  		// The next component is the offset, decode it
  		next_offset = 1;
  		continue;
  	} 
  	
  	filename = argv[argi];
  
	  f = fopen(filename, "rt");
	  if (!image) {
		char s[512];
		sprintf(s, "Opening of %s failed.", filename);
		free(image);
		perror(s);
		exit(6);
	  }
	  printf("File: %s\n\tOffset=%d 0x%04X\n", filename, offset, offset);
	  line_nr = 0;
	  while(!feof(f)) {
		char s[512];
		char *p = s;
		char c;
		int val, go, checksum;

		if(!fgets(s, sizeof(s), f))
			continue;
		line_nr++;

		go = 1;
		while((c = *p++) && go) {	
			switch(c) {
				case '0': 
					// Section size tag
					val = get_word(&p);
					printf("\tPSEG size %d\n", val);
					p += 8; // skip the name
				break;
			case '9':
			  val = get_word(&p);
			  // printf("AORG %04X\n", val);
			  set_min( &min_addr_ref, val);
			  set_max( &max_addr_ref, val);
			  org = val;
			  break;
			case 'B':
			  val = get_word(&p);
			  set_min( &min_addr_written, org);
			  set_max( &max_addr_written, org);
			  if (org+offset > rom_size) {
				fprintf(stderr, "org+offset address beoynd rom_size %04X > %04X\n", org, rom_size);
				exit(5);
			  }
			  image[org+offset] = 0xFF & (val >> 8); // Big endian
			  image[org+offset] = 0xFF & val;
			  org += 2;
			  break;
			case 'F': 
			  go = 0; // End of record
			  break; 
			case '7':
			  {
				checksum = 0;
				char *t = s;
				short int si;
				while(t < p) 
				  checksum += *t++;

				val = get_word(&p); // Fetch checksum value. It is a 16 bit signed value.
				si = (short int)val;
		  
				if (checksum != -si) {
					printf("Line %d: Checksum error %d, %d\n", line_nr, checksum, -si);
				}
			  }  
			  break;
			case ':':
				go = 0;
				printf("\tReferenced memory locations: %04X - %04X\n" 
					"\tWritten memory locations: %04X - %04X\n",
					min_addr_ref, max_addr_ref, 
					min_addr_written, max_addr_written);
				printf("Processed %d lines.\n", line_nr);
				break;
			default:
			  printf("Unknown TAG %c, line %d, from %s\n",
				c, line_nr, p);
			  exit(0);
			  break;
			}
		}
	  }
	  fclose(f);
	} // for argi
	
  f = fopen(argv[1], "wb");
  if(f == NULL) {
    fprintf(stderr, "Unable to open rom file: %s", argv[1]);
	return 2;
  }
  printf("Writing %d kilobytes to %s\n", rom_size/1024, argv[1]);
  if(fwrite(image, 1, rom_size, f) != rom_size) {
  	fprintf(stderr, "The fwrite needs a loop - bug in source.\n");
  }
  fclose(f);
  
  free(image);
  return 0;
}