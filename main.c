#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "image_compression.h"


#define	TERMINAL_COMPR	0
#define TERMINAL_DECOMP 0
#define TEST_MODE		1

///Peroooo que main hermosoooo 
int main(int argc, char *argv[])
{
	
	#if TERMINAL_COMPR && (!TEST_MODE && !TERMINAL_DECOMP)

	if(argc==4)//img name, compression output, threshold
		compress_image(argv[1],argv[2],atoi(argv[3]));
		
	#endif

	#if	TERMINAL_DECOMP && (!TERMINAL_COMPR && !TEST_MODE)

	if(argc==3)//img name, compression output threshold
		decompress_image(argv[1],argv[2]);

	#endif
	
	#if TEST_MODE && (!TERMINAL_COMPR && !TERMINAL_DECOMP)
	printf("Empezando compresion imagen\n");
	if(compress_image("obama_eagle.png","compressed.taco",40)) //numero es el threshold
	{
		printf("Compresion terminada, descomprimiendo\n");
		if(decompress_image("compressed.taco","obama.png"))
			printf("Va como piña colada!!\n");
		else
			perror("ERROR: No se pudo descomprimir la imagen ");
	}
	else
		printf("No se pudo comprimir la imagen\n");
	
	#endif
	
	return 0;
}




