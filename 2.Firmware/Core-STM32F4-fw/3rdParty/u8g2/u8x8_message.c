/*

  u8x8_message.c
  
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

uint8_t u8x8_draw_button_line(u8x8_t *u8x8, uint8_t y, uint8_t w, uint8_t cursor, const char *s)
{
  uint8_t i;
  uint8_t cnt;
  uint8_t total;
  uint8_t d;
  uint8_t x;
  cnt = u8x8_GetStringLineCnt(s);
  
  /* calculate the width of the button */
  total = 0;
  for( i = 0; i < cnt; i++ )
  {
    total += u8x8_GetUTF8Len(u8x8, u8x8_GetStringLineStart(i, s));
  }
  total += (cnt-1);	/* had one space between the buttons */
  
  /* calculate the left offset */
  d = 0;
  if ( total < w )
  {
    d = w;
    d -= total;
    d /= 2;
  }
  
  /* draw the buttons */
  x = d;
  u8x8_SetInverseFont(u8x8, 0);
  for( i = 0; i < cnt; i++ )
  {
    if ( i == cursor )
      u8x8_SetInverseFont(u8x8, 1);
      
    x+=u8x8_DrawUTF8(u8x8, x, y, u8x8_GetStringLineStart(i, s));
    u8x8_SetInverseFont(u8x8, 0);
    x+=u8x8_DrawUTF8(u8x8, x, y, " ");
  }
  
  /* return the number of buttons */
  return cnt;
}

/*
  title1:	Multiple lines,separated by '\n'
  title2:	A single line/string which is terminated by '\0' or '\n' . "title2" accepts the return value from u8x8_GetStringLineStart()
  title3:	Multiple lines,separated by '\n'
  buttons:	one more more buttons separated by '\n' and terminated with '\0'
*/

uint8_t u8x8_UserInterfaceMessage(u8x8_t *u8x8, const char *title1, const char *title2, const char *title3, const char *buttons)
{
  uint8_t height;
  uint8_t y;
  uint8_t cursor = 0;
  uint8_t button_cnt;
  uint8_t event;

  u8x8_SetInverseFont(u8x8, 0);
  
  /* calculate overall height of the message box */
  height = 1;	/* button line */
  height += u8x8_GetStringLineCnt(title1);
  if ( title2 != NULL )
    height ++;
  height += u8x8_GetStringLineCnt(title3);
  
  /* calculate offset from top */
  y = 0;
  if ( height < u8x8_GetRows(u8x8)  )
  {
    y = u8x8_GetRows(u8x8);
    y -= height;
    y /= 2;
  }

  /* draw message box */
  
  u8x8_ClearDisplay(u8x8);   /* required, because not everything is filled */
  
  y += u8x8_DrawUTF8Lines(u8x8, 0, y, u8x8_GetCols(u8x8), title1);
  if ( title2 != NULL )
  {
    u8x8_DrawUTF8Line(u8x8, 0, y, u8x8_GetCols(u8x8), title2);
    y++;
  }
  y += u8x8_DrawUTF8Lines(u8x8, 0, y, u8x8_GetCols(u8x8), title3);

  button_cnt = u8x8_draw_button_line(u8x8, y, u8x8_GetCols(u8x8), cursor, buttons);
  
  for(;;)
  {
    event = u8x8_GetMenuEvent(u8x8);
    if ( event == U8X8_MSG_GPIO_MENU_SELECT )
      return cursor+1;
    else if ( event == U8X8_MSG_GPIO_MENU_HOME )
      break;
    else if ( event == U8X8_MSG_GPIO_MENU_NEXT || event == U8X8_MSG_GPIO_MENU_UP )
    {
      cursor++;
      if ( cursor >= button_cnt )
	cursor = 0;
      u8x8_draw_button_line(u8x8, y, u8x8_GetCols(u8x8), cursor, buttons);
    }
    else if ( event == U8X8_MSG_GPIO_MENU_PREV || event == U8X8_MSG_GPIO_MENU_DOWN  )
    {
      if ( cursor == 0 )
	cursor = button_cnt;
      cursor--;
      u8x8_draw_button_line(u8x8, y, u8x8_GetCols(u8x8), cursor, buttons);
    }    
  }  
  return 0;
}

