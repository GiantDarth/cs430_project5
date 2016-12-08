#ifndef CS430_PNM_H
#define CS430_PNM_H

#include <stddef.h>

#define CS430_PNM_MIN 1
#define CS430_PNM_BITMAP_MAX 255
#define CS430_PNM_GREY_MAX 65535
#define CS430_PNM_FULL_MAX 65535
#define CS430_PNM_MAX_SUPPORTED 255
#define CS430_WIDTH_MIN 1
#define CS430_HEIGHT_MIN 1
#define CS430_MAX_LINE 70

typedef struct pnmHeader {
    int mode;
    size_t width;
    size_t height;
    size_t maxColorSize;
} pnmHeader;

typedef struct pixel {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} pixel;

#endif // CS430_PNM_H
