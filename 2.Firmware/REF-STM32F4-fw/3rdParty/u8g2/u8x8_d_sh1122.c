/*

  u8x8_d_sh1122.c
  
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


  Copied from sh1122 mostly because of the similar RAM architecture.
  However: Commands are very different!
  
*/
#include "u8x8.h"




static const uint8_t u8x8_d_sh1122_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* sh1122: display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_sh1122_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* sh1122: display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};




/*
  input:
    one tile (8 Bytes)
  output:
    Tile for SH1122 (32 Bytes)
*/

/*
static uint8_t u8x8_sh1122_to32_dest_buf[32];

static uint8_t *u8x8_sh1122_8to32(U8X8_UNUSED u8x8_t *u8x8, uint8_t *ptr)
{
  uint8_t v;
  uint8_t a,b;
  uint8_t i, j;
  uint8_t *dest;
  
  for( j = 0; j < 4; j++ )
  {
    dest = u8x8_sh1122_to32_dest_buf;
    dest += j;
    a =*ptr;
    ptr++;
    b = *ptr;
    ptr++;
    for( i = 0; i < 8; i++ )
    {
      v = 0;
      if ( a&1 ) v |= 0xf0;
      if ( b&1 ) v |= 0x0f;
      *dest = v;
      dest+=4;
      a >>= 1;
      b >>= 1;
    }
  }
  
  return u8x8_sh1122_to32_dest_buf;
}
*/


static uint8_t u8x8_write_byte_to_16gr_device(u8x8_t *u8x8, uint8_t b)
{
  static uint8_t buf[4];
  static uint8_t map[4] = { 0, 0x00f, 0x0f0, 0x0ff };
  buf [3] = map[b & 3];
  b>>=2;
  buf [2] = map[b & 3];
  b>>=2;
  buf [1] = map[b & 3];
  b>>=2;
  buf [0] = map[b & 3];
  return u8x8_cad_SendData(u8x8, 4, buf);
}

uint8_t u8x8_d_sh1122_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x; 
  uint8_t y, c, i;
  uint8_t *ptr;
  switch(msg)
  {
    /* U8X8_MSG_DISPLAY_SETUP_MEMORY is handled by the calling function */
    /*
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1122_256x64_init_seq);
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_sh1122_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_sh1122_powersave1_seq);
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* sh1122 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      x *= 2;		// only every 4th col can be addressed
      x += u8x8->x_offset;		
    
      y = (((u8x8_tile_t *)arg_ptr)->y_pos);
      y *= 8;
          
      
      c = ((u8x8_tile_t *)arg_ptr)->cnt;	/* number of tiles */
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;	/* data ptr to the tiles */
      for( i = 0; i < 8; i++ )
      {
	u8x8_cad_SendCmd(u8x8, 0x0b0 );	/* set row address */
	u8x8_cad_SendArg(u8x8, y);
	u8x8_cad_SendCmd(u8x8, x & 15 );	/* lower 4 bit*/
	u8x8_cad_SendCmd(u8x8, 0x010 | (x >> 4) );	/* higher 3 bit */	  
	c = ((u8x8_tile_t *)arg_ptr)->cnt;	/* number of tiles */

	while (  c > 0 )
	{
	  u8x8_write_byte_to_16gr_device(u8x8, *ptr);
	  c--;
	  ptr++;
	}
	y++;
      }

      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}

/*=========================================================*/

static const uint8_t u8x8_d_sh1122_256x64_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),		/* remap */
  U8X8_C(0x0c8),		/* remap */
  U8X8_C(0x060),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_sh1122_256x64_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),		/* remap */
  U8X8_C(0x0c0),		/* remap */
  U8X8_C(0x040),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const u8x8_display_info_t u8x8_sh1122_256x64_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 10, 	/* sh1122: 10 us */
  /* post_reset_wait_ms = */ 20, 	/* */
  /* sda_setup_time_ns = */ 125,		/* sh1122: cycle time is 250ns, so use 250/2 */
  /* sck_pulse_width_ns = */ 125,	/* sh1122: cycle time is 250ns, so use 250/2 */
  /* sck_clock_hz = */ 40000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns  */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 10,
  /* write_pulse_width_ns = */ 150,	/* sh1122: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 32,		/* 256 pixel, so we require 32 bytes for this */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,	/* this is the byte offset (there are two pixel per byte with 4 bit per pixel) */
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 256,
  /* pixel_height = */ 64
};


static const uint8_t u8x8_d_sh1122_256x64_init_seq[] = {
    
  U8X8_DLY(1),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(1),
  
  U8X8_C(0xae),		                /* display off */
  U8X8_C(0x40),				/* display start line */  
  U8X8_C(0x0a0),		/* remap */
  U8X8_C(0x0c0),		/* remap */
  U8X8_CA(0x81, 0x80),			/* set display contrast  */  
  U8X8_CA(0xa8, 0x3f),			/* multiplex ratio 1/64 Duty (0x0F~0x3F) */  
  U8X8_CA(0xad, 0x81),			/* use buildin DC-DC with 0.6 * 500 kHz */  
  
  U8X8_CA(0xd5, 0x50),			/* set display clock divide ratio (lower 4 bit)/oscillator frequency (upper 4 bit)  */  
  U8X8_CA(0xd3, 0x00),			/* display offset, shift mapping ram counter */  
  U8X8_CA(0xd9, 0x22),			/* pre charge (lower 4 bit) and discharge(higher 4 bit) period */  
  U8X8_CA(0xdb, 0x35),			/* VCOM deselect level */  
  U8X8_CA(0xdc, 0x35),			/* Pre Charge output voltage */  
  U8X8_C(0x030),				/* discharge level */

  U8X8_DLY(1),					/* delay  */

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_sh1122_256x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_sh1122_256x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1122_256x64_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_sh1122_256x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_sh1122_256x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
    
    default:
      return u8x8_d_sh1122_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}

