#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <vector>
#include <math.h>
#include <jpeglib.h>

#include "socket.h"
#include "camera.h"

//Main!
int main()
{
	char* rec = new char[256];
	Camera camera;

	Socket sock(false, 8001);
	sock.initialize();
	
	camera.setWidthHeight(160, 120);
	camera.initialize();
	
	
	unsigned char* mem = NULL;
	unsigned long mem_size = 0;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	cinfo.err = jpeg_std_error(&jerr);
	
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &mem, &mem_size);
	
	//sock.waitForConnection();
	int i = 0;
	unsigned char* image;
	printf("size:%d\n", camera.getWidth());
	while(true)
	{
		
		image = camera.getBuffer();
		if(image == NULL)
			printf("whata \n");
		else
		{
			char buffer[50];
			int n;
			n=sprintf(buffer, "image%d.jpeg", i);
			FILE* outfile = fopen(buffer, "wb");
			
			printf("Image %d \n", i);
			
			jpeg_create_compress(&cinfo);
			jpeg_stdio_dest(&cinfo, outfile);
			
			cinfo.image_width = camera.getWidth();
			cinfo.image_height = camera.getHeight();
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			
			jpeg_set_defaults( &cinfo );
			jpeg_set_quality(&cinfo, 75, TRUE);
			
			jpeg_start_compress( &cinfo, TRUE );
			
			while( cinfo.next_scanline < cinfo.image_height )
			{
				row_pointer[0] = &image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
				jpeg_write_scanlines( &cinfo, row_pointer, 1 );
			}
			
			jpeg_finish_compress( &cinfo );
			jpeg_destroy_compress( &cinfo );
			
		}
		i++;
	}
	
	return 0;
}