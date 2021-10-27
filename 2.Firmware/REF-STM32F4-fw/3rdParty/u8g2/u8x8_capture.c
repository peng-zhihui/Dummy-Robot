/*

  u8x8_capture.c
  
  Screen capture funcion
  
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

/*========================================================*/


/* vertical top lsb memory architecture */
uint8_t u8x8_capture_get_pixel_1(uint16_t x, uint16_t y, uint8_t *dest_ptr, uint8_t tile_width)
{
  //uint8_t *dest_ptr = capture->buffer;
  //if ( dest_ptr == NULL )
    //return 0;
  //dest_ptr += (y/8)*capture->tile_width*8;
  dest_ptr += (y/8)*tile_width*8;
  y &= 7;
  dest_ptr += x;
  if ( (*dest_ptr & (1<<y)) == 0 )
    return 0;
  return 1;
}

/* horizontal right lsb memory architecture */
/* SH1122, LD7032, ST7920, ST7986, LC7981, T6963, SED1330, RA8835, MAX7219, LS0 */ 
uint8_t u8x8_capture_get_pixel_2(uint16_t x, uint16_t y, uint8_t *dest_ptr, uint8_t tile_width)
{
  //uint8_t *dest_ptr = capture->buffer;
  //if ( dest_ptr == NULL )
  //  return 0;
  //dest_ptr += y*capture->tile_width;
  y *= tile_width;
  dest_ptr += y;
  dest_ptr += x>>3;
  if ( (*dest_ptr & (128>>(x&7))) == 0 )
    return 0;
  return 1;
}

void u8x8_capture_write_pbm_pre(uint8_t tile_width, uint8_t tile_height, void (*out)(const char *s))
{
  out("P1\n");
  out(u8x8_utoa((uint16_t)tile_width*8));
  out("\n");
  out(u8x8_utoa((uint16_t)tile_height*8));
  out("\n");
}


void u8x8_capture_write_pbm_buffer(uint8_t *buffer, uint8_t tile_width, uint8_t tile_height, uint8_t (*get_pixel)(uint16_t x, uint16_t y, uint8_t *dest_ptr, uint8_t tile_width), void (*out)(const char *s))
{
  uint16_t x, y;
  uint16_t w, h;

  w = tile_width;
  w *= 8;
  h = tile_height;
  h *= 8;
    
  for( y = 0; y < h; y++)
  {
    for( x = 0; x < w; x++)
    {
      if ( get_pixel(x, y, buffer, tile_width) )
	out("1");
      else
	out("0"); 	  
    }
    out("\n");
  }
}




void u8x8_capture_write_xbm_pre(uint8_t tile_width, uint8_t tile_height, void (*out)(const char *s))
{
  out("#define xbm_width ");
  out(u8x8_utoa((uint16_t)tile_width*8));
  out("\n");
  out("#define xbm_height ");
  out(u8x8_utoa((uint16_t)tile_height*8));
  out("\n");  
  out("static unsigned char xbm_bits[] = {\n");  
}

void u8x8_capture_write_xbm_buffer(uint8_t *buffer, uint8_t tile_width, uint8_t tile_height, uint8_t (*get_pixel)(uint16_t x, uint16_t y, uint8_t *dest_ptr, uint8_t tile_width), void (*out)(const char *s))
{
  uint16_t x, y;
  uint16_t w, h;
  uint8_t v, b;
  char s[2];
  s[1] = '\0';

  w = tile_width;
  w *= 8;
  h = tile_height;
  h *= 8;

  y = 0;
  for(;;)
  {
    x = 0;
    for(;;)
    {
      v = 0;
      for( b = 0; b < 8; b++ )
      {
	v <<= 1;
	if ( get_pixel(x+7-b, y, buffer, tile_width) )
	  v |= 1;
      }
      out("0x");
      s[0] = (v>>4);
      if ( s[0] <= 9 )
	s[0] += '0';
      else
	s[0] += 'a'-10;
      out(s);
      s[0] = (v&15);
      if ( s[0] <= 9 )
	s[0] += '0';
      else
	s[0] += 'a'-10;
      out(s);
      x += 8;
      if ( x >= w )
	break;
      out(",");
    }
    y++;
    if ( y >= h )
      break;
    out(",");
    out("\n");
  }
  out("};\n");
  
}



/*========================================================*/

#ifdef NOT_YET_IMPLEMENTED_U8X8_SCREEN_CAPTURE

struct _u8x8_capture_struct
{
  u8x8_msg_cb old_cb;
  uint8_t *buffer;	/* tile_width*tile_height*8 bytes */
  uint8_t tile_width;
  uint8_t tile_height;
};
typedef struct _u8x8_capture_struct u8x8_capture_t;


u8x8_capture_t u8x8_capture;


static void u8x8_capture_memory_copy(uint8_t *dest, uint8_t *src, uint16_t cnt)
{
  while( cnt > 0 )
  {
    *dest++ = *src++;
    cnt--;
  }
}

static void u8x8_capture_DrawTiles(u8x8_capture_t *capture, uint8_t tx, uint8_t ty, uint8_t tile_cnt, uint8_t *tile_ptr)
{
  uint8_t *dest_ptr = capture->buffer;
  //printf("tile pos: %d %d, cnt=%d\n", tx, ty, tile_cnt);
  if ( dest_ptr == NULL )
    return;
  dest_ptr += (uint16_t)ty*capture->tile_width*8;
  dest_ptr += (uint16_t)tx*8;
  u8x8_capture_memory_copy(dest_ptr, tile_ptr, tile_cnt*8);
}

uint8_t u8x8_d_capture(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if (  msg ==  U8X8_MSG_DISPLAY_DRAW_TILE )
  {
    uint8_t x, y, c;
    uint8_t *ptr;
    x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
    y = ((u8x8_tile_t *)arg_ptr)->y_pos;
    c = ((u8x8_tile_t *)arg_ptr)->cnt;
    ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
    do
    {
      u8x8_capture_DrawTiles(&u8x8_capture, x, y, c, ptr);
      x += c;
      arg_int--;
    } while( arg_int > 0 );
  }
  return u8x8_capture.old_cb(u8x8, msg, arg_int, arg_ptr);
}

uint8_t u8x8_GetCaptureMemoryPixel(u8x8_t *u8x8, uint16_t x, uint16_t y)
{
  return u8x8_capture_GetPixel(&u8x8_capture, x, y);
}

/* memory: tile_width*tile_height*8 bytes */
void u8x8_ConnectCapture(u8x8_t *u8x8, uint8_t tile_width, uint8_t tile_height, uint8_t *memory)
{
  if ( u8x8->display_cb == u8x8_d_capture )
    return;	/* do nothing, capture already installed */

  u8x8_capture.buffer = memory;	/* tile_width*tile_height*8 bytes */
  u8x8_capture.tile_width = tile_width;
  u8x8_capture.tile_height = tile_height;
  u8x8_capture.old_cb = u8x8->display_cb;
  u8x8->display_cb = u8x8_d_capture;
  return;
}

#endif