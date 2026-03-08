**Image Steganography using LSB Technique**

This project implements Image Steganography using the Least Significant Bit (LSB) technique to hide and retrieve secret data inside BMP images.

# Features

Embed secret data inside a BMP image

Extract hidden data from a stego image

Uses bitwise manipulation to modify image pixel bits

Supports command-line arguments for encoding and decoding operations

# Technologies Used

C Programming

File Handling

Bitwise Operations

Learning Outcomes

Understanding the internal structure of BMP image files

Implementing data hiding using the Least Significant Bit method

Performing bit-level data manipulation in C

Working with file input/output operations

# How to Run

Compile make

# Encode

./encode <source_image.bmp> <secret_file> <output_image.bmp>

# Decode

./decode <stego_image.bmp>
