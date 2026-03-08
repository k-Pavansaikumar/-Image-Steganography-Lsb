#include <stdio.h>          // Provides printf(), scanf() functions
#include <string.h>         // Provides strcmp() for string comparison
#include "encode.h"         // Declarations related to encoding
#include "decode.h"         // Declarations related to decoding
#include "types.h"          // User-defined data types and enums

/* Prototype to decide encode / decode operation */
OperationType get_operation(char *argv[]);

int main(int argc, char *argv[])
{
    /* Check whether minimum arguments are provided */
    if (argc < 2)                                // Operation flag missing
    {
        printf("Usage Instructions:\n");         // Display help message
        printf("./a.out -e <src.bmp> <secret.txt> [stego.bmp]\n");
        printf("./a.out -d <stego.bmp> [output_file]\n");
        return e_failure;                        // Exit program with failure
    }

    /* Determine which operation user requested */
    OperationType op = get_operation(argv);      // Fetch encode/decode option

    /* -------------------- ENCODE SECTION -------------------- */
    if (op == e_encode)                          // If user selected encoding
    {
        EncodeInfo encode_data;                  // Structure to store encode info

        /* Validate encoding arguments */
        if (read_and_validate_encode_args(argc, argv, &encode_data) == e_success)
        {
            printf("Encode: Arguments validated successfully\n");

            /* Perform encoding process */
            if (do_encoding(&encode_data) == e_success)
            {
                printf("Encode: Encoding completed successfully\n");
            }
            else
            {
                printf("Encode: Encoding failed\n");
            }
        }
        else
        {
            printf("Encode: Argument validation failed\n");
        }
    }

    /* -------------------- DECODE SECTION -------------------- */
    else if (op == e_decode)                     // If user selected decoding
    {
        DecodeInfo decode_data;                  // Structure to store decode info

        /* Validate decoding arguments */
        if (read_and_validate_decode_args(argc, argv, &decode_data) == e_success)
        {
            printf("Decode: Arguments validated successfully\n");

            /* Perform decoding process */
            if (do_decoding(&decode_data) == e_success)
            {
                printf("Decode: Decoding completed successfully\n");
            }
            else
            {
                printf("Decode: Decoding failed\n");
            }
        }
        else
        {
            printf("Decode: Argument validation failed\n");
        }
    }

    /* -------------------- INVALID OPTION -------------------- */
    else                                         // Unsupported flag provided
    {
        printf("Invalid operation selected\n");  // Inform user
        printf("Use -e for encode or -d for decode\n");
        return e_failure;                        // Exit with failure
    }

    return e_success;                            // Program executed successfully
}

/* Function to identify operation type */
OperationType get_operation(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)              // Compare with encode flag
    {
        return e_encode;                         // Return encode enum
    }
    else if (strcmp(argv[1], "-d") == 0)         // Compare with decode flag
    {
        return e_decode;                         // Return decode enum
    }
    else
    {
        return e_unsupported;                   // Invalid operation
    }
}
