/*

  u8x8_d_ist3020.c
  
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

  
*/
#include "u8x8.h"




static const uint8_t u8x8_d_ist3020_erc19264_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a4),		                /* all pixel off, issue 142 */
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ist3020_erc19264_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x0a5),		                /* enter powersafe: all pixel on, issue 142 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ist3020_erc19264_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ist3020_erc19264_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const u8x8_display_info_t u8x8_ist3020_erc19264_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 150,	/* IST3020 datasheet, page 56 */
  /* pre_chip_disable_wait_ns = */ 150,	/* IST3020 datasheet, page 56 */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 1, 
  /* sda_setup_time_ns = */ 100,		/* IST3020 datasheet, page 56 */
  /* sck_pulse_width_ns = */ 100,	/* IST3020 datasheet, page 56 */
  /* sck_clock_hz = */ 4000000UL,	/* */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,	/* IST3020 datasheet, page 54 */
  /* write_pulse_width_ns = */ 60,	/* IST3020 datasheet, page 54 */
  /* tile_width = */ 24,		/* width of 24*8=192 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 64,
  /* pixel_width = */ 192,
  /* pixel_height = */ 64
};

static const uint8_t u8x8_d_ist3020_erc19264_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x0e2),            			/* soft reset */
  U8X8_C(0x0ab),            			/* build in osc on, used in ER code, but not mentioned in data sheet */
  U8X8_C(0x0ae),		                /* display off */
  
  U8X8_C(0x040),		                /* set display start line to 0 */
  
  U8X8_C(0x0a0),		                /* ADC set to reverse */
  U8X8_C(0x0c8),		                /* common output mode */
  // Flipmode
  //U8X8_C(0x0a0),		                /* ADC set to reverse */
  //U8X8_C(0x0c8),		                /* common output mode */
  
  U8X8_C(0x0a6),		                /* display normal, bit val 0: LCD pixel off. */
  U8X8_C(0x0a3),		                /* FIX: LCD bias 1/7, old value was 1/9 (0x0a2) */
  
  U8X8_C(0x028|4),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|6),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|7),		                /* all power  control circuits on */
  U8X8_DLY(50),
  
  U8X8_C(0x020),		                /* v0 voltage resistor ratio */
  U8X8_CA(0x081, 0x019),		/* set contrast, contrast value (from ER code: 45) */
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x0a5),		                /* enter powersafe: all pixel on, issue 142 */
   
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_ist3020_erc19264(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ist3020_erc19264_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ist3020_erc19264_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ist3020_erc19264_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ist3020_erc19264_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ist3020_erc19264_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ist3020_erc19264_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int >> 2 );	/* st7567 has range from 0 to 63 */
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


