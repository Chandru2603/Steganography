#include <stdio.h>
#include <stdint.h>
#include <string.h>

unsigned char get_lsb(unsigned char byte)
{
    return byte & 1;
}

int main()
{
    FILE *file;

    unsigned char pixel[3];

    file = fopen("images/stego.bmp", "rb");

    if (file == NULL)
    {
        printf("Error opening stego.bmp\n");
        return 1;
    }

    /* BMP information */
    int32_t width;
    int32_t height;
    
    fseek(file, 18, SEEK_SET);
    fread(&width, sizeof(width), 1, file);
    
    fseek(file, 22, SEEK_SET);
    fread(&height, sizeof(height), 1, file);
    
    fseek(file, 54, SEEK_SET);

    int row_bytes = width * 3;
    int padding = (4 - (row_bytes % 4)) % 4;

    /*
     * We need to read:
     *
     * STEG       = 4 bytes
     * Length     = 4 bytes
     * Message    = 5 bytes
     *
     * Total = 13 bytes
     */
    unsigned char data[108] = {0};

    int bit_index = 0;
    int total_bits = 108 * 8;

    /* Move past BMP header */
    fseek(file, 54, SEEK_SET);

    /* Read pixels row by row */
    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            fread(pixel, 1, 3, file);

            if (bit_index < total_bits)
            {
                int byte_index = bit_index / 8;
                int bit_position = 7 - (bit_index % 8);

                unsigned char secret_bit =
                    get_lsb(pixel[0]);

                data[byte_index] |=
                    secret_bit << bit_position;

                bit_index++;
            }
        }

        /* Skip padding */
        fseek(file, padding, SEEK_CUR);
    }

    fclose(file);

    /* Check magic */
    if (data[0] != 'S' ||
        data[1] != 'T' ||
        data[2] != 'E' ||
        data[3] != 'G')
    {
        printf("Error: Invalid steganography image.\n");
        return 1;
    }

    printf("Magic: STEG\n");

    /* Decode length */
    uint32_t length =
        ((uint32_t)data[4] << 24) |
        ((uint32_t)data[5] << 16) |
        ((uint32_t)data[6] << 8) |
        data[7];

    printf("Message length: %u bytes\n", length);

    /* Display message */
    printf("Decoded message: ");

    for (uint32_t i = 0; i < length; i++)
    {
        printf("%c", data[8 + i]);
    }

    printf("\n");

    return 0;
}