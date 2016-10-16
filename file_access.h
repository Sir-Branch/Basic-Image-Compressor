/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   file_access.h
 * Author: r2d2
 *
 * Created on April 8, 2016, 10:10 PM
 */

#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

    int f_gen_write(FILE *dest_file,void *source,size_t num_bytes);
    int f_gen_read(FILE *source_file,void *dest,size_t num_bytes);

#endif /* FILE_ACCESS_H */

