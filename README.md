# Image Steganography in C

A command-line tool that hides (encodes) and extracts (decodes) a secret text
message inside a 24-bit BMP image using **Least Significant Bit (LSB)
steganography**. The message is embedded in the blue channel of each pixel,
making the visual change imperceptible to the human eye while keeping the
image's dimensions, format, and file size effectively unchanged.

---

## Table of Contents

- [How It Works](#how-it-works)
- [Features](#features)
- [Requirements](#requirements)
- [Project Structure](#project-structure)
- [How to Compile](#how-to-compile)
- [How to Use](#how-to-use)
  - [Encoding a Message](#encoding-a-message)
  - [Decoding a Message](#decoding-a-message)
- [Example](#example)
- [Technical Details](#technical-details)
- [Limitations](#limitations)
- [Future Improvements](#future-improvements)
- [License](#license)
- [Author](#author)

---

## How It Works

LSB steganography works by replacing the least significant bit of each byte
in the image's pixel data with one bit of the secret message. Since the LSB
contributes the smallest possible value to a color channel (±1 out of 255),
the change is invisible to the naked eye.

This project specifically:

1. Reads the BMP file's header to locate the pixel data and validate the
   image is 24-bit.
2. Writes a **signature** (e.g. `"STEG"`) into the LSBs of the first bytes of
   pixel data, so the decoder can verify the image actually contains hidden
   data before attempting to extract it.
3. Writes the **length of the secret message** into the LSBs of the
   following bytes, so the decoder knows exactly how many characters to
   read back out.
4. Writes the **message itself**, bit by bit, into the LSB of the blue
   channel of each subsequent pixel.
5. Accounts for **BMP row padding** (each row in a BMP file is padded to a
   multiple of 4 bytes), so the encoder/decoder never accidentally treats
   padding bytes as pixel data.

## Features

- Reads and parses BMP image headers and pixel data
- Uses the LSB of the blue channel to store hidden data
- Embeds a `STEG` signature to verify encoded images before decoding
- Stores the message length so decoding stops at the correct point
- Correctly handles BMP row padding (rows padded to multiples of 4 bytes)
- Encodes secret text messages into an image
- Decodes and extracts hidden messages from an image
- Leaves the image's visual appearance unchanged

## Requirements

- GCC (C compiler)
- Linux or WSL (Windows Subsystem for Linux)
- A 24-bit (uncompressed) BMP image to use as the carrier file

> **Note:** The image must be a true 24-bit BMP (no compression, no
> palette/indexed color). Images exported from tools like GIMP or Photoshop
> as "24-bit BMP" work well.

## Project Structure

```
.
├── encode_final.c   # Source code for encoding a message into a BMP image
├── decode_final.c   # Source code for decoding a message from a BMP image
├── input.bmp         # Example carrier image (not included, supply your own)
├── output.bmp        # Resulting image after encoding (generated)
└── README.md
```

## How to Compile

```bash
gcc -Wall -Wextra -std=c11 encode_final.c -o encode_final
gcc -Wall -Wextra -std=c11 decode_final.c -o decode_final
```

- `-Wall -Wextra` enables extra compiler warnings to catch potential bugs.
- `-std=c11` compiles against the C11 standard.

## How to Use

### Encoding a Message

```bash
./encode_final <input.bmp> <output.bmp> "Your secret message"
```

| Argument         | Description                                      |
|------------------|---------------------------------------------------|
| `input.bmp`      | Path to the original 24-bit BMP carrier image     |
| `output.bmp`     | Path where the new image with hidden data is saved|
| `"message"`      | The secret text message to hide                  |

### Decoding a Message

```bash
./decode_final <output.bmp>
```

| Argument      | Description                                   |
|---------------|------------------------------------------------|
| `output.bmp`  | Path to the BMP image containing hidden data   |

The program verifies the `STEG` signature, reads the stored message length,
then extracts and prints the hidden message to the terminal.

## Example

```bash
$ gcc -Wall -Wextra -std=c11 encode_final.c -o encode_final
$ gcc -Wall -Wextra -std=c11 decode_final.c -o decode_final

$ ./encode_final input.bmp output.bmp "Meet me at midnight"
[+] Message encoded successfully into output.bmp

$ ./decode_final output.bmp
[+] Signature verified.
[+] Decoded message: Meet me at midnight
```

## Technical Details

- **Capacity:** Each pixel can hide 1 bit (in the blue channel's LSB), so a
  message of *N* characters requires roughly `N * 8` usable pixels, plus a
  few extra bytes for the signature and length header. Larger carrier
  images can hide longer messages.
- **BMP structure:** A BMP file consists of a file header, a DIB (info)
  header, an optional color table, and the pixel data array. Pixel rows are
  stored bottom-to-top and padded to a multiple of 4 bytes — this program
  reads the header fields (e.g. width, height, bits-per-pixel) to correctly
  navigate the pixel array and skip padding bytes.
- **Why the blue channel:** Human eyes are least sensitive to small
  variations in blue, making it a common choice for LSB steganography to
  minimize any perceptible change.

## Limitations

- Only supports uncompressed 24-bit BMP images (no PNG, JPEG, or compressed
  BMP support).
- Message length is limited by the number of pixels available in the
  carrier image.
- No encryption — the hidden message is only concealed, not encrypted. Combine
  with a cipher beforehand if confidentiality against determined attackers is
  required.
- No error-correction — heavy image editing or format conversion after
  encoding will corrupt or destroy the hidden message.

## Future Improvements

- [ ] Add support for other image formats (PNG)
- [ ] Add optional AES/XOR encryption of the message before encoding
- [ ] Support hiding files (not just text) inside images
- [ ] Add a capacity check with a clear error message before encoding
- [ ] Build a simple GUI or web front-end

## License

This project is licensed under the MIT License — feel free to use, modify,
and distribute it.

## Author

Chandrashekhar GitHub profile - (https://github.com/Chandru2603)