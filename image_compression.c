#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "file_access.h"
#include "lodepng.h"
#include "image_compression.h"



static void compress_image_recursion(unsigned char*imagen_24bits,FILE *img_comp,unsigned int threshold,
				unsigned int x1, unsigned int y1,unsigned int x2, unsigned int y2,uint32_t dimension);

static unsigned int puntaje_rgb_sector(unsigned char*imagen_24bits,unsigned int x1, unsigned int y1,
								unsigned int x2, unsigned int y2,uint32_t dimension,unsigned char rgb[3]);

static void draw_pixels (unsigned char * imagen_24bits,unsigned int X1, unsigned int Y1, unsigned int X2, unsigned int Y2, 
							uint32_t w_size, unsigned char red, unsigned char green, unsigned char blue);

static bool decompress_recursion (unsigned char * image_arrey, unsigned int X1, unsigned int Y1, unsigned int X2, unsigned int Y2,
									FILE *image_file,uint32_t image_width, uint32_t image_height);
/*
 *	Tanto el compresor y descompresor pueden recibir imagenes de CUALQUIER dimension, y proporcion
 *  al trabajar con un algoritmo que emplea puntos de la imagen no se ve afectado por las dimensiones de la misma. 
 *  Y de hecho resulto muy ventajoso esto en el momento en que se decibio ampliarlo hacia cualquier tamaño, ya que
 * solamente requirio pequeños cambios.
 * 
 * Input en orden:
 *	-Png_file_name: El nombre del archivo png que sea desea comprimir
 *	-Save_name: El nombre con el que se desea guardar la imagen comprimida
 *	-Threshold: nivel de compression 0-765 cuanto mas alto menor sera la compresion
 * Output:
 *	Bool true si se pudo comprimir 
 *		 fasle sino se pudo comprimir
 * 
 * 
 * 
 * No resulta necesario un control de error de threshold pq el mismo de pasar de los 765 dara una imagen de un solo color
 */

bool compress_image(char*png_file_name,char *save_name,unsigned int threshold)
{
	bool program_success=false;
	FILE *img_comprimida=NULL;
	unsigned int width,height;
	unsigned char *imagen_24bits=NULL;

	if(save_name!=NULL)
		lodepng_decode24_file(&imagen_24bits,&width,&height,png_file_name);
	if(save_name!=NULL && imagen_24bits!=NULL)
	{
		#if FORCE_EXTENSION
		char *real_save_name=malloc(strlen(save_name)+strlen(COMP_EXT));
		if(real_save_name==NULL)
			real_save_name=save_name;
		else{
			strcpy(real_save_name,save_name);
			strcat(real_save_name,COMP_EXT);//Todo esto agrega la extension
		}
		img_comprimida=fopen(real_save_name,"wb+");
		free(real_save_name);
		#else
		img_comprimida=fopen(save_name,"wb+");
		#endif
		
		if(img_comprimida!=NULL)
		{
			
			f_gen_write(img_comprimida,&width,sizeof(uint32_t));//Guardamos tamaño imagen 
			f_gen_write(img_comprimida,&height,sizeof(uint32_t));//Guardamos tamaño imagen
			compress_image_recursion(imagen_24bits,img_comprimida,threshold,0,0,width,height,width);
			fclose(img_comprimida);

			program_success=true;

		}
		free(imagen_24bits);

	}
	return program_success;
}


/*	Es llamada por compress_image lleva a cabo la rcursion 
 * 
 *	Input en orden:
 *		-imagen_24bits: el arreglo de la imagen en formato RGB
 *		-img_comp:el archivo abierto donde se guardar comprimida en forma de texto
 *		-threshold: cuanto se comprimira de 0-765 siendo 0 sin perdida de calida
 *		-x1,y1:En cada recursion contendran el punto de arriba a la izquierda del rectangulo de imagen que se esta analizando
 *		-x2,y2:En cada recursion contendran el punto de abajo a la derecha del rectangulo de imagen que se esta analizando
 *		-dimension: como trabaja con alto en los puntos no es necesario pasarlo explicitamente pero si la dimension para saber hasta
 *				que punto llegar en cada fila del areglo
 * 
 */

void compress_image_recursion(unsigned char*imagen_24bits,FILE *img_comp,unsigned int threshold,
				unsigned int x1, unsigned int y1,unsigned int x2, unsigned int y2,uint32_t dimension)
{
	unsigned char rgb_array[3];

	if(puntaje_rgb_sector(imagen_24bits,x1,y1,x2,y2,dimension,rgb_array)>threshold)
	{
		
		fputc(BRANCH_CHARACTER,img_comp);
		//printf("Se divide\n");
		//Se divide la imagen en Cuatro partes iguales y eso se sigue compimiendo
		compress_image_recursion(imagen_24bits,img_comp,threshold,LEFT_X(x1,x2),TOP_Y(y1,y2),MIDDLE_X(x1,x2),MIDDLE_Y(y1,y2),dimension);
		compress_image_recursion(imagen_24bits,img_comp,threshold,MIDDLE_X(x1,x2),TOP_Y(y1,y2),RIGHT_X(x1,x2),MIDDLE_Y(y1,y2),dimension);
		compress_image_recursion(imagen_24bits,img_comp,threshold,LEFT_X(x1,x2),MIDDLE_Y(y1,y2),MIDDLE_X(x1,x2),BOTTOM_Y(y1,y2),dimension);
		compress_image_recursion(imagen_24bits,img_comp,threshold,MIDDLE_X(x1,x2),MIDDLE_Y(y1,y2),RIGHT_X(x1,x2),BOTTOM_Y(y1,y2),dimension);

	}
	else
	{
		fputc(DRAW_CHARACTER,img_comp);
		//printf("pinta\n");
		f_gen_write(img_comp,rgb_array,3);
	}


}

/*
 * Es llamada por compress_image_recursion calcula la diferencia de colores en un sector
 * 
 *	Input en orden:
 *		-imagen_24bits: el arreglo de la imagen en formato RGB
 *		-x1,y1:En cada recursion contendran el punto de arriba a la izquierda del rectangulo de imagen que se esta analizando
 *		-x2,y2:En cada recursion contendran el punto de abajo a la derecha del rectangulo de imagen que se esta analizando
 *		-dimension: como trabaja con alto en los puntos no es necesario pasarlo explicitamente pero si la dimension para saber hasta
 *				que punto llegar en cada fila del areglo
 *		-rgb[3]: un arreglo donde se guardan los valores promedio de cada color (colormax+colormin)/2
 * 
 * Output: Puntaje rgb
 *		-Devuelve un valor entre 0-756 con la diferencia de colores entre ese sector cuanto mas bajo sea este valor menor sera la diferencia
 * 
 * 
 * 
 */
unsigned int puntaje_rgb_sector(unsigned char*imagen_24bits,unsigned int x1, unsigned int y1,
								unsigned int x2, unsigned int y2,uint32_t dimension,unsigned char rgb[3])
{
	int ancho=x1,alto=y1;
	
	unsigned char rmax,rmin,gmax,gmin,bmax,bmin;
	
	rmax=rmin=imagen_24bits[ (y1*dimension+x1)*RGB_SIZE ];
	gmax=gmin=imagen_24bits[ (y1*dimension+x1)*RGB_SIZE +GREEN_POS];
	bmax=bmin=imagen_24bits[ (y1*dimension+x1)*RGB_SIZE +BLUE_POS];

	
	for(alto=y1; alto<y2;alto++)
		for(ancho=x1;ancho<x2;ancho++)
		{
				if( imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE ]>rmax)
					rmax=imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE ];
				
				else if( imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE ]<rmin) 
					rmin=imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE ];

					
				if( imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE +GREEN_POS ]>gmax)
					gmax=imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE + GREEN_POS ];
				
				else if( imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE + GREEN_POS]<gmin) 
					gmin=imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE + GREEN_POS ];
				
				
				if( imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE +BLUE_POS ] >bmax)
					bmax=imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE + BLUE_POS ];
				
				else if( imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE + BLUE_POS] <bmin) 
					bmin=imagen_24bits[ (alto*dimension+ancho)*RGB_SIZE + BLUE_POS];

		}
	
	rgb[RED_POS]=PROMEDIO(rmax,rmin);
	rgb[GREEN_POS]=PROMEDIO(gmax,gmin);
	rgb[BLUE_POS]=PROMEDIO(bmax,bmin);
		
	
	//printf("Puntaje %u\n",rmax-rmin+gmax-gmax+bmax-bmin);
	return rmax-rmin+gmax-gmax+bmax-bmin;//PUNTAJE
	
	
}

/*
 *	Tanto el compresor y descompresor pueden recibir imagenes de CUALQUIER dimension, y proporcion
 *  al trabajar con un algoritmo que emplea puntos de la imagen no se ve afectado por las dimensiones de la misma. 
 *  Y de hecho resulto muy ventajoso esto en el momento en que se decibio ampliarlo hacia cualquier tamaño, ya que
 * solamente requirio pequeños cambios(pasar alto y no solo ancho).
 *  
 * Input en orden:
 *	-image_to_decompress: El nombre del archivo png que sea desea descomprimir
 *	-Save_name: El nombre con el que se desea guardar la imagen desomprimida en formato png
 * Output:
 *	Bool true si se pudo comprimir 
 *		 fasle sino se pudo comprimir
 * 
 * Llama a decompress_recursion que realiza la verdadera compresion esta es la funcion user friendly que se 
 * encarga de  abrir el archivo, crear un arreglo que luego se transformar a png,etc
 * 
 * 
 */
bool decompress_image (char *image_to_decompress, char *new_image)
{
	bool program_success=false;
	unsigned char * image_array = NULL; //definimos el arreglo imagen en NULL
	uint32_t width, height; 
	FILE *image_file;	

	image_file = fopen(image_to_decompress, "rb+");
	if(image_file == NULL)
		perror("ERROR: No se pudo abrir el archivo para descomprimir\n");
		//primer caso de error no existe el archivo que se quiere descomprimir 
	

	//el archivo existe. Ahora tomamos el ancho y alto de la imagen que se quiere descomprimir
	else if(f_gen_read(image_file, &width, sizeof(uint32_t)) < sizeof(uint32_t))
		perror("ERROR:No se pudo leer el ancho de la imagen\n"); //caso de error, el ancho que se levanto del archivo es mas chico que 4 bytes


	else if(f_gen_read(image_file, &height, sizeof(uint32_t)) < sizeof(uint32_t))
		perror("ERROR:No se pudo leer el alto de la imagen\n"); //caso de error, el alto que se levanto del archivo es mas chico que 4 bytes
	
	//hacemos un malloc al arreglo donde estaran guardados todos los pixeles. Tomamos en cuenta el ancho, alto y el RGB + el alpha
	else if( (image_array = malloc(width*height*4*sizeof(unsigned char)))==NULL )
		perror("ERROR: No se pudo reservar memoria descomprimir la imagen\n");
	//Ahora que el arreglo esta definido y con memoria dinamica, se puede llamar a la funcion recursiva que descomprime la imagen.
	//Para comenzar con la recursion llamamos a decompress_recursion con las coordenadas (0,0) (x,y), el ancho, el alto, el puntero del arreglo, ancho y alto nuevamente. 

	else if(decompress_recursion(image_array,0,0,width,height,image_file,width,height))
	{	
		//Si no hay error, se llama a encode para crear el png con las mismas dimensiones de la imagen compresa.
		lodepng_encode32_file(new_image,image_array,width,height);
		free(image_array); //liberamos toda la memoria utilizada para el arreglo.
		program_success=true;
	}
	else
		perror("Error: No se puedo descomprir el archivo correctamente \n");

	return program_success;
}


/*	Es llamada por decompress_image lleva a cabo la recursion 
 * 
 *	Input en orden:
 *		-image_arrey: el arreglo de la imagen en formato RGBA
 *		-x1,y1:En cada recursion contendran el punto de arriba a la izquierda del rectangulo de imagen que se esta analizando
 *		-x2,y2:En cada recursion contendran el punto de abajo a la derecha del rectangulo de imagen que se esta analizando
 *		-image_width, image_height: ancho y altura de la imagen/
 *		-image_file:el archivo abierto donde se guardar comprimida en forma de texto

 * Output: Bool true(1) en caso de que no se produzca error de lo contrario false(0)
 */
bool decompress_recursion (unsigned char * image_arrey, unsigned int X1, unsigned int Y1, unsigned int X2, unsigned int Y2, FILE *image_file,uint32_t image_width, uint32_t image_height)
{
	unsigned char character,red, green, blue;
	bool ret=false;

	//Se definio que si se encuentra una N, hay una hoja en el arbol hipotetico, inmediatamente se encuentra los tres bytes del RGB. 
	//Si hay una B significa que hay un branch en el arbol hipotetico.
	character= fgetc(image_file); 
	if(character == DRAW_CHARACTER)	
	{
		red=fgetc(image_file); //se toma el byte del R
		green=fgetc(image_file); //se toma el byte del G
		blue=fgetc(image_file); //se toma el byte del B
		draw_pixels(image_arrey,X1,Y1,X2,Y2,image_width,red,green,blue); //se envia a dibujar el rectangulo con el RGB tomados anteriormente
		//Observar que no es necesario pasar el alto de la imagen al draw_pixels.
	}
	else if(character == BRANCH_CHARACTER)
	{
		if(decompress_recursion(image_arrey,LEFT_X(X1,X2),TOP_Y(Y1,Y2),MIDDLE_X(X1,X2),MIDDLE_Y(Y1,Y2),image_file, image_width, image_height)==false);
		else if(decompress_recursion(image_arrey,MIDDLE_X(X1,X2),TOP_Y(Y1,Y2),RIGHT_X(X1,X2), MIDDLE_Y(Y1,Y2),image_file, image_width, image_height)==false);
		else if(decompress_recursion(image_arrey,LEFT_X(X1,X2), MIDDLE_Y(Y1,Y2),MIDDLE_X(X1,X2), BOTTOM_Y(Y1,Y2),image_file, image_width, image_height)==false);
		else if(decompress_recursion(image_arrey,MIDDLE_X(X1,X2), MIDDLE_Y(Y1,Y2),RIGHT_X(X1,X2), BOTTOM_Y(Y1,Y2),image_file, image_width, image_height)==false);		
		else ret=true;
	}
	ret=true;
	return ret;
}

/*
 * Deja los valores de rgb de los pixeles en un arreglo lineal.
 * 
 */

void draw_pixels (unsigned char * image_arrey,unsigned int X1, unsigned int Y1, unsigned int X2, unsigned int Y2, uint32_t w_size, unsigned char red, unsigned char green, unsigned char blue)
{ 
	unsigned int  counter_row, counter_column;
	for(counter_row=Y1; counter_row<Y2; counter_row++)
		for(counter_column=X1; counter_column<X2; counter_column++)
			{
				image_arrey[((counter_column*4)+(4*w_size*counter_row))+0] = red; //primer elemento es el byte del red
				image_arrey[((counter_column*4)+(4*w_size*counter_row))+1] = green; //el segundo elemento es el byte del green
				image_arrey[((counter_column*4)+(4*w_size*counter_row))+2] = blue; //el tercero es el byte del blue
				image_arrey[((counter_column*4)+(4*w_size*counter_row))+3] = 0xff; //alpha en 100
			}
}
