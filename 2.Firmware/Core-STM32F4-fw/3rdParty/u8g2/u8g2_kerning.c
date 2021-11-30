/*

  u8g2_kerning.c

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

/* this function is used as "u8g2_get_kerning_cb" */
/*
uint8_t u8g2_GetNullKerning(u8g2_t *u8g2, uint16_t e1, uint16_t e2)
{
  return 0;
}
*/

/* this function is used as "u8g2_get_kerning_cb" */
uint8_t u8g2_GetKerning(U8X8_UNUSED u8g2_t *u8g2, u8g2_kerning_t *kerning, uint16_t e1, uint16_t e2)
{
  uint16_t i1, i2, cnt, end;
  if ( kerning == NULL )
    return 0;
  
  /* search for the encoding in the first table */
  cnt = kerning->first_table_cnt;
  cnt--;	/* ignore the last element of the table, which is 0x0ffff */
  for( i1 = 0; i1 < cnt; i1++ )
  {
    if ( kerning->first_encoding_table[i1] == e1 )
      break;
  }
  if ( i1 >= cnt )
    return 0;	/* e1 not part of the kerning table, return 0 */

  /* get the upper index for i2 */
  end = kerning->index_to_second_table[i1+1];
  for( i2 = kerning->index_to_second_table[i1]; i2 < end; i2++ )
  {
    if ( kerning->second_encoding_table[i2] == e2 )
      break;
  }
  
  if ( i2 >= end )
    return 0;	/* e2 not part of any pair with e1, return 0 */
  
  return kerning->kerning_values[i2];
}

uint8_t u8g2_GetKerningByTable(U8X8_UNUSED u8g2_t *u8g2, const uint16_t *kt, uint16_t e1, uint16_t e2)
{
  uint16_t i;
  i = 0;
  if ( kt == NULL )
    return 0;
  for(;;)
  {
    if ( kt[i] == 0x0ffff )
      break;
    if ( kt[i] == e1 && kt[i+1] == e2 )
      return kt[i+2];
    i+=3;
  }
  return 0;
}

