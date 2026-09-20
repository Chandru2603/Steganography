#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Sets only the least significant bit (LSB) of a byte, keeping the other 7 bits untouched
void set_lsb(unsigned char *byte, unsigned char bit)
{
    *byte = (*byte & 0xFE) | bit;
}

int main()
{
    FILE *input;
    FILE *output;

    char message[100];
    // Buffer to hold the secret message the user wants to hide
    printf("Enter secret message: ");
    fgets(message, sizeof(message), stdin);

    // Strip the trailing newline character left by fgets
    message[strcspn(message, "\n")] = '\0';

    // Length of the message in bytes (used for encoding the header)
    uint32_t length = strlen(message);

    // Buffer to store the 54-byte BMP file header
    unsigned char header[54];
    // Holds one pixel's BGR bytes at a time while scanning the image
    unsigned char pixel[3];

    /* Our hidden data */
    // Payload buffer: 4-byte magic tag + 4-byte length + up to 100 bytes of message
    unsigned char data[108] = {0};

    /* 4 bytes: STEG */
    // Magic signature "STEG" used to identify/validate hidden data later
    data[0] = 'S';
    data[1] = 'T';
    data[2] = 'E';
    data[3] = 'G';

    /* 4 bytes: message length */
    // Store message length as 4 bytes in big-endian order
    data[4] = (length >> 24) & 0xFF;
    data[5] = (length >> 16) & 0xFF;
    data[6] = (length >> 8) & 0xFF;
    data[7] = length & 0xFF;

    /* Message */
    // Copy the actual message bytes right after the tag and length fields
    memcpy(&data[8], message, length);

    // Total number of payload bytes to hide (tag + length + message, sans STEG's 4 bytes double count fix below)
    int total_data_bytes = 8 + length;
    // Total number of bits to encode, since we hide 1 bit per pixel's blue channel
    int total_bits = total_data_bytes * 8;

    // Open the source BMP image for reading in binary mode
    input = fopen("images/original.bmp", "rb");

    if (input == NULL)
    {
        printf("Error opening original.bmp\n");
        return 1;
    }

    // Create the output BMP file that will contain the hidden message
    output = fopen("images/stego.bmp", "wb");

    if (output == NULL)
    {
        printf("Error creating stego.bmp\n");
        fclose(input);
        return 1;
    }

    /* Copy BMP header */
    // Copy the 54-byte BMP header unchanged from input to output
    fread(header, 1, 54, input);
    fwrite(header, 1, 54, output);

    /*
     * Our image:
     * 599 pixels per row
     * 3 bytes per pixel
     * 3 padding bytes per row
     */
    int32_t width;
    int32_t height;
    
    // Read image width from BMP header offset 18
    fseek(input, 18, SEEK_SET);
    fread(&width, sizeof(width), 1, input);
    // Read image height from BMP header offset 22
    fseek(input, 22, SEEK_SET);
    fread(&height, sizeof(height), 1, input);
    
    // Move file pointer to the start of pixel data (right after 54-byte header)
    fseek(input, 54, SEEK_SET);

    // Number of bytes per row of pixel data (before padding)
    int row_bytes = width * 3;
    // BMP rows are padded to a multiple of 4 bytes; compute how many pad bytes exist
    int padding = (4 - (row_bytes % 4)) % 4;

    // Tracks how many bits of the payload have been embedded so far
    int bit_index = 0;

    /* Process every row */
    for (int row = 0; row < height; row++)
    {
        /* Process every pixel */
        for (int column = 0; column < width; column++)
        {
            // Read one pixel's BGR bytes
            fread(pixel, 1, 3, input);

            /* Encode while secret data remains */
            if (bit_index < total_bits)
            {
                // Determine which byte in the payload this bit belongs to
                int byte_index = bit_index / 8;
                // Determine which bit position within that byte (MSB-first order)
                int bit_position = 7 - (bit_index % 8);

                // Extract the specific bit to hide
                unsigned char secret_bit =
                    (data[byte_index] >> bit_position) & 1;

                // Embed the bit into the blue channel's LSB
                set_lsb(&pixel[0], secret_bit);

                // Move on to the next bit for the next pixel
                bit_index++;
            }

            // Write the (possibly modified) pixel to the output file
            fwrite(pixel, 1, 3, output);
        }

        /* Copy padding unchanged */
        for (int i = 0; i < padding; i++)
        {
            unsigned char pad;

            // Padding bytes carry no pixel data, so copy them as-is
            fread(&pad, 1, 1, input);
            fwrite(&pad, 1, 1, output);
        }
    }

    // Done reading/writing, close both files
    fclose(input);
    fclose(output);

    printf("Message encoded successfully!\n");
    printf("Message: %s\n", message);
    printf("Length : %u bytes\n", length);
    printf("Padding handled correctly.\n");
    printf("Output : images/stego.bmp\n");

    return 0;
}