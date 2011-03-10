#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "zlib.h"

#define REGION_SIDE 32

int toShort(char *data) {
	return data[0] << 8 | data[1];
}
int toInt(char *data) {
	return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}
void dumphex(unsigned char * data, int num) {
	while (num --) {
             fprintf(stderr, "%.2X", *data);
	     data++;
	}
	putchar('\n');
}

int sizeNBT(int type, char *data) {
      switch(type) {
	      case 0:
		      assert(type != 0);
		      break;
	      case 1:
		      return 1;
	      case 2:
		      return 2;
	      case 3:
		      return 4;
	      case 4:
		      return 8;
	      case 5:
		      return 4;
	      case 6: 
		      return 8;
	      case 7:
                      return toInt(data) + 4;
	      case 8:
                      return toShort(data) + 2;
              case 9:
		      {
		      int size = 5;
		      int type = data[0];
		      int num = toInt(data + 1);
		      
		      for (int i = 0; i < num; i++) {
			      size += sizeNBT(type, data + size);
		      }
		      return size;
		      }
	      case 10:
		      {
		      int size = 0;
		      while (1) {
			      int type = data[size];
			      size ++;
			      if (type == 0) return size ;
			      int stringlength = toShort(data + size);
			      size += stringlength + 2 ;
			      size += sizeNBT(type, data + size);
		      }
		      }
	}
       assert(0);
} 


char * getNBT(char *string, char *data) {

       char * search;
       int search_length;
      { 
	      char * tmp = strchr(string, '.');

	      if (tmp == 0) {
		      search_length = strlen(string);
	      } else {
		      search_length = tmp - string;
		  }
	      search = (char *)malloc(search_length + 1);
	      assert(search);
	      memcpy(search, string, search_length + 1);
	      search[search_length] = 0;
	  }



	      

		      int size = 0;
		      while (data[size] != 0) {
			      int type = data[size];
			      size ++;
			      int stringlength = toShort(data + size);
			      int chk = (stringlength == search_length && memcmp(search, data + size + 2, stringlength) == 0);
			      size += stringlength + 2;
			      if ( chk) {
				      assert(4);
					// MATCH !!!
					if (type == 10) {
						return getNBT(string + search_length + 1, data + size);
					} else {
						return data + size;
					}
				}
			      size += sizeNBT(type, data + size);
		      }
		      assert(0);
		     return 0;

}








char * zlib_decompress(char * compressed_data, int compressed_length, int *decompressed_length) {
    z_stream strm;
    int ret;
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = compressed_length;
    strm.next_in = (Bytef*) compressed_data;
    int output_size = compressed_length;
    Bytef * output = (Bytef *)malloc(output_size);
    assert(output);
    int have = 0;
    
    ret = inflateInit(&strm);
    assert(ret == Z_OK);
    /* decompress until deflate stream ends or end of file */
    do {
        if (output_size <= have / 2) {
    	    output_size *= 2;
    	    output = (Bytef *)realloc(output, output_size);
    	    assert(output);
        }
        strm.avail_out = output_size - have;
        strm.next_out = output + have;
        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
        switch (ret) {
          case Z_NEED_DICT:
        	ret = Z_DATA_ERROR;     /* and fall through */
           case Z_DATA_ERROR:
           case Z_MEM_ERROR:
        	(void)inflateEnd(&strm);
        	return 0;
        }
        have = output_size - strm.avail_out;
    } while (ret != Z_STREAM_END);
    /* clean up and return */
    (void)inflateEnd(&strm);

    *decompressed_length = have;
    return (char *)output;
}    


int readmcfile(FILE *source, char **dest)
{

    // find mc
    int mcheader_size = 4 * 2 * REGION_SIDE;
    unsigned char *mcheader_data = (unsigned char *) malloc(mcheader_size);
    assert(mcheader_data);

    fread(mcheader_data, 1, mcheader_size, source);
    assert(!feof(source));


    // funky shit cause byte order is fucked
    int offset = mcheader_data[0] << 16 | mcheader_data[1] << 8 | mcheader_data[2] ;
    int sectors = mcheader_data[4];
    fprintf(stderr, "%d, %d\n", offset, sectors);
    assert(!fseek(source, offset << 12, SEEK_SET));
   

    unsigned int chunk_length;
    fread(&chunk_length, 1, 4, source);

    char chunk_compression_type;
    fread(&chunk_compression_type, 1, 1, source);
    fprintf(stderr, "%d\n", chunk_compression_type);
    assert(chunk_compression_type == 2);

    char * chunk_data = (char *) malloc(chunk_length);
    assert(chunk_data);
    fread(chunk_data, 1, chunk_length, source);

    int uchunk_length;
    unsigned char * uchunk_data = (unsigned char *) zlib_decompress(chunk_data, chunk_length, &uchunk_length);

//    fwrite(uchunk_data, 1, uchunk_length, stdout)

    dumphex((unsigned char *) getNBT("Level.Blocks", (char *)uchunk_data + 3), 20);
	fprintf(stderr,  "%d", toInt((char *) getNBT("Level.Blocks", (char *)uchunk_data + 3)));

	return 0;
}


int main(int agvc, char ** argv) {
	readmcfile(stdin, 0);
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
