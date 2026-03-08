#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

/* 
 * Structure to store information required for
 * decoding secret file from stego Image
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 5

typedef struct _DecodeInfo
{
    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;
    long file_size;
    int extn_size;


    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;


/* ---------------- Decoding function prototypes ---------------- */

/* Read and validate decode args from argv */
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Open output secret file */
Status open_file(DecodeInfo *decInfo);

/* Skip bmp image header */
Status skip_bmp_header(FILE *fptr_dest_image);

/* Decode magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_secret_extn_size(long extn_size, DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(long file_size, DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode encoded image data to secret data */
Status decode_image_to_data(char *data, int size, FILE *fptr_stego_image);

/* Decode one byte from LSBs */
char decode_lsb_to_byte(char *image_buffer);

/* Decode integer value from LSBs */
int decode_size_to_lsb(char *image_buffer);

#endif
