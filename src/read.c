#include <inttypes.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

#include "read.h"

int readChannel(pnmHeader header, FILE* inputFd, int isLast);
int skipWhitespace(FILE* fd);
int skipLine(FILE* fd);
int skipUntilNext(FILE* fd);
int getMagicNumber(FILE* fd);
long long getNumber(size_t maxDigits, FILE* fd);

int readHeader(pnmHeader* header, FILE* inputFd) {
    long long value;

    // Read the magic number, if any
    if((value = getMagicNumber(inputFd)) < 0) {
        return -1;
    }
    else {
        header->mode = value;
    }

    if(skipUntilNext(inputFd) < 0) {
        return -1;
    }

    // Read the width, if there is one.
    if((value = getNumber(20, inputFd)) < 0) {
        return -1;
    }
    else if(value < CS430_WIDTH_MIN) {
        fprintf(stderr, "Error: Width cannot be less than %d\n", CS430_WIDTH_MIN);
        return -1;
    }
    else {
        header->width = value;
    }

    if(skipUntilNext(inputFd) < 0) {
        return -1;
    }

    // Read the height, if there is one.
    if((value = getNumber(20, inputFd)) < 0) {
        return -1;
    }
    else if(value < 1) {
        fprintf(stderr, "Error: Height cannot be less than %d\n", CS430_HEIGHT_MIN);
        return -1;
    }
    else {
        header->height = value;
    }

    if(skipUntilNext(inputFd) < 0) {
        return -1;
    }

    if((value = getNumber(5, inputFd)) < 0) {
        return -1;
    }
    else if(value < CS430_PNM_MIN) {
        fprintf(stderr, "Error: Max color value cannot be less than %d.\n",
            CS430_PNM_MIN);
        return -1;
    }
    // If the value exceeds 1 byte (8-bits)
    else if(value > CS430_PNM_MAX_SUPPORTED) {
        fprintf(stderr, "Error: Max color value cannot be greater than 1 byte (aka. %d)\n",
            CS430_PNM_MAX_SUPPORTED);
        return -1;
    }
    else {
        header->maxColorSize = value;
    }

    if((value = fgetc(inputFd)) == EOF) {
        // If end-of-file reached and not at the very last pixel
        if(feof(inputFd)) {
            fprintf(stderr, "Error: Premature EOF reading pixel data\n");
            return -1;
        }
        // If some read error has occurred
        else if(ferror(inputFd)) {
            perror("Error: Read error during pixel data\n");
            return -1;
        }
    }

    if(value == '#') {
        // If next character starts a comment, then skip the remaining
        while((value == '#' && (value = skipLine(inputFd)) >= CHAR_MIN));
        if(value < CHAR_MIN) {
            return -1;
        }
        else if((value = fgetc(inputFd)) == EOF) {
            // If end-of-file reached and not at the very last pixel
            if(feof(inputFd)) {
                fprintf(stderr, "Error: Premature EOF reading pixel data\n");
                return -1;
            }
            // If some read error has occurred
            else if(ferror(inputFd)) {
                perror("Error: Read error during pixel data\n");
                return -1;
            }
        }
    }

    // If character immediately following the max color value or comments is not
    // whitespace, then return on error.
    if(!isspace(value)) {
        fprintf(stderr, "Error: Pixel values must be preceeded by a single whitespace\n");
        return -1;
    }

    return 0;
}

int readBody(pnmHeader header, pixel* pixels, FILE* inputFd) {
    if(header.mode < 1 || header.mode > 7) {
        fprintf(stderr, "Error: Mode %d not valid\n", header.mode);
        return -1;
    }

    if(header.mode == 3) {
        int value;

        for(size_t i = 0; i < header.height; i++) {
            for(size_t j = 0; j < header.width; j++) {
                if((value = readChannel(header, inputFd, 0)) < 0) {
                    return -1;
                }
                pixels[i * header.width + j].red = value;

                if((value = readChannel(header, inputFd, 0)) < 0) {
                    return -1;
                }
                pixels[i * header.width + j].green = value;

                if((value = readChannel(header, inputFd, (i == header.height - 1) &&
                        (j == header.width - 1))) < 0) {
                    return -1;
                }
                pixels[i * header.width + j].blue = value;
            }
        }
    }
    else if(header.mode == 6) {
        // Loop through the following raw binary in the file with the image
        // represented from top-to-bottom going row-by-row
        for(size_t i = 0; i < header.height; i++) {
            for(size_t j = 0; j < header.width; j++) {
                fread(&(pixels[i * header.width + j].red), 1, 1, inputFd);
                fread(&(pixels[i * header.width + j].green), 1, 1, inputFd);
                fread(&(pixels[i * header.width + j].blue), 1, 1, inputFd);
            }
        }
    }
    else {
        fprintf(stderr, "Error: Mode %d not supported\n", header.mode);
        return -1;
    }

    return 0;
}

int skipWhitespace(FILE* fd) {
    char value;

    // Loop until either EOF or no whitespace remains.
    while((value = fgetc(fd)) != EOF && isspace(value));

    if(feof(fd)) {
        fprintf(stderr, "Error: Premature EOF during skip whitespace\n");
        return CHAR_MIN - 1;
    }

    if(ferror(fd)) {
        perror("Error: Read error during skip shitespace\n");
        return CHAR_MIN - 1;
    }

    return value;
}

int skipLine(FILE* fd) {
    char value;

    while((value = fgetc(fd)) != EOF && value != '\n' && value != '\r');

    if(feof(fd)) {
        fprintf(stderr, "Error: Premature EOF during skip line\n");
        return CHAR_MIN - 1;
    }

    if(ferror(fd)) {
        perror("Error: Read error during skip line\n");
        return CHAR_MIN - 1;
    }

    return value;
}

int skipUntilNext(FILE* fd) {
    int value;

    // Continue skipping whitespace and comments until an error occurs or no
    // more comments exist.
    while((value = skipWhitespace(fd)) < CHAR_MIN ||
        (value == '#' && (value = skipLine(fd)) < CHAR_MIN));
    if(value < CHAR_MIN) {
        return -1;
    }

    if((value != '\n' && value != '\r') && ungetc(value, fd) == EOF) {
        perror("Error: Read error during unget newline\n");
        return -1;
    }

    return 0;
}

int getMagicNumber(FILE* fd) {
    char buffer[3] = { '\0' };

    if(fgets(buffer, 3, fd) == NULL) {
        fprintf(stderr, "Error: Empty file\n");
        return -1;
    }

    if(strlen(buffer) < 2) {
        fprintf(stderr, "Error: Magic number less than two characters\n");
        return -1;
    }

    if(buffer[0] != 'P' || buffer[1] < '1' || buffer[1] > '7') {
        fprintf(stderr, "Error: File lacks one of the correct magic numbers P1-P7\n");
        return -1;
    }

    if(buffer[1] != '3' && buffer[1] != '6') {
        fprintf(stderr, "Error: P%c not supported\n", buffer[1]);
        return -1;
    }

    // Convert ASCII to single-digit number.
    return buffer[1] - '0';
}

long long getNumber(size_t maxDigits, FILE* fd) {
    char buffer[64] = { '\0' };
    char* endptr;
    size_t i = 0;

    long long value = 0;

    // Continue to read character-by-character until end-of-buffer reached,
    // or end-of-file / read error reached, or some non-decimal is reached.
    while(i < maxDigits && (value = fgetc(fd)) != EOF &&
            isdigit(value)) {
        buffer[i++] = value;
    }

    if(value == EOF) {
        // If end-of-file reached and not at the very last pixel
        if(feof(fd)) {
            fprintf(stderr, "Error: Premature EOF in header\n");
            return -1;
        }
        // If some read error has occurred
        else if(ferror(fd)) {
            perror("Error: Read error during header\n");
            return -1;
        }
    }

    if(ungetc(value, fd) == EOF) {
        perror("Error: Read error on ungetc\n");
        return -1;
    }

    value = strtoll(buffer, &endptr, 10);
    // If the first character is not empty and the set first invalid
    // character is empty, then the whole string is valid. (see 'man strtol')
    // Otherwise, part of the string is not a number.
    if(!(*buffer != '\0' && *endptr == '\0')) {
        fprintf(stderr, "Error: Invalid value (non-decimal)\n");
        return -1;
    }

    if(errno == ERANGE) {
        if(value == LLONG_MAX) {
            fprintf(stderr, "Error: Value larger than %lld\n", LLONG_MAX);
        }
        else {
            fprintf(stderr, "Error: Value smaller than %lld\n", LLONG_MIN);
        }

        return -1;
    }

    return value;
}

int readChannel(pnmHeader header, FILE* inputFd, int isLast) {
    char buffer[4] = { '\0' };
    char* endptr;
    int value, i = 0;
    // Continue to read character-by-character until end-of-buffer reached,
    // or end-of-file / read error reached, or some non-decimal is reached.
    while((size_t)i < sizeof(buffer) - 1 && (value = fgetc(inputFd)) != EOF &&
            isdigit(value)) {
        buffer[i++] = value;
    }

    if(value == EOF) {
        // If end-of-file reached and not at the very last pixel
        if(!isLast && feof(inputFd)) {
            fprintf(stderr, "Error: Premature EOF reading pixel data\n");
            return -1;
        }
        // If some read error has occurred
        else if(ferror(inputFd)) {
            perror("Error: Read error during pixel data\n");
            return -1;
        }
    }
    // If there isn't at least some whitespace inbetween each pixel
    // and not at the very last pixel
    else if(!isLast) {
        // If the last read character is a digit still (if the number read is 3
        // digits long out of the 4 possible bytes), then get the next
        // character to use for whitespace check instead
        if(isdigit(value)) {
            value = fgetc(inputFd);

            if(value == EOF) {
                // If end-of-file reached
                if(feof(inputFd)) {
                    fprintf(stderr, "Error: Premature EOF reading pixel data\n");
                    return -1;
                }
                // If some read error has occurred
                else if(ferror(inputFd)) {
                    perror("Error: Read error during pixel data\n");
                    return -1;
                }
            }
        }

        // Check if at least one whitespace splits channel data
        if(!isspace(value)) {
            fprintf(stderr, "Error: There must be at least one whitespace "
                "character inbetween pixel data\n");
            return -1;
        }

        // Skip remaining potential whitespace inbetween channels
        if((value = skipWhitespace(inputFd)) < 0) {
            return -1;
        }

        // Undo the first digit of the next channel value
        if(ungetc(value, inputFd) == EOF) {
            fprintf(stderr, "Error: Read error ungetting digit in pixel data\n");
            return -1;
        }
    }

    value = strtol(buffer, &endptr, 10);
    // If the first character is not empty and the set first invalid
    // character is empty, then the whole string is valid. (see 'man strtol')
    // Otherwise, part of the string is not a number.
    if(!(*buffer != '\0' && *endptr == '\0')) {
        fprintf(stderr, "Error: Invalid decimal value on channel\n");
        return -1;
    }

    if(value < 0) {
        fprintf(stderr, "Error: Pixel value cannot be less than 0\n");
        return -1;
    }
    else if((size_t)value > header.maxColorSize) {
        fprintf(stderr, "Error: Pixel value cannot exceed supplied "
            "max color value\n");
        return -1;
    }

    return value;
}
