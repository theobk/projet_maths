/*
 * A simple libpng example program
 * http://zarb.org/~gc/html/libpng.html
 *
 * Modified by Yoshimasa Niwa to make it much simpler
 * and support all defined color_type.
 *
 * To build, use the next instruction on OS X.
 * $ brew install libpng
 * $ clang -lz -lpng15 libpng_test.c
 *
 * Copyright 2002-2010 Guillaume Cottenceau.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <png.h>

int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers;


void read_png_file(char *filename) {
  FILE *fp = fopen(filename, "rb");

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png) abort();

  png_infop info = png_create_info_struct(png);
  if(!info) abort();

  if(setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  png_read_info(png, info);

  width      = png_get_image_width(png, info);
  height     = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);

  // Read any color_type into 8bit depth, RGBA format.
  // See http://www.libpng.org/pub/png/libpng-manual.txt

  if(bit_depth == 16)
    png_set_strip_16(png);

  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if(png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if(color_type == PNG_COLOR_TYPE_RGB ||
     color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }

  png_read_image(png, row_pointers);

  fclose(fp);
}

void write_png_file(char *filename) {

  FILE *fp = fopen(filename, "wb");
  if(!fp) abort();

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();

  png_infop info = png_create_info_struct(png);
  if (!info) abort();

  if (setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  // Output is 8bit depth, RGBA format.
  png_set_IHDR(
    png,
    info,
    width, height,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  //png_set_filler(png, 0, PNG_FILLER_AFTER);

  png_write_image(png, row_pointers);
  png_write_end(png, NULL);

  for(int y = 0; y < height; y++) {
    free(row_pointers[y]);
  }
  free(row_pointers);

  fclose(fp);
}

void png_to_tab(png_bytep * px_tab, png_bytep * px_tab_flou) {

	for(int y = 0; y < height; y++) {
		png_bytep row = row_pointers[y];
		for(int x = 0; x < width; x++) {
		  png_bytep px = &(row[x * 4]);

		  px_tab[y*width+x]=px;
		  px_tab_flou[y*width+x]=px;


		  //printf("%4d, %4d = RGBA(%d, %d, %d, %d)\n", x, y, px[0], px[1], px[2], px[3]);
		}
	}
}

double flou_gauss_point(int x, int y, double sigma){
	return 1. / (2 * M_PI * sigma * sigma) * exp(-(x * x + y * y) / (2 * sigma * sigma));
}

void flou_tab(png_bytep * px_tab, png_bytep * px_tab_flou, double sigma, int mu){

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			int buff;

			if(mu%2==0){
				buff = mu/2-1;
			}
			else {
				buff = mu/2;
			}

			double resR=0;
			double sumR=0;
			double resG=0;
			double sumG=0;
			double resB=0;
			double sumB=0;
			double resA=0;
			double sumA=0;

			double g=0;

			for(int i =-(buff) ; i<buff; i++){
				for(int j = -1*buff; j<buff; j++){

					if(y+i < 0 || x+j < 0 || y+i >= height || x+j >= width){
						perror("en dehors de l'image\n");
					}
					else {
						g=flou_gauss_point(y+i, x+j, sigma);

						printf("x+j : %d, y+i : %d\n",x+j,y+i);

						sumR += px_tab[((y+i) * width)+(x+j)][0];

						sumG += px_tab[((y+i) * width)+(x+j)][1];

						sumB += px_tab[((y+i) * width)+(x+j)][2];

						sumA += px_tab[((y+i) * width)+(x+j)][3];


						resR += g * px_tab[((y+i) * width)+(x+j)][0];

						resG += g * px_tab[((y+i) * width)+(x+j)][1];

						resB += g * px_tab[((y+i) * width)+(x+j)][2];

						resA += g * px_tab[((y+i) * width)+(x+j)][3];
					}
				}
			}

			printf("Avant les ajouts dans tab_flou\n");
			px_tab_flou[y*width+x][0]=resR/sumR;
			px_tab_flou[y*width+x][1]=resG/sumG;
			px_tab_flou[y*width+x][2]=resB/sumB;
			px_tab_flou[y*width+x][3]=resA/sumA;
			printf("Après les ajouts dans tab_flou\n");


		}
	}
}

int main(int argc, char *argv[]) {

	if(argc<2){
		perror("Manque d'arguments : ");
		exit(1);
	}

	int sigma=4;
	int mu=2*sigma;

	printf("\n-------------------Chargement de l'image-------------------\n\n");


	read_png_file(argv[1]);
	printf("Height : %d\nWidth : %d\n", height, width );

	printf("\n----------------Création du tableau de pixel----------------\n\n");

	png_bytep px_tab[height*width];
	png_bytep px_tab_flou[height*width];

	png_to_tab(px_tab, px_tab_flou);

	printf("Tableau créé\n");

	printf("\n----------------Application du flou Gaussien----------------\n\n");

	double res=flou_gauss_point(5,5, 4.);

	printf("%f\n", res);

	flou_tab(px_tab, px_tab_flou,sigma, mu);




	return 0;
}
