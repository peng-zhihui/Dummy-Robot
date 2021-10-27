/*

  u8x8_string.c
  
  string line procedures
  
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

uint8_t u8x8_GetStringLineCnt(const char *str)
{
  char e;
  uint8_t line_cnt = 1;
  if ( str == NULL )
    return 0;
  for(;;)
  {
    e = *str;
    if ( e == '\0' )
      break;
    str++;
    if ( e == '\n' )
      line_cnt++;
  }
  return line_cnt;
}


/*
    Assumes strings, separated by '\n' in "str".
    Returns the string at index "line_idx". First strng has line_idx = 0
    Example:
      Returns "xyz" for line_idx = 1 with str = "abc\nxyz"
    Support both UTF8 and normal strings.
*/
const char *u8x8_GetStringLineStart(uint8_t line_idx, const char *str )
{
  char e;
  uint8_t line_cnt = 1;
  
  if ( line_idx == 0 )
    return str;

  for(;;)
  {
    e = *str;
    if ( e == '\0' )
      break;
    str++;
    if ( e == '\n' )
    {
      if ( line_cnt == line_idx )
	return str;
      line_cnt++;
    }
  }
  return NULL;	/* line not found */
}

/* copy until first '\n' or '\0' in str */
/* Important: There is no string overflow check, ensure */
/* that the destination buffer is large enough */
void u8x8_CopyStringLine(char *dest, uint8_t line_idx, const char *str)
{
  if ( dest == NULL )
    return;
  str = u8x8_GetStringLineStart( line_idx, str );
  if ( str != NULL )
  {
    for(;;)
    {
      if ( *str == '\n' || *str == '\0' )
	break;
      *dest = *str;
      dest++;
      str++;
    }
  }
  *dest = '\0';
}

/*
  Draw a string
  Extend the string to size "w"
  Center the string within "w"
  return the size of the string

*/
uint8_t u8x8_DrawUTF8Line(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t w, const char *s)
{
  uint8_t d, lw;
  uint8_t cx, dx;
    
  d = 0;
  
  lw = u8x8_GetUTF8Len(u8x8, s);
  if ( lw < w )
  {
    d = w;
    d -=lw;
    d /= 2;
  }
    
  cx = x;
  dx = cx + d;
  while( cx < dx )
  {
    u8x8_DrawUTF8(u8x8, cx, y, " ");
    cx++;
  }
  cx += u8x8_DrawUTF8(u8x8, cx, y, s);
  dx = x + w;
  while( cx < dx )
  {
    u8x8_DrawUTF8(u8x8, cx, y, " ");
    cx++;
  }
  cx -= x;
  return cx;
}

/*
  draw several lines at position x,y.
  lines are stored in s and must be separated with '\n'.
  lines can be centered with respect to "w" 
  if s == NULL nothing is drawn and 0 is returned
  returns the number of lines in s
*/
uint8_t u8x8_DrawUTF8Lines(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t w, const char *s)
{
  uint8_t i;
  uint8_t cnt;
  cnt = u8x8_GetStringLineCnt(s);
  for( i = 0; i < cnt; i++ )
  {
    u8x8_DrawUTF8Line(u8x8, x, y, w, u8x8_GetStringLineStart(i, s));
    y++;
  }
  return cnt;
}
