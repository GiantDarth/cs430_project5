#ifndef CS430_PNM_READ_H
#define CS430_PNM_READ_H

#include <stdio.h>

#include "pnm.h"

int readHeader(pnmHeader* header, FILE* inputFd);
int readBody(pnmHeader header, pixel* pixels, FILE* inputFd);

#endif // CS430_PNM_READ_H
