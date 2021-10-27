/*

  u8g2_selection_list.c
  
  selection list with scroll option
  
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

#define MY_BORDER_SIZE 1


/*
  Draw a string at x,y
  Center string within w (left adjust if w < pixel len of s)
  
  Side effects:
    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFontPosBaseline(u8g2);

*/
void u8g2_DrawUTF8Line(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, const char *s, uint8_t border_size, uint8_t is_invert)
{
  u8g2_uint_t d, str_width;
  u8g2_uint_t fx, fy, fw, fh;

  /* only horizontal strings are supported, so force this here */
  u8g2_SetFontDirection(u8g2, 0);

  /* revert y position back to baseline ref */
  y += u8g2->font_calc_vref(u8g2);   

  /* calculate the width of the string in pixel */
  str_width = u8g2_GetUTF8Width(u8g2, s);

  /* calculate delta d within the box */
  d = 0;
  if ( str_width < w )
  {
    d = w;
    d -=str_width;
    d /= 2;
  }
  else
  {
    w = str_width;
  }

  /* caluclate text box */
  fx = x;
  fy = y - u8g2_GetAscent(u8g2) ;
  fw = w;
  fh = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2) ;

  /* draw the box, if inverted */
  u8g2_SetDrawColor(u8g2, 1);
  if ( is_invert )
  {
    u8g2_DrawBox(u8g2, fx, fy, fw, fh);
  }

  /* draw the frame */
  while( border_size > 0 )
  {
    fx--;
    fy--;
    fw +=2;
    fh +=2;
    u8g2_DrawFrame(u8g2, fx, fy, fw, fh );
    border_size--;
  }

  if ( is_invert )
  {
    u8g2_SetDrawColor(u8g2, 0);
  }
  else
  {
    u8g2_SetDrawColor(u8g2, 1);
  }

  /* draw the text */
  u8g2_DrawUTF8(u8g2, x+d, y, s);

  /* revert draw color */
  u8g2_SetDrawColor(u8g2, 1);

}


/*
  draw several lines at position x,y.
  lines are stored in s and must be separated with '\n'.
  lines can be centered with respect to "w"
  if s == NULL nothing is drawn and 0 is returned
  returns the number of lines in s multiplied with line_height
*/
u8g2_uint_t u8g2_DrawUTF8Lines(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t line_height, const char *s)
{
  uint8_t i;
  uint8_t cnt;
  u8g2_uint_t yy = 0;
  cnt = u8x8_GetStringLineCnt(s);
  //printf("str=%s\n", s);
  //printf("cnt=%d, y=%d, line_height=%d\n", cnt, y, line_height);
  for( i = 0; i < cnt; i++ )
  {
    //printf("  i=%d, y=%d, line_height=%d\n", i, y, line_height);
    u8g2_DrawUTF8Line(u8g2, x, y, w, u8x8_GetStringLineStart(i, s), 0, 0);
    y+=line_height;
    yy+=line_height;
  }
  return yy;
}

/*
  selection list with string line
  returns line height
*/
static u8g2_uint_t u8g2_draw_selection_list_line(u8g2_t *u8g2, u8sl_t *u8sl, u8g2_uint_t y, uint8_t idx, const char *s) U8G2_NOINLINE;
static u8g2_uint_t u8g2_draw_selection_list_line(u8g2_t *u8g2, u8sl_t *u8sl, u8g2_uint_t y, uint8_t idx, const char *s)
{
  u8g2_uint_t yy;
  uint8_t border_size = 0;
  uint8_t is_invert = 0;
	
  u8g2_uint_t line_height = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2)+MY_BORDER_SIZE;

  /* calculate offset from display upper border */
  yy = idx;
  yy -= u8sl->first_pos;
  yy *= line_height;
  yy += y;

  /* check whether this is the current cursor line */
  if ( idx == u8sl->current_pos )
  {
    border_size = MY_BORDER_SIZE;
    is_invert = 1;
  }

  /* get the line from the array */
  s = u8x8_GetStringLineStart(idx, s);

  /* draw the line */
  if ( s == NULL )
    s = "";
  u8g2_DrawUTF8Line(u8g2, MY_BORDER_SIZE, y, u8g2_GetDisplayWidth(u8g2)-2*MY_BORDER_SIZE, s, border_size, is_invert);
  return line_height;
}

void u8g2_DrawSelectionList(u8g2_t *u8g2, u8sl_t *u8sl, u8g2_uint_t y, const char *s)
{
  uint8_t i;
  for( i = 0; i < u8sl->visible; i++ )
  {
    y += u8g2_draw_selection_list_line(u8g2, u8sl, y, i+u8sl->first_pos, s);
  }
}


/*
  title: 		NULL for no title, valid str for title line. Can contain mutliple lines, separated by '\n'
  start_pos: 	default position for the cursor, first line is 1.
  sl:			string list (list of strings separated by \n)
  returns 0 if user has pressed the home key
  returns the selected line if user has pressed the select key
  side effects:
    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFontPosBaseline(u8g2);
	
*/
uint8_t u8g2_UserInterfaceSelectionList(u8g2_t *u8g2, const char *title, uint8_t start_pos, const char *sl)
{
  u8sl_t u8sl;
  u8g2_uint_t yy;

  uint8_t event;

  u8g2_uint_t line_height = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2)+MY_BORDER_SIZE;

  uint8_t title_lines = u8x8_GetStringLineCnt(title);
  uint8_t display_lines;

  
  if ( start_pos > 0 )	/* issue 112 */
    start_pos--;		/* issue 112 */


  if ( title_lines > 0 )
  {
	display_lines = (u8g2_GetDisplayHeight(u8g2)-3) / line_height;
	u8sl.visible = display_lines;
	u8sl.visible -= title_lines;
  }
  else
  {
	display_lines = u8g2_GetDisplayHeight(u8g2) / line_height;
	u8sl.visible = display_lines;
  }

  u8sl.total = u8x8_GetStringLineCnt(sl);
  u8sl.first_pos = 0;
  u8sl.current_pos = start_pos;

  if ( u8sl.current_pos >= u8sl.total )
    u8sl.current_pos = u8sl.total-1;
  if ( u8sl.first_pos+u8sl.visible <= u8sl.current_pos )
    u8sl.first_pos = u8sl.current_pos-u8sl.visible+1;

  u8g2_SetFontPosBaseline(u8g2);
  
  for(;;)
  {
      u8g2_FirstPage(u8g2);
      do
      {
        yy = u8g2_GetAscent(u8g2);
        if ( title_lines > 0 )
        {
          yy += u8g2_DrawUTF8Lines(u8g2, 0, yy, u8g2_GetDisplayWidth(u8g2), line_height, title);
		
	  u8g2_DrawHLine(u8g2, 0, yy-line_height- u8g2_GetDescent(u8g2) + 1, u8g2_GetDisplayWidth(u8g2));
		
	  yy += 3;
        }
        u8g2_DrawSelectionList(u8g2, &u8sl, yy, sl);
      } while( u8g2_NextPage(u8g2) );
      
#ifdef U8G2_REF_MAN_PIC
      return 0;
#endif


      for(;;)
      {
        event = u8x8_GetMenuEvent(u8g2_GetU8x8(u8g2));
        if ( event == U8X8_MSG_GPIO_MENU_SELECT )
          return u8sl.current_pos+1;		/* +1, issue 112 */
        else if ( event == U8X8_MSG_GPIO_MENU_HOME )
          return 0;				/* issue 112: return 0 instead of start_pos */
        else if ( event == U8X8_MSG_GPIO_MENU_NEXT || event == U8X8_MSG_GPIO_MENU_DOWN )
        {
          u8sl_Next(&u8sl);
          break;
        }
        else if ( event == U8X8_MSG_GPIO_MENU_PREV || event == U8X8_MSG_GPIO_MENU_UP )
        {
          u8sl_Prev(&u8sl);
          break;
        }
      }
  }
}
