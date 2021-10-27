/*

  u8x8_d_st7920.c
  
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


  The ST7920 controller does not support hardware graphics flip.
  Contrast adjustment is done by an external resistor --> no support for contrast adjustment
  
  
*/
#include "u8x8.h"



static const uint8_t u8x8_d_st7920_init_seq[] = {
    
  U8X8_DLY(100),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(10),
  
  U8X8_C(0x038),            			/* 8 Bit interface (DL=1), basic instruction set (RE=0) */
  U8X8_C(0x008),		                /* display on, cursor & blink off; 0x08: all off */
  U8X8_C(0x006),		                /* Entry mode: Cursor move to right ,DDRAM address counter (AC) plus 1, no shift  */  
  U8X8_C(0x002),		                /* disable scroll, enable CGRAM adress  */
  U8X8_C(0x001),		                /* clear RAM, needs 1.6 ms */
  U8X8_DLY(4),					/* delay 2ms */

  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7920_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x038),            			/* 8 Bit interface (DL=1), basic instruction set (RE=0) */
  U8X8_C(0x00c),		                /* display on, cursor & blink off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7920_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x038),            			/* 8 Bit interface (DL=1), basic instruction set (RE=0) */
  U8X8_C(0x008),		                /* display off */
  U8X8_C(0x034), /* 8 Bit interface (DL=1), extended instruction set (RE=1) */
  U8X8_C(0x001), /* Standby mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_st7920_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, y, c, i;
  uint8_t *ptr;
  switch(msg)
  {
    /* U8X8_MSG_DISPLAY_SETUP_MEMORY is handled by the calling function */
    /*
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      break;
    */
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7920_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7920_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7920_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      y = (((u8x8_tile_t *)arg_ptr)->y_pos);
      y*=8;
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x /= 2;		/* not sure whether this is a clever idea, problem is, the ST7920 can address only every second tile */
    
      if ( y >= 32 )	/* this is the adjustment for 128x64 displays */
      {
	y-=32;
	x+=8;
      }
    
      u8x8_cad_StartTransfer(u8x8);
        

      /* 
	Tile structure is reused here for the ST7920, however u8x8 is not supported 
	tile_ptr points to data which has cnt*8 bytes (same as SSD1306 tiles)
	Buffer is expected to have 8 lines of code fitting to the ST7920 internal memory
	"cnt" includes the number of horizontal bytes. width is equal to cnt*8
	Also important: Width must be a multiple of 16 (ST7920 requirement), so cnt must be even.
	
	TODO: Consider arg_int, however arg_int is not used by u8g2
      */
      c = ((u8x8_tile_t *)arg_ptr)->cnt;	/* number of tiles */
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;	/* data ptr to the tiles */
      /* The following byte is sent to allow the ST7920 to sync up with the data */
      /* it solves some issues with garbage data */
      u8x8_cad_SendCmd(u8x8, 0x03e );	/* enable extended mode */
      u8x8_cad_SendCmd(u8x8, 0x03e );	/* enable extended mode, issue 487 */
      for( i = 0; i < 8; i++ )
      {
	//u8x8_cad_SendCmd(u8x8, 0x03e );	/* enable extended mode */
	u8x8_cad_SendCmd(u8x8, 0x080 | (y+i) );      /* y pos  */
	u8x8_cad_SendCmd(u8x8, 0x080 | x );      /* set x pos */
	c = ((u8x8_tile_t *)arg_ptr)->cnt;	/* number of tiles */

	//u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, 200, NULL);	/* extra dely required */
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes, send one line of data */
	ptr += c;
	//u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, 200, NULL);	/* extra dely required */
      }

      u8x8_cad_EndTransfer(u8x8);

      break;
    default:
      return 0;
  }
  return 1;
}

static const u8x8_display_info_t u8x8_st7920_192x32_display_info =
{
  /* chip_enable_level = */ 1,
  /* chip_disable_level = */ 0,
  
  /* post_chip_enable_wait_ns = */ 5,
  /* pre_chip_disable_wait_ns = */ 5,
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 20,		
  /* sck_pulse_width_ns = */  140,	/* datasheet ST7920 */
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 3,		/* old: sck_takeover_edge, new: active high (bit 1), rising edge (bit 0), 18 Aug 16: changed from 1 to 3 which works for 101 */
	/* Arduino mode 3: aktive low clock, but use rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 30,
  /* write_pulse_width_ns = */ 40,
  /* tile_width = */ 24,
  /* tile_hight = */ 4,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 192,
  /* pixel_height = */ 32
};

static const u8x8_display_info_t u8x8_st7920_128x64_display_info =
{
  /* chip_enable_level = */ 1,
  /* chip_disable_level = */ 0,
  
  /* post_chip_enable_wait_ns = */ 5,
  /* pre_chip_disable_wait_ns = */ 5,
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 20,		
  /* sck_pulse_width_ns = */  140,	/* datasheet ST7920 */
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* ST7920+Due work with 1MHz but not with 2MHz, ST7920+Uno works with 2MHz */
  /* spi_mode = */ 3,		/* active high, rising edge, 18 Aug 16: changed from 1 to 3 which works for 101  */
  /* in theory mode 3 should be correct  */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 30,
  /* write_pulse_width_ns = */ 40,
  /* tile_width = */ 16,
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};

uint8_t u8x8_d_st7920_192x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7920_192x32_display_info);
      break;
    default:
      return u8x8_d_st7920_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}

uint8_t u8x8_d_st7920_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7920_128x64_display_info);
      break;
    default:
      return u8x8_d_st7920_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}



  

  