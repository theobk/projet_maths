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
png_bytep *row_pointers_flou;


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

void write_png_file(char *filename, png_bytep *row_pointers_generique) {
  int y;

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

  png_write_image(png, row_pointers_generique);
  png_write_end(png, NULL);

  for(int y = 0; y < height; y++) {
    free(row_pointers_generique[y]);
  }
  free(row_pointers_generique);

  fclose(fp);
}

double flou_gauss_point(int x, int y, int sigma){
	return 1. / (2 * M_PI * sigma * sigma) * exp(-(x * x + y * y) / (2 * sigma * sigma));
}

void flou_stuct_png(int sigma){

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) abort();

	png_infop info = png_create_info_struct(png);
	if(!info) abort();

	row_pointers_flou = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) {
	  row_pointers_flou[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	int mu=2*sigma;

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
		  int buff;

		//   int R = row_pointers[y][x*4];
		//   int G = row_pointers[y][x*4+1];
		//   int B = row_pointers[y][x*4+2];
		  //
		  //
		//   printf("%d / %d / %d\n", R, G, B);

		  if(mu%2==0){
			  buff = mu/2-1;
		  }
		  else {
			  buff = mu/2;
		  }

		  double resR=0;
		  double resG=0;
		  double resB=0;
		  double resA=0;
		  double summGauss=0;

		  double g=0;

		  for(int i =-(buff) ; i<buff; i++){
			  for(int j = -1*buff; j<buff; j++){

				  if(y+i < 0 || x+j < 0 || y+i >= height || x+j >= width){
				  }
				  else {

					  g=flou_gauss_point(i, j, sigma);

					  summGauss += g;

					  png_bytep px = &row_pointers[y+i][(x+j)*4];

					//   printf("R : %d\n", px[0]);
					//   printf("G : %d\n", px[1]);
					//   printf("B : %d\n", px[2]);
					//   printf("A : %d\n", px[3]);
					  //
					  //
					//   printf("g : %f\n\n", g);

					  resR += g * px[0];
					  resG += g * px[1];
					  resB += g * px[2];
					  resA += g * px[3];



				  }
			  }
		  }
		  row_pointers_flou[y][x*4]=resR/summGauss;
		  row_pointers_flou[y][x*4+1]=resG/summGauss;
		  row_pointers_flou[y][x*4+2]=resB/summGauss;
		  row_pointers_flou[y][x*4+3]=resA/summGauss;

		  //printf("%4d, %4d = RGBA(%d, %d, %d, %d)\n", x, y, px[0], px[1], px[2], px[3]);
		}
	}
}

void print_row_pointer(png_bytep *row_pointers_print, char *s){
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			  int R = row_pointers_print[y][x*4];
			  int G = row_pointers_print[y][x*4+1];
			  int B = row_pointers_print[y][x*4+2];
			  int A = row_pointers_print[y][x*4+3];


			  printf("%d / %d / %d/ %d\n", R, G, B, A);
		}
	}

	printf("%s\n", s);
}

void deriv_tab(png_bytep * px_tab_flou, png_bytep * px_tab_deriv){
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			double stencil[5];

			stencil[0]=1/12;
			stencil[1]=-8/12;
			stencil[2]=0;
			stencil[3]=8/12;
			stencil[4]=-1/12;

			px_tab_deriv[y*width+x][0]=0;
			px_tab_deriv[y*width+x][1]=0;
			px_tab_deriv[y*width+x][2]=0;
			px_tab_deriv[y*width+x][3]=0;

			for(int i = -2; i<3; i++){
				px_tab_deriv[y*width+x][0]+=px_tab_flou[y*width+x+i][0]*stencil[i+2];
				px_tab_deriv[y*width+x][0]+=px_tab_flou[y+i*width+x][0]*stencil[i+2];
				px_tab_deriv[y*width+x][1]+=px_tab_flou[y*width+x+i][1]*stencil[i+2];
				px_tab_deriv[y*width+x][1]+=px_tab_flou[y+i*width+x][1]*stencil[i+2];
				px_tab_deriv[y*width+x][2]+=px_tab_flou[y*width+x+i][2]*stencil[i+2];
				px_tab_deriv[y*width+x][2]+=px_tab_flou[y+i*width+x][2]*stencil[i+2];
				px_tab_deriv[y*width+x][3]+=px_tab_flou[y*width+x+i][3]*stencil[i+2];
				px_tab_deriv[y*width+x][3]+=px_tab_flou[y+i*width+x][3]*stencil[i+2];
			}

		}
	}
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
			double resG=0;
			double resB=0;
			double resA=0;
			double summGauss=0;

			double g=0;

			for(int i =-(buff) ; i<buff; i++){
				for(int j = -1*buff; j<buff; j++){

					if(y+i < 0 || x+j < 0 || y+i >= height || x+j >= width){
					}
					else {

						g=flou_gauss_point(i, j, sigma);

						summGauss += g;

						resR += g * px_tab[((y+i) * width)+(x+j)][0];

						resG += g * px_tab[((y+i) * width)+(x+j)][1];

						resB += g * px_tab[((y+i) * width)+(x+j)][2];

						resA += g * px_tab[((y+i) * width)+(x+j)][3];
					}
				}
			}

			//printf("Avant les ajouts dans tab_flou /%f/%f/%f/%f\n", resR, resG, resB, resA);
			px_tab_flou[y*width+x][0]=resR/summGauss;
			px_tab_flou[y*width+x][1]=resG/summGauss;
			px_tab_flou[y*width+x][2]=resB/summGauss;
			px_tab_flou[y*width+x][3]=resA/summGauss;
			//printf("Après les ajouts dans tab_flou /%f/%f/%f/%f\n",resR/summGauss,resG/summGauss, resB/summGauss, resA/summGauss );

		}
	}
}

int main(int argc, char *argv[]) {

	if(argc<2){
		perror("Manque d'arguments : ");
		exit(1);
	}

	int sigma=4;

	printf("\n-------------------Chargement de l'image-------------------\n\n");


	read_png_file(argv[1]);
	printf("Height : %d\nWidth : %d\n", height, width );

	//write_png_file("res.png", row_pointers);


	printf("\n----------------Application du flou Gaussien----------------\n\n");

	flou_stuct_png(sigma);

	print_row_pointer(row_pointers, "Pas Flou");

	printf("\n--------------------Ecriture de l'image--------------------\n\n");

	//write_png_file("res.png", row_pointers_flou);


	//deriv_tab(px_tab_flou, px_tab_deriv);


	return 0;
}
