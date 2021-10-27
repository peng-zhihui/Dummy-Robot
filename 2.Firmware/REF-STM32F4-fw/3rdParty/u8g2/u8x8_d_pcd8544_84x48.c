/*

  u8x8_d_pcd8544_84x48.c (so called "Nokia 5110" displays)
  
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




static const uint8_t u8x8_d_pcd8544_84x48_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x021),            			/* activate chip (PD=0), horizontal increment (V=0), enter extended command set (H=1) */
  U8X8_C(0x006),		                /* temp. control: b10 = 2  */
  U8X8_C(0x013),		                /* bias system 1:48 */
  U8X8_C(0x0c0),		                /* medium Vop  */
  
  U8X8_C(0x020),		                /* activate chip (PD=0), horizontal increment (V=0), enter normal command set (H=0) */
  U8X8_C(0x008),				/* blank */
  U8X8_C(0x024),		                /* power down (PD=1), horizontal increment (V=0), enter normal command set (H=0) */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_pcd8544_84x48_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x020),		                /* activate chip (PD=0), horizontal increment (V=0), enter normal command set (H=0) */
  U8X8_C(0x00c),				/* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_pcd8544_84x48_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x020),		                /* activate chip (PD=0), horizontal increment (V=0), enter normal command set (H=0) */
  U8X8_C(0x008),				/* blank */
  U8X8_C(0x024),		                /* power down (PD=1), horizontal increment (V=0), enter normal command set (H=0) */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};



static const u8x8_display_info_t u8x8_pcd8544_84x48_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 5,
  /* pre_chip_disable_wait_ns = */ 5,
  /* reset_pulse_width_ms = */ 2, 
  /* post_reset_wait_ms = */ 2, 
  /* sda_setup_time_ns = */ 12,		
  /* sck_pulse_width_ns = */ 75,	/* half of cycle time (100ns according to datasheet), AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 30,
  /* write_pulse_width_ns = */ 40,
  /* tile_width = */ 11,		/* width of 11*8=88 pixel */
  /* tile_hight = */ 6,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 84,
  /* pixel_height = */ 48
};

uint8_t u8x8_d_pcd8544_84x48(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_pcd8544_84x48_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_pcd8544_84x48_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_pcd8544_84x48_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_pcd8544_84x48_powersave1_seq);
      break;
    // case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
    // 	  break; 	NOT SUPPORTED
      
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x021 ); /* command mode, extended function set */
      u8x8_cad_SendCmd(u8x8, 0x080 | (arg_int >> 1) );
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x020 ); /* activate chip (PD=0), horizontal increment (V=0), enter normal command set (H=0) */
      u8x8_cad_SendCmd(u8x8, 0x080 | (x) );	/* set X address */
      u8x8_cad_SendCmd(u8x8, 0x040 | (((u8x8_tile_t *)arg_ptr)->y_pos) );	/* set Y address */
    
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      c *= 8;	
      do
      {
	if ( c + x > 84u )
	{
	  if ( x >= 84u )
	    break;
	  c = 84u;
	  c -= x;
	}
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	x += c;
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


