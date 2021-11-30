/*

  u8x8_d_ks0108.c 
  
  The classic 5V LCD
  
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




static const uint8_t u8x8_d_ks0108_init_seq[] = {
  U8X8_C(0x0c0),		                /* satart at the top  */  
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ks0108_powersave0_seq[] = {
  U8X8_C(0x03f),		                /* display on */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ks0108_powersave1_seq[] = {
  U8X8_C(0x03e),		                /* display off */
  U8X8_END()             			/* end of sequence */
};


struct u8x8_ks0108_vars
{
  uint8_t *ptr;
  uint8_t x;
  uint8_t c;
  uint8_t arg_int;
};

static void u8x8_ks0108_out(u8x8_t *u8x8, struct u8x8_ks0108_vars *v, void *arg_ptr)
{
  uint8_t cnt;
  u8x8_cad_SendCmd(u8x8, 0x040 | ((v->x << 3) & 63) );
  u8x8_cad_SendCmd(u8x8, 0x0b8 | (((u8x8_tile_t *)arg_ptr)->y_pos));
  
  while( v->arg_int > 0 )
  {
      /* calculate tiles to next boundary (end or chip limit) */
      cnt = v->x;
      cnt += 8;
      cnt &= 0x0f8;
      cnt -= v->x;
            
      if ( cnt > v->c )
	cnt = v->c;
    
      /* of cours we still could use cnt=1 here... */
      /* but setting cnt to 1 is not very efficient */
      //cnt = 1;
    
      v->x +=cnt;
      v->c-=cnt;
      cnt<<=3;
      u8x8_cad_SendData(u8x8, cnt, v->ptr);	/* note: SendData can not handle more than 255 bytes */    
      v->ptr += cnt;
    
      if ( v->c == 0 )
      {
	v->ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
	v->c = ((u8x8_tile_t *)arg_ptr)->cnt;
	v->arg_int--;
      }
      if ( ((v->x) & 7) == 0 )
	break;       
  } 
}


static const u8x8_display_info_t u8x8_ks0108_128x64_display_info =
{
  /* chip_enable_level = */ 0,		/* KS0108: Not used */
  /* chip_disable_level = */ 1,		/* KS0108: Not used */
  
  /* post_chip_enable_wait_ns = */ 100,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 		/* could be faster for the KS0108 */
  /* sda_setup_time_ns = */ 12,		
  /* sck_pulse_width_ns = */ 75,	/* KS0108: Not used */
  /* sck_clock_hz = */ 4000000UL,	/* KS0108: Not used */
  /* spi_mode = */ 0,				/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* KS0108: Not used */
  /* data_setup_time_ns = */ 200,
  /* write_pulse_width_ns = */ 250,	/* KS0108: actially 450 ns, but additional 200 ns are added by the byte transfer function */
  /* tile_width = */ 16,		/* width of 16*8=128 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};

uint8_t u8x8_d_ks0108_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  struct u8x8_ks0108_vars v;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ks0108_128x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
    
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 1, NULL);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_init_seq);
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 0, NULL);
    
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 2, NULL);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_init_seq);
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 0, NULL);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      
      if ( arg_int == 0 )
      {
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 1, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave0_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 0, NULL);

	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 2, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave0_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 0, NULL);
	
      }
      else
      {
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 1, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave1_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 0, NULL);
	
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 2, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave1_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 0, NULL);
	
      }
      break;
// The KS0108 can not mirror the cols and rows, use U8g2 for rotation
//    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
//      break;
// The KS0108 has no internal contrast command
//    case U8X8_MSG_DISPLAY_SET_CONTRAST:
//      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:

      v.ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      v.x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      v.c = ((u8x8_tile_t *)arg_ptr)->cnt;
      v.arg_int = arg_int;    
      
      
      if ( v.x < 8 )
      {
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 1, NULL);
	u8x8_ks0108_out(u8x8, &v, arg_ptr);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 0, NULL);
      }
      if ( v.x < 16 )
      {
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 2, NULL);
	u8x8_ks0108_out(u8x8, &v, arg_ptr);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 0, NULL);
      }
      //if ( v.x < 24 )
      //{
	//u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 4, NULL);
	//u8x8_ks0108_out(u8x8, &v, arg_ptr);
	//u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 0, NULL);
      //}    
      break;
    default:
      return 0;
  }
  return 1;
}

static const u8x8_display_info_t u8x8_ks0108_192x64_display_info =
{
  /* chip_enable_level = */ 0,		/* KS0108: Not used */
  /* chip_disable_level = */ 1,		/* KS0108: Not used */
  
  /* post_chip_enable_wait_ns = */ 100,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 		/* could be faster for the KS0108 */
  /* sda_setup_time_ns = */ 12,		
  /* sck_pulse_width_ns = */ 75,	/* KS0108: Not used */
  /* sck_clock_hz = */ 4000000UL,	/* KS0108: Not used */
  /* spi_mode = */ 0,				/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* KS0108: Not used */
  /* data_setup_time_ns = */ 200,
  /* write_pulse_width_ns = */ 250,	/* KS0108: actially 450 ns, but additional 200 ns are added by the byte transfer function */
  /* tile_width = */ 24,		/* width of 24*8=192 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 192,
  /* pixel_height = */ 64
};


/* east rising (buydisplay.com) ERM19264 */
/* left: 011, middle: 101, right: 110, no chip select: 111 */
uint8_t u8x8_d_ks0108_erm19264(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  struct u8x8_ks0108_vars v;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ks0108_192x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
    
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 3, NULL);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_init_seq);
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);
    
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 5, NULL);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_init_seq);
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);
    
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 6, NULL);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_init_seq);
      u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      
      if ( arg_int == 0 )
      {
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 3, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave0_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);

	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 5, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave0_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);

	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 6, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave0_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);
	
      }
      else
      {
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 3, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave1_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);

	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 5, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave1_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);

	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 6, NULL);
	u8x8_cad_SendSequence(u8x8, u8x8_d_ks0108_powersave1_seq);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);
	
      }
      break;
// The KS0108 can not mirror the cols and rows, use U8g2 for rotation
//    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
//      break;
// The KS0108 has no internal contrast command
//    case U8X8_MSG_DISPLAY_SET_CONTRAST:
//      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:

      v.ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      v.x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      v.c = ((u8x8_tile_t *)arg_ptr)->cnt;
      v.arg_int = arg_int;    
      
/*
    3-bit CS value:
    In u8x8_byte_set_ks0108_cs(u8x8_t *u8x8, uint8_t arg) the lowest
    bit is assigned to CS and highest bit if the 3-bit value to CS2
    
    CS: left part of the display  --> 6
    CS1: middle part --> 5
    CS2: right part of the display --> 3

    Reference: https://github.com/olikraus/u8g2/issues/631
*/
      if ( v.x < 8 )
      {
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 6, NULL);  // 3-->6, issue 631
	u8x8_ks0108_out(u8x8, &v, arg_ptr);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);
      }
      if ( v.x < 16 )
      {
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 5, NULL);
	u8x8_ks0108_out(u8x8, &v, arg_ptr);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);
      }
      if ( v.x < 24 )
      {
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_START_TRANSFER, 3, NULL); // 6-->3, // issue 631
	u8x8_ks0108_out(u8x8, &v, arg_ptr);
	u8x8->cad_cb(u8x8, U8X8_MSG_CAD_END_TRANSFER, 7, NULL);
      }    
      break;
    default:
      return 0;
  }
  return 1;
}

