/*

  u8x8_d_stdio.c

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/

#include "u8x8.h"

#include <stdio.h>

#define W 8
#define H 2

uint8_t bitmap[W * H * 8];

void bitmap_place_tile(uint8_t x, uint8_t y, uint8_t *tile)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
        bitmap[x * 8 + y * W * 8 + i] = tile[i];
}

void bitmap_show(void)
{
    int x, y;
    for (y = 0; y < H * 8; y++)
    {
        for (x = 0; x < W * 8; x++)
        {
            if ((bitmap[x + (y / 8) * W * 8] & (1 << ((y & 7)))) != 0)
            {
                printf("*");
            } else
            {
                printf(".");
            }
        }
        printf("\n");
    }
}


uint8_t u8x8_d_stdio(U8X8_UNUSED u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
        case U8X8_MSG_DISPLAY_INIT:
            break;
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
            if (arg_int == 0)
                bitmap_show();
            break;
        case U8X8_MSG_DISPLAY_SET_CONTRAST:
            break;
        case U8X8_MSG_DISPLAY_DRAW_TILE:
            bitmap_place_tile(((u8x8_tile_t *) arg_ptr)->x_pos, ((u8x8_tile_t *) arg_ptr)->y_pos,
                              ((u8x8_tile_t *) arg_ptr)->tile_ptr);
            break;
        default:
            break;
    }
    return 1;
}


void u8x8_SetupStdio(u8x8_t *u8x8)
{
    u8x8_SetupDefaults(u8x8);
    u8x8->display_cb = u8x8_d_stdio;
}

