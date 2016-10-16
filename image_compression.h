

/* 
 * File:   image_compression.h
 * Author: r2d2
 *
 * Created on April 8, 2016, 10:53 PM
 */

#ifndef IMAGE_COMPRESSION_H
#define IMAGE_COMPRESSION_H

    //Macros compresor
    #define FORCE_EXTENSION 0   //de estar en 1 obligara que el archivo salga con la siguiente ext
    #define COMP_EXT	".taco"

    #define MIDDLE_X(x1,x2) (((x1)+(x2))/2)
    #define MIDDLE_Y(y1,y2) (((y1)+(y2))/2)

    #define LEFT_X(x1,x2)	((x1))
    #define RIGHT_X(x1,x2)	((x2))

    #define TOP_Y(y1,y2)	((y1))
    #define BOTTOM_Y(y1,y2)	((y2))

    #define PROMEDIO(x,y)	( ((x)+(y))/2 )

    #define RGB_SIZE	3
    #define RED_POS     0
    #define GREEN_POS	1
    #define BLUE_POS	2


    //Macros compartidas

    #define BRANCH_CHARACTER	'B'
    #define DRAW_CHARACTER	'N'

    
    bool compress_image(char*png_file_name,char *save_name,unsigned int threshold);
    bool decompress_image (char *image_to_decompress, char *new_image);
#endif /* IMAGE_COMPRESSION_H */

