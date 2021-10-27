/*

  u8x8_d_uc1601.c
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2017, olikraus@gmail.com
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

  This is for the uc1601s controller
  
  
*/
#include "u8x8.h"




static const uint8_t u8x8_d_uc1601_128x32_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1601_128x32_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1601_128x32_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0c4),				/* bit 1: MX, bit 2: MY */
  U8X8_C(0x060),		                /* set display start line to 32 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1601_128x32_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0c2),				/* bit 1: MX, bit 2: MY */
  U8X8_C(0x040),		                /* set display start line to 0 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const u8x8_display_info_t u8x8_uc1601_128x32_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 1,	/* uc1601 datasheet, page 46 */
  /* pre_chip_disable_wait_ns = */ 5,	/* uc1601 datasheet, page 46 */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 12,		/* uc1601 datasheet, page 44 */
  /* sck_pulse_width_ns = */ 15,	/* uc1601 datasheet, page 44 */
  /* sck_clock_hz = */ 2000000UL,	/* */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 1,
  /* data_setup_time_ns = */ 60,	/* uc1601 datasheet, page 43 */
  /* write_pulse_width_ns = */ 80,	/* uc1601 datasheet, page 43 */
  /* tile_width = */ 16,		
  /* tile_hight = */ 4,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 4,
  /* pixel_width = */ 128,
  /* pixel_height = */ 32
};

static const uint8_t u8x8_d_uc1601_128x32_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x0e2),            			/* soft reset */
  U8X8_C(0x0eb),            			/* LCD Bias: 0xe8: 6, 0xe9: 7, 0xea: 8, 0xeb: 9 */
  //U8X8_C(0x023),            			/* 0x020...0x023 only for UC1601, not for UC1601s */
	

  //U8X8_C(0x02e),            			/* LCD Load + Internal Charge Pump (default: 0x2e) */
  U8X8_C(0x024),		                /* Temperature Compenstation, default: 0x24 */
  U8X8_C(0x089),		                /* RAM address ctrl, default: 0x89 */
  U8X8_C(0x0c4),		                /* RAM mapping ctrl */
  U8X8_C(0x0a0),		                /* Frame Rate, 0x0a0 or 0x0a1 */
  U8X8_CA(0x081, 0x0df),		/* set contrast */
  U8X8_C(0x02e),            			/* LCD Load + Internal Charge Pump (default: 0x2e) */	
  U8X8_C(0x060),		                /* set display start line to 32 */
  
  U8X8_C(0x0a6),		                /* normal display */
   
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_uc1601_128x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_uc1601_128x32_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_uc1601_128x32_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1601_128x32_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1601_128x32_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1601_128x32_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1601_128x32_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* uc1601 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));
    
      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      c *= 8;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      do
      {
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


