/*

  u8log.c
  

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

#include <stdint.h>
#include <string.h>
#include "u8x8.h"


/*
static uint8_t u8log_is_on_screen(u8log_t *u8log, uint8_t x, uint8_t y)
{
  if ( x >= u8log->width )
    return 0;
  if ( y >= u8log->height )
    return 0;
  return 1;
}
*/

static void u8log_clear_screen(u8log_t *u8log)
{
  uint8_t *dest = u8log->screen_buffer;
  uint16_t cnt = u8log->height;
  cnt *= u8log->width;
  do
  {
    *dest++ = ' ';
    cnt--;
  } while( cnt > 0 );
  
}


/* scroll the content of the complete buffer, set redraw_line to 255 */
static void u8log_scroll_up(u8log_t *u8log)
{
  uint8_t *dest = u8log->screen_buffer;
  uint8_t *src = dest+u8log->width;
  uint16_t cnt = u8log->height;
  cnt--;
  cnt *= u8log->width;
  do
  {
    *dest++ = *src++;
    cnt--;
  } while( cnt > 0 );
  cnt = u8log->width;
  do
  {
    *dest++ = ' ';
    cnt--;
  } while(cnt > 0);
  
  if ( u8log->is_redraw_line_for_each_char )
    u8log->is_redraw_all = 1;
  else
    u8log->is_redraw_all_required_for_next_nl = 1;
}

/*
  Place the cursor on the screen. This will also scroll, if required 
*/
static void u8log_cursor_on_screen(u8log_t *u8log)
{
  //printf("u8log_cursor_on_screen, cursor_y=%d\n", u8log->cursor_y);
  if ( u8log->cursor_x >= u8log->width )
  {
    u8log->cursor_x = 0;
    u8log->cursor_y++;
  }
  while ( u8log->cursor_y >= u8log->height )
  {
    u8log_scroll_up(u8log);
    u8log->cursor_y--;
  }
}

/*
  Write a printable, single char on the screen, do any kind of scrolling
*/
static void u8log_write_to_screen(u8log_t *u8log, uint8_t c)
{
  u8log_cursor_on_screen(u8log);
  u8log->screen_buffer[u8log->cursor_y * u8log->width + u8log->cursor_x] = c;
  u8log->cursor_x++;
  
  if ( u8log->is_redraw_line_for_each_char )
  {
    u8log->is_redraw_line = 1;
    u8log->redraw_line = u8log->cursor_y;
  }
}

/*
  Handle control codes or write the char to the screen.
  Supported control codes are:
  
    \n		10		Goto first position of the next line. Line is marked for redraw.
    \r		13		Goto first position in the same line. Line is marked for redraw.
    \t		9		Jump to the next tab position
    \f		12		Clear the screen and mark redraw for whole screen
    any other char	Write char to screen. Line redraw mark depends on 
				is_redraw_line_for_each_char flag.
*/
void u8log_write_char(u8log_t *u8log, uint8_t c)
{
  switch(c)
  {
    case '\n':	// 10
      u8log->is_redraw_line = 1;
      u8log->redraw_line = u8log->cursor_y;
      if ( u8log->is_redraw_all_required_for_next_nl )
	u8log->is_redraw_all = 1;
      u8log->is_redraw_all_required_for_next_nl = 0;
      u8log->cursor_y++;
      u8log->cursor_x = 0;
      break;	
    case '\r':	// 13
      u8log->is_redraw_line = 1;
      u8log->redraw_line = u8log->cursor_y;
      u8log->cursor_x = 0;
      break;
    case '\t':	// 9
      u8log->cursor_x = (u8log->cursor_x + 8) & 0xf8;
      break;
    case '\f':	// 12
      u8log_clear_screen(u8log);
      u8log->is_redraw_all = 1;
      u8log->cursor_x = 0;
      u8log->cursor_y = 0;
      break;
    default:
      u8log_write_to_screen(u8log, c);
      break;
  }
}

void u8log_Init(u8log_t *u8log, uint8_t width, uint8_t height, uint8_t *buf)
{
  memset(u8log, 0, sizeof(u8log_t));
  u8log->width = width;
  u8log->height = height;
  u8log->screen_buffer = buf;
  u8log_clear_screen(u8log);
}

void u8log_SetCallback(u8log_t *u8log, u8log_cb cb, void *aux_data)
{
  u8log->cb = cb;
  u8log->aux_data = aux_data;
}

void u8log_SetRedrawMode(u8log_t *u8log, uint8_t is_redraw_line_for_each_char)
{
  u8log->is_redraw_line_for_each_char = is_redraw_line_for_each_char;
}

/* offset can be negative or positive, it is 0 by default */
void u8log_SetLineHeightOffset(u8log_t *u8log, int8_t line_height_offset)
{
  u8log->line_height_offset = line_height_offset;
}



void u8log_WriteChar(u8log_t *u8log, uint8_t c)
{
  u8log_write_char(u8log, c);
  if ( u8log->is_redraw_line || u8log->is_redraw_all )
  {
    if ( u8log->cb != 0 )
    {
      u8log->cb(u8log);
    }
    u8log->is_redraw_line = 0;
    u8log->is_redraw_all = 0;
  }
}

void u8log_WriteString(u8log_t *u8log, const char *s)
{
  while( *s != '\0' )
  {
    u8log_WriteChar(u8log, *s);
    s++;
  }
}

static void u8log_WriteHexHalfByte(u8log_t *u8log, uint8_t b) U8X8_NOINLINE;
static void u8log_WriteHexHalfByte(u8log_t *u8log, uint8_t b)
{
  b &= 0x0f;
  if ( b < 10 )
    u8log_WriteChar(u8log, b+'0');
  else
    u8log_WriteChar(u8log, b+'a'-10);
}

void u8log_WriteHex8(u8log_t *u8log, uint8_t b)
{
  u8log_WriteHexHalfByte(u8log, b >> 4);
  u8log_WriteHexHalfByte(u8log, b);
}

void u8log_WriteHex16(u8log_t *u8log, uint16_t v)
{
  u8log_WriteHex8(u8log, v>>8);
  u8log_WriteHex8(u8log, v);
}

void u8log_WriteHex32(u8log_t *u8log, uint32_t v)
{
  u8log_WriteHex16(u8log, v>>16);
  u8log_WriteHex16(u8log, v);
}

/* v = value, d = number of digits (1..3) */
void u8log_WriteDec8(u8log_t *u8log, uint8_t v, uint8_t d)
{
  u8log_WriteString(u8log, u8x8_u8toa(v, d));
}

/* v = value, d = number of digits (1..5) */
void u8log_WriteDec16(u8log_t *u8log, uint16_t v, uint8_t d)
{
  u8log_WriteString(u8log, u8x8_u16toa(v, d));
}
