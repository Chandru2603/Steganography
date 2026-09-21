#include <stdio.h>      // Provides file handling and input/output functions
#include <stdint.h>     // Provides fixed-size integer types like uint32_t
#include <string.h>     // Provides string handling functions

// Extract the least significant bit (LSB) from a byte
unsigned char get_lsb(unsigned char byte)
{
    return byte & 1;    // Return only the last bit of the byte
}

int main()
{
    FILE *file;         // File pointer used to access the BMP image

    unsigned char pixel[3];     // Stores Blue, Green and Red values of one pixel

    // Open the stego image in binary read mode
    file = fopen("images/stego.bmp", "rb");

    // Check whether the image was opened successfully
    if (file == NULL)
    {
        printf("Error opening stego.bmp\n");   // Display error message
        return 1;                              // Exit with error
    }

    /* BMP information */
    int32_t width;      // Stores the width of the BMP image
    int32_t height;     // Stores the height of the BMP image
    
    // Move to the width field in the BMP header
    fseek(file, 18, SEEK_SET);

    // Read the image width from the BMP header
    fread(&width, sizeof(width), 1, file);
    
    // Move to the height field in the BMP header
    fseek(file, 22, SEEK_SET);

    // Read the image height from the BMP header
    fread(&height, sizeof(height), 1, file);
    
    // Move to the beginning of the pixel data
    fseek(file, 54, SEEK_SET);

    // Calculate the number of bytes used by pixels in one row
    int row_bytes = width * 3;

    // Calculate the padding added to make each row a multiple of 4 bytes
    int padding = (4 - (row_bytes % 4)) % 4;

    /*
     * We need to read:
     *
     * STEG       = 4 bytes
     * Length     = 4 bytes
     * Message    = maximum 99 bytes
     *
     * Total = 108 bytes
     */
    unsigned char data[108] = {0};     // Stores the decoded hidden data

    int bit_index = 0;                 // Keeps track of the current decoded bit

    int total_bits = 108 * 8;          // Calculate total number of bits to decode

    // Move to the beginning of the BMP pixel data
    fseek(file, 54, SEEK_SET);

    // Read the BMP image row by row
    for (int row = 0; row < height; row++)
    {
        // Read every pixel in the current row
        for (int column = 0; column < width; column++)
        {
            // Read Blue, Green and Red values of one pixel
            fread(pixel, 1, 3, file);

            // Continue decoding until all required bits are collected
            if (bit_index < total_bits)
            {
                // Find which byte of the data array the bit belongs to
                int byte_index = bit_index / 8;

                // Find the bit position inside that byte
                int bit_position = 7 - (bit_index % 8);

                // Extract the hidden bit from the Blue channel
                unsigned char secret_bit =
                    get_lsb(pixel[0]);

                // Place the extracted bit into the correct position
                data[byte_index] |=
                    secret_bit << bit_position;

                // Move to the next bit
                bit_index++;
            }
        }

        // Skip the unused padding bytes at the end of the row
        fseek(file, padding, SEEK_CUR);
    }

    // Close the BMP file
    fclose(file);

    // Check whether the image contains the expected STEG signature
    if (data[0] != 'S' ||
        data[1] != 'T' ||
        data[2] != 'E' ||
        data[3] != 'G')
    {
        // Display an error if the image is not a valid stego image
        printf("Error: Invalid steganography image.\n");

        return 1;      // Exit with error
    }

    // Display the valid STEG signature
    printf("Magic: STEG\n");

    // Decode the 4-byte message length
    uint32_t length =
        ((uint32_t)data[4] << 24) |
        ((uint32_t)data[5] << 16) |
        ((uint32_t)data[6] << 8) |
        data[7];

    // Display the length of the hidden message
    printf("Message length: %u bytes\n", length);

    // Display the decoded secret message
    printf("Decoded message: ");

    // Print each character of the hidden message
    for (uint32_t i = 0; i < length; i++)
    {
        // Print one decoded character at a time
        printf("%c", data[8 + i]);
    }

    // Move to the next line after displaying the message
    printf("\n");

    return 0;       // End the program successfully
}