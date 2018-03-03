#pragma once

struct ErrTex {
    unsigned int   width;
    unsigned int   height;
    unsigned int   bytes_per_pixel; /* 3:RGB, 4:RGBA */
    unsigned char  pixel_data[64 * 64 * 4 + 1];
} ;

extern ErrTex g_errTex;