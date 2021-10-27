/*

  u8g2_input_value.c
  
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

#include "u8g2.h"

/*
  return:
    0: value is not changed (HOME/Break Button pressed)
    1: value has been updated
*/

uint8_t u8g2_UserInterfaceInputValue(u8g2_t *u8g2, const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post)
{
  uint8_t line_height;
  uint8_t height;
  u8g2_uint_t pixel_height;
  u8g2_uint_t  y, yy;
  u8g2_uint_t  pixel_width;
  u8g2_uint_t  x, xx;
  
  uint8_t local_value = *value;
  //uint8_t r; /* not used ??? */
  uint8_t event;

  /* only horizontal strings are supported, so force this here */
  u8g2_SetFontDirection(u8g2, 0);

  /* force baseline position */
  u8g2_SetFontPosBaseline(u8g2);
  
  /* calculate line height */
  line_height = u8g2_GetAscent(u8g2);
  line_height -= u8g2_GetDescent(u8g2);
  
  
  /* calculate overall height of the input value box */
  height = 1;	/* value input line */
  height += u8x8_GetStringLineCnt(title);

  /* calculate the height in pixel */
  pixel_height = height;
  pixel_height *= line_height;


  /* calculate offset from top */
  y = 0;
  if ( pixel_height < u8g2_GetDisplayHeight(u8g2)  )
  {
    y = u8g2_GetDisplayHeight(u8g2);
    y -= pixel_height;
    y /= 2;
  }
  
  /* calculate offset from left for the label */
  x = 0;
  pixel_width = u8g2_GetUTF8Width(u8g2, pre);
  pixel_width += u8g2_GetUTF8Width(u8g2, "0") * digits;
  pixel_width += u8g2_GetUTF8Width(u8g2, post);
  if ( pixel_width < u8g2_GetDisplayWidth(u8g2) )
  {
    x = u8g2_GetDisplayWidth(u8g2);
    x -= pixel_width;
    x /= 2;
  }
  
  /* event loop */
  for(;;)
  {
    u8g2_FirstPage(u8g2);
    do
    {
      /* render */
      yy = y;
      yy += u8g2_DrawUTF8Lines(u8g2, 0, yy, u8g2_GetDisplayWidth(u8g2), line_height, title);
      xx = x;
      xx += u8g2_DrawUTF8(u8g2, xx, yy, pre);
      xx += u8g2_DrawUTF8(u8g2, xx, yy, u8x8_u8toa(local_value, digits));
      u8g2_DrawUTF8(u8g2, xx, yy, post);
    } while( u8g2_NextPage(u8g2) );
    
#ifdef U8G2_REF_MAN_PIC
      return 0;
#endif
    
    for(;;)
    {
      event = u8x8_GetMenuEvent(u8g2_GetU8x8(u8g2));
      if ( event == U8X8_MSG_GPIO_MENU_SELECT )
      {
	*value = local_value;
	return 1;
      }
      else if ( event == U8X8_MSG_GPIO_MENU_HOME )
      {
	return 0;
      }
      else if ( event == U8X8_MSG_GPIO_MENU_NEXT || event == U8X8_MSG_GPIO_MENU_UP )
      {
	if ( local_value >= hi )
	  local_value = lo;
	else
	  local_value++;
	break;
      }
      else if ( event == U8X8_MSG_GPIO_MENU_PREV || event == U8X8_MSG_GPIO_MENU_DOWN )
      {
	if ( local_value <= lo )
	  local_value = hi;
	else
	  local_value--;
	break;
      }        
    }
  }
  
  /* never reached */
  //return r;  
}
