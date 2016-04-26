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
#include <string.h>

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

struct config {
  // Global options
  int rom_size;
  int warning_counter;
  int	ignore;
  // file-specific options
	int offset;
  FILE *f;
  char *filename;
};

int read_tagged(struct config *cfg) {
  int min_addr_ref = 1 << 30;
  int max_addr_ref = 0;
  int min_addr_written = min_addr_ref;
  int max_addr_written = 0;
  int line_nr = 0;
  unsigned org = 0;
  
  line_nr = 0;
  while(!feof(cfg->f)) {
    char s[512];
    char *p = s;
    char c;
    int val, go, checksum;
    
    if(!fgets(s, sizeof(s), cfg->f))
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
          if (org+cfg->offset > cfg->rom_size) {
            if(++cfg->warning_counter < 5)
              fprintf(stderr, "\tWarning: org+offset address beoynd rom_size %04X > %04X\n",
                      org, cfg->rom_size);
            else if(cfg->warning_counter == 5)
              fprintf(stderr, "\tOnly first 5 warnings reported\n");
            if (!cfg->ignore)
              return 5;
          }
          if (org+cfg->offset < cfg->rom_size-1) {
            image[org+cfg->offset] = 0xFF & (val >> 8); // Big endian
            image[org+cfg->offset+1] = 0xFF & val;
          }
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
          printf("\tProcessed %d lines.\n", line_nr);
          break;
        default:
          printf("\tUnknown TAG %c, line %d, from %s\n",
                 c, line_nr, p);
          return 6;
      }
    }
  }
  fclose(cfg->f);
  return 0;
}

int main(int argc, char *argv[]) {
  struct config cfg;
  int argi;
  int next_offset=0;
  enum { TAGGED, BINARY, AOUT } read_type = TAGGED;
  FILE *fout = NULL;
  
  cfg.f = NULL;
  cfg.filename = NULL;
  cfg.offset = 0;
  cfg.ignore = 0;
  cfg.rom_size = 64*1024;
  cfg.warning_counter = 0;

  printf("packer99.exe written by Erik Piehl (C) 2016\n");
  if (argc < 4) {
  	fprintf(stderr, 
  		"Usage: packer99 rom-file rom-size [-i] [[-o offset] ti-file]\n"
  		"\trom-file\tName of output binary file.\n"
  		"\trom-size\tSize of the ROM image in kilobytes\n"
  		"\t-i\t\tIgnore writes outside of ROM\n"
  		"\t-o offset\tOptional offset to add to all addresses from source\n"
  		"\t-t\t\tNext source file in tagged TI format\n"
      "\t-b\t\tNext source file is a pure binary file\n"
      "\t-a\t\tNext file in a.out format (16 byte header)\n"
      "\tfilename\tName of source file\n"
  		"It is possible to have multiple files and offsets to write to the same ROM image.\n"
      "Offset can be decimal or hexadecimal, a minus sign can be prepended.\n"
      "Examples: -o 1430 -o -300 -o 0x8000 -o -0x100\n"
  		);
  	return 1;
  }

  if(sscanf(argv[2], "%d", &cfg.rom_size) != 1) {
  	fprintf(stderr, "Unable to read rom-size\n");
  	return 2;
  }
  if(cfg.rom_size < 1 || cfg.rom_size > 4096) {
  	fprintf(stderr, "rom-size out of range (1..4096 kbytes)\n");
  	return 2;
  }
  cfg.rom_size *= 1024;

  image = malloc(cfg.rom_size);
  if (!image) {
      perror("malloc failed...?");
      exit(5);
  }
  memset(image, 0xFF, cfg.rom_size);
  
  for(argi=3; argi<argc; argi++) {
    int r;

  	if (next_offset) {
      char *format = "%d";
      char *p = argv[argi];
      int neg = 0;
      if (p[0] == '-') {
        neg = 1;
        p++;
      }
      if(p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
        format = "%x";
  		next_offset = 0;
  		if(sscanf(p, format, &cfg.offset) != 1) {
		  	fprintf(stderr, "Unable to decode offset: %s\n", argv[argi]);
		  	return 2;
      } else {
        if (neg)
          cfg.offset = -cfg.offset;
        // printf("Current offset %d 0x%X\n", cfg.offset, cfg.offset);
      }
  		continue;
  	}
    
    if(argv[argi][0] == '-') {
      switch(argv[argi][1]) {
        case 'o':
          next_offset = 1;  // The next component is the offset, decode it
          break;
        case 'i':
          cfg.ignore = 1; // Turn on the flag to ignore writes above boundary.
          break;
        case 'a':
          read_type = AOUT;
          break;
        case 't':
          read_type = TAGGED;
          break;
        case 'b':
          read_type = BINARY;
          break;
        default:
          printf("Ignoring unknown option %c\n", argv[argi][1]);
          break;
      }
      continue;
    }
  	
  	cfg.filename = argv[argi];
    cfg.f = fopen(cfg.filename, read_type == TAGGED ? "rt" : "rb");
	  if (!cfg.f) {
			char s[512];
			sprintf(s, "Opening of %s failed.", cfg.filename);
			free(image);
			perror(s);
			exit(6);
	  }
    printf("File: %s\n\tOffset=%d hex 0x%04X\n", cfg.filename, cfg.offset, cfg.offset);
    
    if (read_type == TAGGED) {
      r = read_tagged(&cfg);
      if (r) {
        printf("Processing of tagged file failed - exiting\n");
        exit(r);
      }
    } else {
      // a.out or binary format
      size_t n, count = 0;
      int bs = 1024;
      if (read_type == AOUT)
        fseek(cfg.f, 16, SEEK_SET); // skip header in the beginning
      printf("\tReading %s in %s format to offset 0x%X\n",
             cfg.filename, read_type == BINARY ? "raw binary" : "a.out",
             cfg.offset);
      do {
        if ((long)cfg.offset > (long)cfg.rom_size || (long)cfg.offset < 0) {
          fprintf(stderr, "Error: Offset beyond ROM area.\n");
          exit(8);
        }
        n = fread(image+cfg.offset, 1, bs, cfg.f);
        cfg.offset += n;
        count += n;
      } while(n == bs);
      if(!feof(cfg.f)) {
        char s[512];
        sprintf(s, "End-of-file expected, but error occured.");
        perror(s);
        exit(7);
      }
      fclose(cfg.f);
      printf("\tLast write to address 0x%X, %d bytes\n", cfg.offset, (int)count);
    } // end of binary/aout processing

	} // for argi
	
  fout = fopen(argv[1], "wb");
  if(fout == NULL) {
    fprintf(stderr, "Unable to open rom file: %s", argv[1]);
	return 2;
  }
  printf("Writing %d kilobytes to %s\n", cfg.rom_size/1024, argv[1]);
  if(fwrite(image, 1, cfg.rom_size, fout) != cfg.rom_size) {
  	fprintf(stderr, "The fwrite needs a loop - bug in source.\n");
  }
  fclose(fout);
  
  free(image);
  return 0;
}
