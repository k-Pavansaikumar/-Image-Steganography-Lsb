#include <stdio.h>          // Standard input/output functions
#include <string.h>         // String handling functions
#include "encode.h"         // Encode related structures and prototypes
#include "common.h"         // Common macros and definitions

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint w, h;                                      // Variables to store width and height

    fseek(fptr_image, 18, SEEK_SET);                // Move file pointer to width field in BMP header
    fread(&w, sizeof(uint), 1, fptr_image);         // Read image width
    fread(&h, sizeof(uint), 1, fptr_image);         // Read image height

    
    return (w * h * 3);                             // Return total image capacity (RGB)
}

/* Open files */
Status open_files(EncodeInfo *encInfo)
{
    FILE *src  = fopen(encInfo->src_image_fname, "rb");   // Open source image
    FILE *sec  = fopen(encInfo->secret_fname, "rb");     // Open secret file
    FILE *dest = fopen(encInfo->stego_image_fname, "wb"); // Open output stego image

    if (!src || !sec || !dest)                      // Check if any file failed to open
    {
        return e_failure;                           // Return failure if any file is NULL
    }

    encInfo->fptr_src_image   = src;                // Store source image pointer
    encInfo->fptr_secret      = sec;                // Store secret file pointer
    encInfo->fptr_stego_image = dest;               // Store stego image pointer

    return e_success;                               // Files opened successfully
}

/* Read & validate args */
Status read_and_validate_encode_args(int argc, char *argv[], EncodeInfo *encInfo)
{
    if (argc < 4 || argc > 5)                        // Validate argument count
        return e_failure;

    if (strstr(argv[2], ".bmp") == NULL)             // Check if input image is BMP
        return e_failure;

    encInfo->src_image_fname = argv[2];              // Store source image name
    encInfo->secret_fname    = argv[3];              // Store secret file name

    char *extension = strrchr(argv[3], '.');         // Extract extension from secret file
    if (!extension)                                  // If extension not found
        return e_failure;

    strncpy(encInfo->extn_secret_file,               // Copy extension to structure
            extension,
            MAX_FILE_SUFFIX - 1);

    encInfo->extn_secret_file[MAX_FILE_SUFFIX - 1] = '\0'; // Ensure null termination

    encInfo->stego_image_fname =                     // Set output stego image name
        (argc == 5) ? argv[4] : "stego.bmp";

    return e_success;                                // Argument validation successful
}

/* Copy BMP header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];                                 // Buffer for BMP header

    rewind(fptr_src_image);                          // Move source pointer to start
    fread(header, 1, 54, fptr_src_image);            // Read 54-byte BMP header
    fwrite(header, 1, 54, fptr_dest_image);          // Write header to stego image

    return e_success;
}

/* Get file size */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);                        // Move to end of file
    uint len = ftell(fptr);                          // Get file size
    rewind(fptr);                                   // Reset file pointer

    return len;                                      // Return file size
}

/* Capacity check */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity =                        // Get image capacity
        get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file =                      // Get secret file size
        get_file_size(encInfo->fptr_secret);

    uint bits_required =                             // Calculate required bits
        (strlen(MAGIC_STRING) +
         4 +
         strlen(encInfo->extn_secret_file) +
         4 +
         encInfo->size_secret_file) * 8;

    return (encInfo->image_capacity > bits_required) // Check if capacity is sufficient
           ? e_success
           : e_failure;
}

/* Encode byte */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int bit = 7; bit >= 0; bit--)               // Loop through 8 bits
    {
        image_buffer[7 - bit] &= 0xFE;               // Clear LSB
        image_buffer[7 - bit] |= ((data >> bit) & 1);// Insert data bit into LSB
    }
    return e_success;
}

/* Encode integer */
Status encode_size_to_lsb(int data, char *image_buffer)
{
    for (int bit = 31; bit >= 0; bit--)              // Loop through 32 bits
    {
        image_buffer[31 - bit] &= 0xFE;              // Clear LSB
        image_buffer[31 - bit] |= ((data >> bit) & 1);// Insert size bit
    }
    return e_success;
}

/* Generic encode */
Status encode_data_to_image(char *data, int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image)
{
    char img_buf[8];                                 // Buffer to hold image bytes

    for (int idx = 0; idx < size; idx++)             // Loop through data bytes
    {
        fread(img_buf, 1, 8, fptr_src_image);        // Read 8 image bytes
        encode_byte_to_lsb(data[idx], img_buf);      // Encode one byte into LSBs
        fwrite(img_buf, 1, 8, fptr_stego_image);     // Write encoded bytes
    }

    return e_success;
}

/* Encode magic string */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    return encode_data_to_image((char *)magic_string, // Encode magic string
                                strlen(magic_string),
                                encInfo->fptr_src_image,
                                encInfo->fptr_stego_image);
}

/* Encode extension size */
Status encode_secret_extn_size(long extn_size, EncodeInfo *encInfo)
{
    char buf[32];                                    // Buffer for 32 bytes

    fread(buf, 1, 32, encInfo->fptr_src_image);      // Read image bytes
    encode_size_to_lsb(extn_size, buf);              // Encode extension size
    fwrite(buf, 1, 32, encInfo->fptr_stego_image);   // Write encoded bytes

    return e_success;
}

/* Encode extension */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    return encode_data_to_image((char *)file_extn,   // Encode extension string
                                strlen(file_extn),
                                encInfo->fptr_src_image,
                                encInfo->fptr_stego_image);
}

/* Encode file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buf[32];                                    // Buffer for file size

    fread(buf, 1, 32, encInfo->fptr_src_image);      // Read image bytes
    encode_size_to_lsb(file_size, buf);              // Encode file size
    fwrite(buf, 1, 32, encInfo->fptr_stego_image);   // Write encoded bytes

    return e_success;
}

/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char data, img_buf[8];                           // Variables for secret data and image bytes

    rewind(encInfo->fptr_secret);                    // Reset secret file pointer

    while (fread(&data, 1, 1, encInfo->fptr_secret) == 1) // Read each byte from secret file
    {
        fread(img_buf, 1, 8, encInfo->fptr_src_image);    // Read image bytes
        encode_byte_to_lsb(data, img_buf);                // Encode secret byte
        fwrite(img_buf, 1, 8, encInfo->fptr_stego_image); // Write encoded bytes
    }

    return e_success;
}

/* Copy remaining image */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char byte;                                       // Variable to store image byte

    while (fread(&byte, 1, 1, fptr_src) == 1)        // Read remaining bytes
    {
        fwrite(&byte, 1, 1, fptr_dest);              // Copy to output image
    }

    return e_success;
}

/* Main encoding flow */
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) != e_success)            // Open all required files
        return e_failure;

    if (check_capacity(encInfo) != e_success)        // Check image capacity
        return e_failure;

    copy_bmp_header(encInfo->fptr_src_image,         // Copy BMP header
                    encInfo->fptr_stego_image);

    encode_magic_string(MAGIC_STRING, encInfo);      // Encode magic string
    encode_secret_extn_size(strlen(encInfo->extn_secret_file), encInfo); // Encode extension size
    encode_secret_file_extn(encInfo->extn_secret_file, encInfo);         // Encode extension
    encode_secret_file_size(encInfo->size_secret_file, encInfo);         // Encode file size
    encode_secret_file_data(encInfo);                 // Encode secret data

    copy_remaining_img_data(encInfo->fptr_src_image, // Copy remaining image bytes
                            encInfo->fptr_stego_image);

    return e_success;                                // Encoding completed
}
