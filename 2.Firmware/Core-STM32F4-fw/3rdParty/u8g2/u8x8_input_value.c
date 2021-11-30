/*

  u8x8_input_value.c
  
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

/*
  return:
    0: value is not changed (HOME/Break Button pressed)
    1: value has been updated
*/

uint8_t u8x8_UserInterfaceInputValue(u8x8_t *u8x8, const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post)
{
  uint8_t height;
  uint8_t y;
  uint8_t width;
  uint8_t x;
  uint8_t local_value = *value;
  uint8_t r;
  uint8_t event;

  /* calculate overall height of the input value box */
  height = 1;	/* button line */
  height += u8x8_GetStringLineCnt(title);
  
  /* calculate offset from top */
  y = 0;
  if ( height < u8x8_GetRows(u8x8)  )
  {
    y = u8x8_GetRows(u8x8);
    y -= height;
    y /= 2;
  }
  
  /* calculate offset from left for the label */
  x = 0;
  width = u8x8_GetUTF8Len(u8x8, pre);
  width += digits;
  width += u8x8_GetUTF8Len(u8x8, post);
  if ( width < u8x8_GetCols(u8x8) )
  {
    x = u8x8_GetCols(u8x8);
    x -= width;
    x /= 2;
  }
  
  /* render */
  u8x8_ClearDisplay(u8x8);   /* required, because not everything is filled */
  u8x8_SetInverseFont(u8x8, 0);  
  y += u8x8_DrawUTF8Lines(u8x8, 0, y, u8x8_GetCols(u8x8), title);
  x += u8x8_DrawUTF8(u8x8, x, y, pre);
  u8x8_DrawUTF8(u8x8, x+digits, y, post);
  u8x8_SetInverseFont(u8x8, 1);
  
  /* event loop */
  u8x8_DrawUTF8(u8x8, x, y, u8x8_u8toa(local_value, digits));
  for(;;)
  {
    event = u8x8_GetMenuEvent(u8x8);
    if ( event == U8X8_MSG_GPIO_MENU_SELECT )
    {
      *value = local_value;
      r = 1;
      break;
    }
    else if ( event == U8X8_MSG_GPIO_MENU_HOME )
    {
      r = 0;
      break;
    }
    else if ( event == U8X8_MSG_GPIO_MENU_NEXT || event == U8X8_MSG_GPIO_MENU_UP )
    {
      if ( local_value >= hi )
	local_value = lo;
      else
	local_value++;
      u8x8_DrawUTF8(u8x8, x, y, u8x8_u8toa(local_value, digits));
    }
    else if ( event == U8X8_MSG_GPIO_MENU_PREV || event == U8X8_MSG_GPIO_MENU_DOWN )
    {
      if ( local_value <= lo )
	local_value = hi;
      else
	local_value--;
      u8x8_DrawUTF8(u8x8, x, y, u8x8_u8toa(local_value, digits));
    }        
  }
  
  u8x8_SetInverseFont(u8x8, 0);
  return r;  
}
