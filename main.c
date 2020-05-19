#include <linux/fb.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>

static inline uint32_t pixel_color(uint8_t r, uint8_t g, uint8_t b, struct fb_var_screeninfo *vinfo)
{
	return (r<<vinfo->red.offset) | (g<<vinfo->green.offset) | (b<<vinfo->blue.offset);
}

int main()
{
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;

	int fb_fd = open("/dev/fb0",O_RDWR);

	//Get variable screen information
	ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
	vinfo.grayscale=0;
	vinfo.bits_per_pixel=32;
	ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
	ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);

	ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);

	long screensize = vinfo.yres_virtual * finfo.line_length;
	// memory map the frame buffer
	uint8_t *fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, (off_t)0);
	// clean the canvas
	memset((void *)fbp, '\0', screensize);

	// math variables
	int x_min = -2;
	int x_max = 1;
	int y_min = -1;
	int y_max = 1;
	float x_step = (float)(x_max - x_min) / vinfo.xres;
	float y_step = (float)(y_max - y_min) / vinfo.yres;
	float x_n, y_n, x_temp, x_0, y_0;
	uint32_t pixel;
	int max_iter = 25;
	// loop control variables
	int x,y;

	// final render
	for (x=0;x<vinfo.xres;x++) 
	{
		x_0 = x_min + x_step * x;
		for (y=0;y<vinfo.yres;y++)
		{
			y_0 = y_max - y_step * y;
			long location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;
						
			x_n = x_0;
			y_n = y_0;
			int i = 0;
			
			while( (x_n * x_n + y_n * y_n <= 4) && i < max_iter) {
				x_temp = x_n * x_n - y_n * y_n + x_0;
				y_n = 2 * x_n * y_n + y_0;
				x_n = x_temp;		
				i = i + 1;
			}

			*((uint32_t*)(fbp + location)) = pixel_color(i*10 ,0x00,0x00, &vinfo);
		}
	}

	// int i = 0;
	// double x_array[vinfo.xres][vinfo.yres];
	// memset(x_array, 0, sizeof(x_array[0][0]) * vinfo.xres * vinfo.yres);
	// double y_array[vinfo.xres][vinfo.yres];
	// memset(y_array, 0, sizeof(y_array[0][0]) * vinfo.xres * vinfo.yres);
	// while (i < 25) {
	// 	for (x=0;x<vinfo.xres;x++) 
	// 	{
	// 		x_0 = x_min + x_step * x;
	// 		for (y=0;y<vinfo.yres;y++)
	// 		{
	// 			y_0 = y_max - y_step * y;
	// 				printf("[%.2f, %.2f]\n", 0, y_0);
	// 			long location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;
							
	// 			x_n = x_array[x][y];
	// 			y_n = y_array[x][y];
				
	// 			if ( (x_n * x_n + y_n * y_n > 16) ) { 
	// 				*((uint32_t*)(fbp + location)) = pixel_color(0xFF ,0xFF,0xFF, &vinfo);
	// 			} else {
	// 				int val = (x_n * x_n + y_n * y_n) * 10;
					
	// 				*((uint32_t*)(fbp + location)) = val; //pixel_color(0x00 ,0x00,val, &vinfo);
	// 			}

	// 			x_temp = x_n * x_n - y_n * y_n + x_0;
	// 			// y_n = 2 * x_n * y_n + y_0;
	// 			y_array[x][y] = 2 * x_n * y_n + y_0;
	// 			// x_n = x_temp;		
	// 			x_array[x][y] = x_temp;				
	// 		}
		
	// 	}
	// 	i = i +1;
	// }




	//printf("x  [%ld] and y [%ld]\n", vinfo.xres, vinfo.yres);
	return 0;
}
