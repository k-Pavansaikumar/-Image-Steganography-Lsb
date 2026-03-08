#include <stdio.h>          // Standard I/O functions
#include <stdlib.h>         // Standard library functions
#include <string.h>         // String handling functions
#include "decode.h"         // Decode structure and prototypes
#include "common.h"         // Common macros and definitions

unsigned int file_size = 0; // Global variable to store decoded file size

/* Open stego image file */
Status open_file(DecodeInfo *decInfo)
{
    FILE *fp = fopen(decInfo->stego_image_fname, "rb"); // Open stego image in read-binary mode

    if (fp == NULL)                                     // Check if file opening failed
    {
        printf("File is not opened");                   // Print error message
        return e_failure;                               // Return failure status
    }

    decInfo->fptr_stego_image = fp;                     // Store file pointer in structure
    return e_success;                                   // Return success status
}

/* Read and validate decode arguments */
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if (argc < 3 || argc > 4)                            // Validate argument count
    {
        return e_failure;
    }

    if (strstr(argv[2], ".bmp") == NULL)                 // Ensure input file is BMP
    {
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];                // Store stego image filename

    if (argc == 4)                                       // If output filename is provided
    {
        if (strchr(argv[3], '.') != NULL)                // Output name should not have extension
        {
            return e_failure;
        }
        decInfo->secret_fname = argv[3];                 // Assign output filename
    }
    else
    {
        decInfo->secret_fname = "decode_secret";         // Default output filename
    }

    return e_success;                                    // Validation successful
}

/* Main decoding flow */
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_file(decInfo) != e_success)                 // Open stego image
        return e_failure;

    if (skip_bmp_header(decInfo->fptr_stego_image) != e_success) // Skip BMP header
        return e_failure;

    if (decode_magic_string(MAGIC_STRING, decInfo) != e_success) // Verify magic string
        return e_failure;

    if (decode_secret_extn_size(0, decInfo) != e_success) // Decode extension size
        return e_failure;

    char extn[MAX_FILE_SUFFIX];                          // Buffer to store file extension

    if (decode_secret_file_extn(extn, decInfo) != e_success) // Decode file extension
        return e_failure;

    if (decode_secret_file_size(0, decInfo) != e_success) // Decode secret file size
        return e_failure;

    if (decode_secret_file_data(decInfo) != e_success)    // Decode secret file data
        return e_failure;

    return e_success;                                     // Decoding completed successfully
}

/* Skip BMP header */
Status skip_bmp_header(FILE *fptr_dest_image)
{
    fseek(fptr_dest_image, 54, SEEK_SET);                 // Move file pointer past 54-byte BMP header
    return e_success;
}

/* Decode magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char decoded_data[16] = {0};                          // Buffer to store decoded magic string

    if (decode_image_to_data(decoded_data,                // Decode image LSBs to data
                             strlen(magic_string),
                             decInfo->fptr_stego_image) != e_success)
    {
        return e_failure;
    }

    if (strcmp(decoded_data, magic_string) == 0)          // Compare decoded and original magic string
    {
        return e_success;
    }

    return e_failure;
}

/* Decode image data to bytes */
Status decode_image_to_data(char *data, int size, FILE *fptr_stego_image)
{
    char image_buffer[8];                                 // Buffer to store 8 image bytes

    for (int i = 0; i < size; i++)                        // Loop for each data byte
    {
        fread(image_buffer, 1, 8, fptr_stego_image);      // Read 8 bytes from image
        data[i] = decode_lsb_to_byte(image_buffer);       // Decode 1 byte from LSBs
    }

    return e_success;
}

/* Decode LSBs to one byte */
char decode_lsb_to_byte(char *image_buffer)
{
    char ch = 0;                                          // Variable to store decoded byte

    for (int i = 0, pos = 7; pos >= 0; pos--, i++)        // Loop through 8 bits
    {
        ch |= ((image_buffer[i] & 1) << pos);             // Extract LSB and place in correct bit
    }

    return ch;                                            // Return decoded character
}

/* Decode extension size */
Status decode_secret_extn_size(long extn_size, DecodeInfo *decInfo)
{
    char image_buffer[32];                                // Buffer to read 32 image bytes

    fread(image_buffer, 1, 32, decInfo->fptr_stego_image);// Read 32 bytes from image
    extn_size = decode_size_to_lsb(image_buffer);         // Decode extension size

    decInfo->size_secret_file = extn_size;                // Store extension size

    return (extn_size > 0) ? e_success : e_failure;       // Validate extension size
}

/* Decode 32-bit size */
int decode_size_to_lsb(char *image_buffer)
{
    int value = 0;                                        // Variable to store decoded size

    for (int i = 0, pos = 31; pos >= 0; pos--, i++)       // Loop for 32 bits
    {
        value |= ((image_buffer[i] & 1) << pos);          // Extract LSB and reconstruct integer
    }

    return value;                                         // Return decoded size
}

/* Decode secret file extension */
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo)
{
    char image_buffer[8];                                 // Buffer to read image bytes

    for (int i = 0; i < decInfo->size_secret_file; i++)   // Loop for extension characters
    {
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image); // Read 8 bytes
        file_extn[i] = decode_lsb_to_byte(image_buffer);  // Decode character
    }

    file_extn[decInfo->size_secret_file] = '\0';          // Null terminate string
    strcat(decInfo->secret_fname, file_extn);             // Append extension to filename

    printf("%s\n", file_extn);                             // Print decoded extension
    return e_success;
}

/* Decode secret file size */
Status decode_secret_file_size(long file_size, DecodeInfo *decInfo)
{
    char image_buffer[32];                                // Buffer for reading 32 bytes

    fread(image_buffer, 1, 32, decInfo->fptr_stego_image);// Read size data
    file_size = decode_size_to_lsb(image_buffer);         // Decode file size

    decInfo->file_size = file_size;                       // Store file size

    return (file_size > 0) ? e_success : e_failure;       // Validate file size
}

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    FILE *fp = fopen(decInfo->secret_fname, "wb");        // Open secret file for writing

    if (fp == NULL)                                       // Check file open failure
    {
        return e_failure;
    }

    char image_buffer[8];                                 // Buffer to read image bytes
    char decoded_char;                                    // Variable to store decoded data

    for (int i = 0; i < decInfo->file_size; i++)          // Loop through secret file size
    {
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image); // Read 8 bytes
        decoded_char = decode_lsb_to_byte(image_buffer);  // Decode byte
        fwrite(&decoded_char, 1, 1, fp);                  // Write decoded byte to file
    }

    fclose(fp);                                           // Close secret file
    return e_success;
}
