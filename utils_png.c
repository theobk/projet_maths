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
png_bytep *row_pointers_diff;
png_bytep *row_pointers_deriv;

png_bytep *row_pointers_A;
png_bytep *row_pointers_flou_A;
png_bytep *row_pointers_deriv_A;

png_bytep *row_pointers_B;
png_bytep *row_pointers_flou_B;
png_bytep *row_pointers_deriv_B;



void read_png_file(char *filename, char c) {
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

  if(bit_depth == 16)
    png_set_strip_16(png);

  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

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

  if(c=='A'){
	row_pointers_A=row_pointers;
  }
  else if(c=='B'){
	  row_pointers_B=row_pointers;
  }
  else {
	  printf("Fail il faut A ou B en deuxième argument\n");
  }

  printf("C'est bon\n");

  fclose(fp);
}

void write_png_file(char *filename, png_bytep *row_pointers_print) {
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
    width-100, height-100,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  // png_set_filler(png, 0, PNG_FILLER_AFTER);

  png_write_image(png, row_pointers_print);
  png_write_end(png, NULL);

  for(int y = 0; y < height; y++) {
    free(row_pointers_print[y]);
  }
  free(row_pointers_print);

  fclose(fp);
}

double flou_gauss_point(int x, int y, int sigma){
	return 1. / (2 * M_PI * sigma * sigma) * exp(-(x * x + y * y) / (2 * sigma * sigma));
}

void flou_stuct_png(int sigma, char c){

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) abort();

	png_infop info = png_create_info_struct(png);
	if(!info) abort();

	row_pointers_flou = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) {
	  row_pointers_flou[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	int mu=2*sigma;

	if(c=='A'){
	  row_pointers=row_pointers_A;
	}
	else if(c=='B'){
		row_pointers=row_pointers_B;
	}
	else {
		printf("Fail il faut A ou B en deuxième argument\n");
	}

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

				  if(y+i < 0 || x+j < 0 || y+i >= height-1 || x+j >= width-1){

				  }
				  else {

					  g=flou_gauss_point(i, j, sigma);
					  summGauss += g;

					  png_bytep px = &(row_pointers[y+i][(x+j)*4]);
					  resR += g * px[0];
					  resG += g * px[1];
					  resB += g * px[2];
					  resA += g * px[3];
				  }
			  }
		  }
		  // Ces 4 lignes rendent la création de l'image impossioble mais je ne sais pas pourquoi
				row_pointers_flou[y][x*4]=(int)(resR/summGauss);
				row_pointers_flou[y][x*4+1]=(int)(resG/summGauss);
				row_pointers_flou[y][x*4+2]=(int)(resB/summGauss);
				row_pointers_flou[y][x*4+3]=(int)(resA/summGauss);
		  //printf("%4d, %4d = RGBA(%d, %d, %d, %d)\n", x, y, px[0], px[1], px[2], px[3]);
		}

	}

	if(c=='A'){
	  row_pointers_flou_A=row_pointers_flou;
	}
	else if(c=='B'){
		row_pointers_flou_B=row_pointers_flou;
	}
	else {
		printf("Fail il faut A ou B en deuxième argument\n");
	}

	printf("C'est bon\n");


}

void print_row_pointer(png_bytep *row_pointers_print){

	printf("%d\n", row_pointers_print[0][0]);
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			  int R = row_pointers_print[y][x*4];
			  int G = row_pointers_print[y][x*4+1];
			  int B = row_pointers_print[y][x*4+2];
			  int A = row_pointers_print[y][x*4+3];


			  printf("%d / %d / %d / %d\n", R, G, B, A);
		}
	}
}

void deriv_row_pointer(png_bytep *row_pointers_base, char c){

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) abort();

	png_infop info = png_create_info_struct(png);
	if(!info) abort();

	row_pointers_deriv = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) {
	  row_pointers_deriv[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			double stencil[5];

			stencil[0]=1./12.;
			stencil[1]=-8./12.;
			stencil[2]=0.;
			stencil[3]=8./12.;
			stencil[4]=-1./12.;


			row_pointers_deriv[y][x*4]=0;
			row_pointers_deriv[y][x*4+1]=0;
			row_pointers_deriv[y][x*4+2]=0;
			row_pointers_deriv[y][x*4+3]=0;


			for(int i = -2; i<3; i++){

				if(y+i>height-1 || y+i<0 || x+i>width-1 || x+i<0){

				}
				else{
					row_pointers_deriv[y][x*4]+=row_pointers_base[y][(x+i)*4]*stencil[i+2];
					row_pointers_deriv[y][x*4]+=row_pointers_base[y+i][(x)*4]*stencil[i+2];

					row_pointers_deriv[y][x*4+1]+=row_pointers_base[y][(x+i)*4+1]*stencil[i+2];
					row_pointers_deriv[y][x*4+1]+=row_pointers_base[y+i][(x)*4+1]*stencil[i+2];

					row_pointers_deriv[y][x*4+2]+=row_pointers_base[y][(x+i)*4+2]*stencil[i+2];
					row_pointers_deriv[y][x*4+2]+=row_pointers_base[y+i][(x)*4+2]*stencil[i+2];

					row_pointers_deriv[y][x*4+3]+=row_pointers_base[y][(x+i)*4+3]*stencil[i+2];
					row_pointers_deriv[y][x*4+3]+=row_pointers_base[y+i][(x)*4+3]*stencil[i+2];
				}
			}

		}
	}

	if(c=='A'){
		row_pointers_deriv_A=row_pointers_deriv;
	}
	else if(c=='B'){
		row_pointers_deriv_B=row_pointers_deriv;
	}
	else {
		printf("Fail il faut A ou B en deuxième argument\n");
	}

	printf("C'est bon\n");
}

void diff_row_pointer(){

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) abort();

	png_infop info = png_create_info_struct(png);
	if(!info) abort();

	row_pointers_diff = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) {
	  row_pointers_diff[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}


	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {

			row_pointers_diff[y][x*4]=row_pointers_flou_A[y][x*4]-row_pointers_flou_B[y][x*4];
			row_pointers_diff[y][x*4+1]=row_pointers_flou_A[y][x*4+1]-row_pointers_flou_B[y][x*4+1];
			row_pointers_diff[y][x*4+2]=row_pointers_flou_A[y][x*4+2]-row_pointers_flou_B[y][x*4+2];
			row_pointers_diff[y][x*4+3]=row_pointers_flou_A[y][x*4+3]-row_pointers_flou_B[y][x*4+3];
		}
	}

	printf("C'est bon\n\n");
}

int main(int argc, char *argv[]) {

	if(argc<2){
		perror("Manque d'arguments : Deux images png à comparer");
		exit(1);
	}

	int sigma=4;

	printf("\n-------------------Chargement de l'image-------------------\n\n");


	read_png_file(argv[1], 'A');
	printf("Height : %d\nWidth : %d\n\n", height, width );


	read_png_file(argv[2], 'B');
	printf("Height : %d\nWidth : %d\n", height, width );

	//write_png_file("res.png", row_pointers);


	printf("\n----------------Application du flou Gaussien----------------\n\n");

	flou_stuct_png(sigma, 'A');
	flou_stuct_png(sigma, 'B');


	printf("\n--------------------Ecriture de l'image--------------------\n\n");

	//write_png_file("res.png", row_pointers_flou);

	printf("Nous avons malheuresement un problème d'écriture d'image.\nAprès de noumbreuses heures/années de recherche nous n'avons pu trouver la solution...\n");

	printf("\n---------------------Dérivée de l'image---------------------\n\n");

	printf("Dérivée en x et en y\n");
	deriv_row_pointer(row_pointers_flou_A, 'A');
	deriv_row_pointer(row_pointers_flou_B, 'B');
	//print_row_pointer(row_pointers_deriv);


	printf("\n---------------------Différences entre les deux images---------------------\n\n");

	printf("Différence entre les deux images\n");

	printf("Ca marchait mais ça ne marche plus...\n");

	//diff_row_pointer();
	/*
		png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(!png) abort();

		Cette partie a décidé de faire une erreur de segmentation

	*/

	//print_row_pointer(row_pointers_diff);

	return 0;
}
