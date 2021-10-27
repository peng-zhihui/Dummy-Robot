/*

  u8log_u8x8.c
  

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2018, olikraus@gmail.com
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

static void u8x8_DrawLogLine(u8x8_t *u8x8, uint8_t disp_x, uint8_t disp_y, uint8_t buf_y, u8log_t *u8log) U8X8_NOINLINE;
static void u8x8_DrawLogLine(u8x8_t *u8x8, uint8_t disp_x, uint8_t disp_y, uint8_t buf_y, u8log_t *u8log)
{
  uint8_t buf_x;
  uint8_t c;
  for( buf_x = 0; buf_x < u8log->width; buf_x++ )
  {
    c = u8log->screen_buffer[buf_y * u8log->width + buf_x];
    u8x8_DrawGlyph(u8x8, disp_x, disp_y, c);
    disp_x++;
  }
}

void u8x8_DrawLog(u8x8_t *u8x8, uint8_t x, uint8_t y, u8log_t *u8log)
{
  uint8_t buf_y;
  for( buf_y = 0; buf_y < u8log->height; buf_y++ )
  {
    u8x8_DrawLogLine(u8x8, x, y, buf_y, u8log);
    y++;
  }
}


void u8log_u8x8_cb(u8log_t * u8log)
{
  u8x8_t *u8x8 = (u8x8_t *)(u8log->aux_data);
  if ( u8log->is_redraw_all )
  {
    u8x8_DrawLog(u8x8, 0, 0, u8log);
  }
  else if ( u8log->is_redraw_line )
  {
    u8x8_DrawLogLine(u8x8, 0, u8log->redraw_line, u8log->redraw_line, u8log);
  }
}

