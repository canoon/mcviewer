
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "zlib.h"
#include <new>

#define REGION_SIDE 32
#define REGION_HEADER_SIZE (REGION_SIDE * REGION_SIDE * 4 * 2)


void
dumphex (const unsigned char *data, int num)
{
  while (num--)
    {
      fprintf (stderr, "%.2X", *data);
      data++;
    }
  fprintf (stderr, "\n");
}

unsigned int
reverse_byte_order(unsigned int v) {
	return v >> 24 | v << 24 | v >> 8 & 0xFF00 | v << 8 & 0xFF0000;
}
unsigned char *
zlib_decompress (const unsigned char *compressed_data, unsigned int compressed_length,
		 unsigned int *decompressed_length)
{
  int ret;

  // zlib init
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = compressed_length;
  strm.next_in = (unsigned char *)compressed_data;
  // Guess a first output size
  unsigned int output_size = compressed_length;
  Bytef *output = (unsigned char *) malloc (output_size);
  assert (output);
  unsigned int have = 0;
  ret = inflateInit (&strm);
  if (ret != Z_OK) {
	  fprintf(stderr, "ERROR: inflateInit() returned %d\n", ret);
	  return 0;
  }

  do
    {
      if (have == compressed_length && have != output_size) {
	      return 0;
      }
      // Simple growing buffer
      if (output_size >= have / 2)
	{
	  output_size *= 2;
	  output = (unsigned char *) realloc (output, output_size);
	  assert (output);
	}
      strm.avail_out = output_size - have;
      strm.next_out = output + have;
      ret = inflate (&strm, Z_NO_FLUSH);
      if (ret == Z_STREAM_ERROR) {
	  fprintf(stderr, "ERROR: inflate() returned %d\n", ret);
	  return 0;
      }
      switch (ret)
	{
	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_MEM_ERROR:
	  (void) inflateEnd (&strm);
	  fprintf(stderr, "ERROR: inflate() returned %d\n", ret);
	  return 0;
	}
      have = output_size - strm.avail_out;
    }
  while (ret != Z_STREAM_END);
  (void) inflateEnd (&strm);

  *decompressed_length = have;
  return output;
}

class MCRegion {
	char * region_file_name;
	FILE * region_file;
	unsigned char * region_header;
	unsigned char * chunks[REGION_SIDE][REGION_SIDE];
	public:
		MCRegion(char *w, int x, int z) {
		  // Find region file
		  {
		    assert(  asprintf (&region_file_name, "%s/region/r.%d.%d.mcr", w, x, z) != -1);
		    assert(region_file_name);
		    region_file = fopen (region_file_name, "r");
		    if (!region_file)
		      {
			fprintf (stderr, "ERROR: Could not open region file %s.\n", region_file_name);
		      }
		  }
		  if (region_file) {
			  // Load header
			  unsigned int region_header_size = 4 * 2 * REGION_SIDE * REGION_SIDE;
			  region_header = (unsigned char *) malloc (region_header_size);
			  assert (region_header);

			  if(fread (region_header, 1, region_header_size, region_file) != region_header_size) {
				  fprintf(stderr, "WARNING: Could not load region header completely.\n");
			  }
			  fprintf(stderr, "a: %d\n", sizeof(chunks));
			  memset(chunks, 0, sizeof(chunks));
			  dumphex((unsigned char *)chunks, 4000);
		  }
		}

	        unsigned char * getChunk(int x, int z) {
		if (region_file) {

			  // Find chunk
			  unsigned int chunk_number = 4 * (x + z * REGION_SIDE);
			  unsigned int chunk_offset =
			    (unsigned int) region_header[chunk_number] << 16 | (unsigned int) region_header[chunk_number + 1] << 8 | region_header[chunk_number + 2];
			  unsigned int sectors = region_header[chunk_number + 3];
			  if (chunk_offset == 0) {
				  return 0;
			  }
			  fprintf (stderr, "INFO: Loading chunk #%d (%d, %d) from offset %d with %d sectors.\n", chunk_number, x, z, chunk_offset, sectors);
			  if (fseek (region_file, chunk_offset << 12, SEEK_SET) != 0) {
				  fprintf(stderr, "ERROR: Could not seek to %d in region file (perhaps incomplete).\n", chunk_offset << 12);
				  return 0;
			  }

			  // Read chunk header
			  unsigned int chunk_length;
			  if (fread (&chunk_length, 1, 4, region_file) != 4) {
				  fprintf(stderr, "ERROR: Could not read chunk length from region file (perhaps incomplete).\n");
				  return 0;
			  }
			  chunk_length = reverse_byte_order(chunk_length);
			  char chunk_compression_type;
			  if (fread (&chunk_compression_type, 1, 1, region_file) != 1) {
				  fprintf(stderr, "ERROR: Could not read chunk compression type from region file (perhaps incomplete).\n");
				  return 0;
			  }
			  if (chunk_compression_type != 2) {
				  fprintf(stderr, "WARNING: Chunk compression type is not 2 (zlib) but %d.\n", chunk_compression_type);
			  }
			  // Read chunk header
			  unsigned char *chunk = (unsigned char *) malloc (chunk_length);
			  assert (chunk);
			  unsigned int a;
			  int chunk_length_read = fread (chunk, 1, chunk_length, region_file);
			  if (chunk_length_read != chunk_length) {
				  
				  fprintf(stderr, "ERROR: Could not read all chunk data from region file (%d/%d).\n", chunk_length_read, chunk_length);
			  }

			  unsigned int uchunk_length;
			  unsigned char *uchunk = zlib_decompress (chunk, chunk_length,
							       &uchunk_length);
			  if (!uchunk) {
				  fprintf(stderr, "ERROR: zlib_decompress() returned 0");
				  return 0;
			  }

			  free(chunk);


			  return uchunk;
			 } else {
				 return 0;
			}
		}
		unsigned char * chunk(int x, int z) {
			if (chunks[x][z]) {
				return chunks[x][z];
			} else {
				return chunks[x][z] = getChunk(x, z);
			}
		}
};

class World {
		MCRegion *regions[256][256];
	public:
                char * world_path;
		World(char * world_path) {
			this->world_path = world_path;
			memset(regions, 0, 256 * 256); 
		}
		MCRegion *region(int x, int z) {
			if (regions[x][z]) {
				return regions[x][z];
			} else {
				fprintf(stderr, "Creating new region\n");
				return (regions[x][z] = new MCRegion(world_path, x, z));
			}
		}
		unsigned char * chunk(int x, int z) {
			region(x / REGION_SIDE, z / REGION_SIDE)->chunk(x % REGION_SIDE, z % REGION_SIDE);
		}



};


int
toShort (const unsigned char *data)
{
  return data[0] << 8 | data[1];
}

int
toInt (const unsigned char *data)
{
  return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}



int
sizeNBT (int type, const unsigned char *data)
{
  switch (type)
    {
    case 1:
// bool	  
      return 1;
    case 2:
      // short
      return 2;
    case 3:
      // int
      return 4;
    case 4:
      // long
      return 8;
    case 5:
      // float
      return 4;
    case 6:
      // double
      return 8;
    case 7:
      // array[int]
      return toInt (data) + 4;
    case 8:
      // array[short]
      return toShort (data) + 2;
    case 9:
      // list
      {
	int size = 5;
	int type = data[0];
	int num = toInt (data + 1);

	for (int i = 0; i < num; i++)
	  {
	    int tmpsize = sizeNBT (type, data + size);
	    if (!tmpsize) {
		    return 0;
	    }
	    size += tmpsize;
	  }
	return size;
      }
    case 10:
      // compound
      {
	int size = 0;
	while (1)
	  {
	    int type = data[size];
	    size++;
	    if (type == 0)
	      return size;
	    int stringlength = toShort (data + size);
	    size += stringlength + 2;
	    int tmpsize = sizeNBT (type, data + size);
	    if (!tmpsize) {
		    return 0;
	    }
	    size += tmpsize;
	  }
      }
    }
  return 0;
}


const unsigned char *
getNBT (const char *search, const unsigned char *nbt)
{
  // Separate first part (search.split('.').first in ruby :-( )
  char *search_first;
  int search_first_length;
  {
    const char *tmp = strchr (search, '.');

    if (tmp == 0)
      {
	search_first_length = strlen (search);
      }
    else
      {
	search_first_length = tmp - search;
      }
    search_first = (char *) malloc (search_first_length + 1);
    assert (search_first);
    memcpy (search_first, search, search_first_length);
    search_first[search_first_length] = '\0';
  }
 
  // Find sub item
  int size = 0;
  while (nbt[size] != 0)
    {
      int type = nbt[size];
      size++;
      int name_length = toShort (nbt + size);
      size += 2;
      int chk = (name_length == search_first_length
		 && memcmp (search_first, nbt + size, search_first_length) == 0);
      size += name_length;
      if (chk)
	{
	  // MATCH !!!
	  if (type == 10)
	    {
	      return getNBT (search + name_length + 1, nbt + size);
	    }
	  else
	    {
	      return nbt + size;
	    }
	}
      int tmpsize = sizeNBT (type, nbt + size);
      if (!tmpsize) {
	      fprintf(stderr, "ERROR: Malformed NBT (sizeNBT() returned 0).\n");
	      return 0;
      }
      size += tmpsize;
    }
  return 0;

}







/*
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}*/
