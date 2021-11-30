/*

  u8x8_d_uc1617.c
  
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





static const uint8_t u8x8_d_uc1617_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  //U8X8_C(0x0ad),            			/* display enable BW Mode*/
  U8X8_C(0x0af),            			/* display enable GS Mode*/
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1617_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ac),		                /* display off, enter sleep mode */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1617_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0c0),            	/* LCD Mapping */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1617_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0c6),            	/* LCD Mapping */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


//static uint8_t u8x8_upscale_4bit(uint8_t x) U8X8_NOINLINE;
static uint8_t u8x8_upscale_4bit(uint8_t x) 
{
	uint8_t y = x;
	y |= (y << 4);		// x = (x | (x << S[2])) & B[2];
	y &= 0x0f;
	y |= (y << 2);		// x = (x | (x << S[1])) & B[1];
	y &= 0x33;
	y |= (y << 1);		// x = (x | (x << S[0])) & B[0];
	y &= 0x55;
  
	y |= (y << 1);		// z = x | (y << 1);
	return y;
}

static uint8_t u8x8_uc1617_tile_half_buffer[8];

static uint8_t *u8x8_convert_tile_for_uc1617_lower4bit(uint8_t *t)
{
  uint8_t i;
  uint8_t *pbuf = u8x8_uc1617_tile_half_buffer;

  for( i = 0; i < 8; i++ )
  {
    *pbuf++ = u8x8_upscale_4bit(*t++);
  }
  return u8x8_uc1617_tile_half_buffer;
}

static uint8_t *u8x8_convert_tile_for_uc1617_upper4bit(uint8_t *t)
{
  uint8_t i;
  uint8_t *pbuf = u8x8_uc1617_tile_half_buffer;

  for( i = 0; i < 8; i++ )
  {
    *pbuf++ = u8x8_upscale_4bit((*t++)>>4);
  }
  return u8x8_uc1617_tile_half_buffer;
}

#ifdef NOT_USED
static uint8_t *u8x8_convert_tile_for_uc1617(uint8_t *t)
{
  uint8_t i;
  uint16_t r;
  static uint8_t buf[16];
  uint8_t *pbuf = buf;

  for( i = 0; i < 8; i++ )
  {
    r = u8x8_upscale_byte(*t++);
    *pbuf = r & 255;
    r >>= 8;
    pbuf+=8;
    *pbuf = r;
    pbuf-=7;
  }
  return buf;
}
#endif


uint8_t u8x8_d_uc1617_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, y, c, a;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    


      y = ((u8x8_tile_t *)arg_ptr)->y_pos;
      y*=2;

      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x060 | (x&15));
      u8x8_cad_SendCmd(u8x8, 0x070 | (x>>4)); 
      u8x8_cad_SendCmd(u8x8, 0x00 | (y));

#ifdef NOT_REQUIRED
      u8x8_cad_SendCmd(u8x8, 0xf8 );	/* disable window */
      u8x8_cad_SendCmd(u8x8, 0xf4 );	/* page start */
      u8x8_cad_SendCmd(u8x8, y );	
      u8x8_cad_SendCmd(u8x8, 0xf5 );	/* x start */
      u8x8_cad_SendCmd(u8x8, x );	
      u8x8_cad_SendCmd(u8x8, 0xf6 );	/* page end */
      u8x8_cad_SendCmd(u8x8, y );	
      u8x8_cad_SendCmd(u8x8, 0xf7 );	/* x end */
      u8x8_cad_SendCmd(u8x8, 127 );	
      u8x8_cad_SendCmd(u8x8, 0xf9 );	/* enable window  */
#endif

      a = arg_int;
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
	do
	{
	  u8x8_cad_SendData(u8x8, 8, u8x8_convert_tile_for_uc1617_lower4bit(ptr));
	  ptr += 8;
	  x += 8;
	  c--;
	} while( c > 0 );	
	a--;
      } while( a > 0 );

      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x060 | (x&15));
      u8x8_cad_SendCmd(u8x8, 0x070 | (x>>4));    
      u8x8_cad_SendCmd(u8x8, 0x00 | (y+1));
      a = arg_int;
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
	do
	{
	  u8x8_cad_SendData(u8x8, 8, u8x8_convert_tile_for_uc1617_upper4bit(ptr));
	  ptr += 8;
	  x += 8;
	  c--;
	} while( c > 0 );	
	a--;
      } while( a > 0 );
          
      u8x8_cad_EndTransfer(u8x8);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1617_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1617_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1617_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1617_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int  );	/* uc1617 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    default:
      return 0;
  }
  return 1;
}

/*================================================*/
/* JLX128128 */

static const uint8_t u8x8_d_uc1617_jlx128128_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x0e2),            			/* reset */
  U8X8_DLY(10),
  
  //U8X8_D1(0x0ff),
  U8X8_C(0x027),            			/* temperature compensation */
  U8X8_C(0x02b),            			/* panel loading: 13-18nF */
  
  U8X8_C(0x02f),            			/* internal pump control */
  U8X8_C(0x0eb),            			/* bias=1/11 */
  U8X8_CA(0x081, 0x028),		/* set contrast */
  //U8X8_C(0x0a9),            			/* used in display datasheet, but cmd not described in controller datasheet */
  
  U8X8_CA(0x0f1, 0x07f),			/* set COM end */
  U8X8_CA(0x0f2, 0x000),		/* display line start */
  U8X8_CA(0x0f3, 127),			/* display line end */
  U8X8_C(0x0a3),            			/* line rate */
  
  U8X8_C(0x0d3),            			/* */
  U8X8_C(0x0d7),            			/* */
  

  //U8X8_C(0x0a5),            			/* all pixel on */

  //U8X8_C(0x0d1),            			/* display pattern */  
  U8X8_C(0x08b),            			/* auto increment */
  U8X8_C(0x0c0),            	/* LCD Mapping */

  //U8X8_C(0x0ad),            			/* display enable BW Mode*/
  //U8X8_C(0x0af),            			/* display enable GS Mode*/
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const u8x8_display_info_t u8x8_uc1617_128x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 10,	/* uc1617 datasheet, page 54, actually 5 */
  /* pre_chip_disable_wait_ns = */ 10,	/* uc1617 datasheet, page 54, actually 5 */
  /* reset_pulse_width_ms = */ 10, 
  /* post_reset_wait_ms = */ 20, 	/* uc1617 datasheet, page 56 */
  /* sda_setup_time_ns = */ 24,		/* uc1617 datasheet, page 54 */
  /* sck_pulse_width_ns = */ 45,	/* half of cycle time  uc1617 datasheet, page 54*/
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 30,	/* uc1617 datasheet, page 52 */
  /* write_pulse_width_ns = */ 65,	/* uc1617 datasheet, page 52 */
  /* tile_width = */ 16,		/* width of 16*8=128 pixel */
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 128
};

uint8_t u8x8_d_uc1617_jlx128128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call common procedure first and handle messages there */
  if ( u8x8_d_uc1617_common(u8x8, msg, arg_int, arg_ptr) == 0 )
  {
    /* msg not handled, then try here */
    switch(msg)
    {
      case U8X8_MSG_DISPLAY_SETUP_MEMORY:
	u8x8_d_helper_display_setup_memory(u8x8, &u8x8_uc1617_128x128_display_info);
	break;
      case U8X8_MSG_DISPLAY_INIT:
	u8x8_d_helper_display_init(u8x8);
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1617_jlx128128_init_seq);
	break;
      default:
	return 0;		/* msg unknown */
    }
  }
  return 1;
}
