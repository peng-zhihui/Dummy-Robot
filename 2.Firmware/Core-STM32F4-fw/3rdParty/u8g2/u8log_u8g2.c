/*

  u8log_u8g2.c
  

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

#include "u8g2.h"
/*
  Draw the u8log text at the specified x/y position.
  x/y position is the reference position of the first char of the first line.
  the line height is 
    u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2) + line_height_offset;
  line_height_offset can be set with u8log_SetLineHeightOffset()
  Use
    u8g2_SetFontRefHeightText(u8g2_t *u8g2);
    u8g2_SetFontRefHeightExtendedText(u8g2_t *u8g2);
    u8g2_SetFontRefHeightAll(u8g2_t *u8g2);
  to change the return values for u8g2_GetAscent and u8g2_GetDescent

*/
void u8g2_DrawLog(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8log_t *u8log)
{
  u8g2_uint_t disp_x, disp_y;
  uint8_t buf_x, buf_y;
  uint8_t c;
  
  disp_y = y;  
  u8g2_SetFontDirection(u8g2, 0);
  for( buf_y = 0; buf_y < u8log->height; buf_y++ )
  {
    disp_x = x;
    for( buf_x = 0; buf_x < u8log->width; buf_x++ )
    {
      c = u8log->screen_buffer[buf_y * u8log->width + buf_x];
      disp_x += u8g2_DrawGlyph(u8g2, disp_x, disp_y, c);
    }
    disp_y += u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2);
    disp_y += u8log->line_height_offset;
  }
}

/*
  u8lib callback for u8g2
  
  Only font direction 0 is supported: u8g2_SetFontDirection(u8g2, 0)
  Use
    u8g2_SetFontRefHeightText(u8g2_t *u8g2);
    u8g2_SetFontRefHeightExtendedText(u8g2_t *u8g2);
    u8g2_SetFontRefHeightAll(u8g2_t *u8g2);
  to change the top offset and the line height and
    u8log_SetLineHeightOffset(u8log_t *u8log, int8_t line_height_offset)
  to change the line height.
  
*/
void u8log_u8g2_cb(u8log_t * u8log)
{
  u8g2_t *u8g2 = (u8g2_t *)(u8log->aux_data);
  if ( u8log->is_redraw_line || u8log->is_redraw_all )
  {
    u8g2_FirstPage(u8g2);
    do
    {
      u8g2_DrawLog( u8g2, 0, u8g2_GetAscent(u8g2), u8log);
    }
    while( u8g2_NextPage(u8g2) );
  }
}

