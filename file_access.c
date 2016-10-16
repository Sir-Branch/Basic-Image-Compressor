/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdlib.h>
#include <stdio.h>
#define ERROR -1


//YA FUNCIONA JOYA
int f_gen_read(FILE *source_file,void *dest,size_t num_bytes)
{
	int bytes_leidos;
	unsigned int char_leido;

	if(source_file==NULL || dest ==NULL)
		bytes_leidos=ERROR;
	else
		for(bytes_leidos=0;bytes_leidos<num_bytes;bytes_leidos++)
			if((char_leido=fgetc(source_file))==EOF)
				break;
			else
				((char*)dest)[bytes_leidos]=char_leido;

	return bytes_leidos;


}

int f_gen_write(FILE *dest_file,void *source,size_t num_bytes)
{

	int bytes_leidos;
	unsigned int char_leido;

	if(dest_file==NULL || source ==NULL)
		bytes_leidos=ERROR;
	else
		for(bytes_leidos=0;bytes_leidos<num_bytes;bytes_leidos++)
			if(fputc(((char*)source)[bytes_leidos],dest_file)==EOF)
			{
				--bytes_leidos;
				break;
			}

	return bytes_leidos;

}