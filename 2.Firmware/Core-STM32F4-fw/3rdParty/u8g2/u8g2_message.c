/*

  u8g2_message.c
  
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

#define SPACE_BETWEEN_BUTTONS_IN_PIXEL 6
#define SPACE_BETWEEN_TEXT_AND_BUTTONS_IN_PIXEL 3

uint8_t u8g2_draw_button_line(u8g2_t *u8g2, u8g2_uint_t y, u8g2_uint_t w, uint8_t cursor, const char *s)
{
  u8g2_uint_t button_line_width;
	
  uint8_t i;
  uint8_t cnt;
  uint8_t is_invert;
	
  u8g2_uint_t d;
  u8g2_uint_t x;
	
  cnt = u8x8_GetStringLineCnt(s);
  
	
  /* calculate the width of the button line */
  button_line_width = 0;
  for( i = 0; i < cnt; i++ )
  {
    button_line_width += u8g2_GetUTF8Width(u8g2, u8x8_GetStringLineStart(i, s));
  }
  button_line_width += (cnt-1)*SPACE_BETWEEN_BUTTONS_IN_PIXEL;	/* add some space between the buttons */
  
  /* calculate the left offset */
  d = 0;
  if ( button_line_width < w )
  {
    d = w;
    d -= button_line_width;
    d /= 2;
  }
  
  /* draw the buttons */
  x = d;
  for( i = 0; i < cnt; i++ )
  {
    is_invert = 0;
    if ( i == cursor )
      is_invert = 1;

    u8g2_DrawUTF8Line(u8g2, x, y, 0, u8x8_GetStringLineStart(i, s), 1, is_invert);
    x += u8g2_GetUTF8Width(u8g2, u8x8_GetStringLineStart(i, s));
    x += SPACE_BETWEEN_BUTTONS_IN_PIXEL;
  }
  
  /* return the number of buttons */
  return cnt;
}

/*
  title1:	Multiple lines,separated by '\n'
  title2:	A single line/string which is terminated by '\0' or '\n' . "title2" accepts the return value from u8x8_GetStringLineStart()
  title3:	Multiple lines,separated by '\n'
  buttons:	one more more buttons separated by '\n' and terminated with '\0'
  side effects:
    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFontPosBaseline(u8g2);
*/

uint8_t u8g2_UserInterfaceMessage(u8g2_t *u8g2, const char *title1, const char *title2, const char *title3, const char *buttons)
{
  uint8_t height;
  uint8_t line_height;
  u8g2_uint_t pixel_height;
  u8g2_uint_t y, yy;
	
  uint8_t cursor = 0;
  uint8_t button_cnt;
  uint8_t event;
	
  /* only horizontal strings are supported, so force this here */
  u8g2_SetFontDirection(u8g2, 0);

  /* force baseline position */
  u8g2_SetFontPosBaseline(u8g2);
	
	
  /* calculate line height */
  line_height = u8g2_GetAscent(u8g2);
  line_height -= u8g2_GetDescent(u8g2);

  /* calculate overall height of the message box in lines*/
  height = 1;	/* button line */
  height += u8x8_GetStringLineCnt(title1);
  if ( title2 != NULL )
    height++;
  height += u8x8_GetStringLineCnt(title3);
  
  /* calculate the height in pixel */
  pixel_height = height;
  pixel_height *= line_height;
  
  /* ... and add the space between the text and the buttons */
  pixel_height +=SPACE_BETWEEN_TEXT_AND_BUTTONS_IN_PIXEL;
  
  /* calculate offset from top */
  y = 0;
  if ( pixel_height < u8g2_GetDisplayHeight(u8g2)   )
  {
    y = u8g2_GetDisplayHeight(u8g2);
    y -= pixel_height;
    y /= 2;
  }
  y += u8g2_GetAscent(u8g2);

  
  for(;;)
  {
      u8g2_FirstPage(u8g2);
      do
      {
	  yy = y;
	  /* draw message box */
	  
	  yy += u8g2_DrawUTF8Lines(u8g2, 0, yy, u8g2_GetDisplayWidth(u8g2), line_height, title1);
	  if ( title2 != NULL )
	  {
	    u8g2_DrawUTF8Line(u8g2, 0, yy, u8g2_GetDisplayWidth(u8g2), title2, 0, 0);
	    yy+=line_height;
	  }
	  yy += u8g2_DrawUTF8Lines(u8g2, 0, yy, u8g2_GetDisplayWidth(u8g2), line_height, title3);
	  yy += SPACE_BETWEEN_TEXT_AND_BUTTONS_IN_PIXEL;

	  button_cnt = u8g2_draw_button_line(u8g2, yy, u8g2_GetDisplayWidth(u8g2), cursor, buttons);
	  
      } while( u8g2_NextPage(u8g2) );

#ifdef U8G2_REF_MAN_PIC
      return 0;
#endif
	  
      for(;;)
      {
	    event = u8x8_GetMenuEvent(u8g2_GetU8x8(u8g2));
	    if ( event == U8X8_MSG_GPIO_MENU_SELECT )
	      return cursor+1;
	    else if ( event == U8X8_MSG_GPIO_MENU_HOME )
	      return 0;
	    else if ( event == U8X8_MSG_GPIO_MENU_NEXT || event == U8X8_MSG_GPIO_MENU_DOWN )
	    {
	      cursor++;
	      if ( cursor >= button_cnt )
		cursor = 0;
	      break;
	    }
	    else if ( event == U8X8_MSG_GPIO_MENU_PREV || event == U8X8_MSG_GPIO_MENU_UP )
	    {
	      if ( cursor == 0 )
		cursor = button_cnt;
	      cursor--;
	      break;
	    }    
      }
  }
  /* never reached */
  //return 0;
}

