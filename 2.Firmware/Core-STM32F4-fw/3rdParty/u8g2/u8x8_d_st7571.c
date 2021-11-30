/*

  u8x8_d_st7571.c

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2020, olikraus@gmail.com
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


  ST7571: 128x129 2-bit graylevel LCD
  
  https://github.com/olikraus/u8g2/issues/921

*/


#include "u8x8.h"

static const uint8_t u8x8_d_st7571_128x128_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x071),		                /* exit power save mode */
  U8X8_C(0x0a8),		                /* disable powersave mode */
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7571_128x128_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off */  
  U8X8_C(0x0a9),		                /* enter powersave mode */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7571_128x128_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7571_128x128_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};




/*===================================================*/

static uint8_t u8x8_d_st7571_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint16_t x;
  uint8_t c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7571_128x128_display_info);
      break;
    */
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7571_128x128_init_seq);    
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7571_128x128_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7571_128x128_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7571_128x128_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7571_128x128_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int>>2);			// 6 bit for the ST7571
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
    


      do
      {
        c = ((u8x8_tile_t *)arg_ptr)->cnt;
        ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
        /* SendData can not handle more than 255 bytes */
	/*
        if ( c > 31 )
        {
          u8x8_cad_SendData(u8x8, 31*8, ptr); 
          ptr+=31*8;
          c -= 31;
        }
	*/
        
        u8x8_cad_SendData(u8x8, c*8, ptr); 	
        arg_int--;
      } while( arg_int > 0 );

      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}

/*===================================================*/


/* QT-2832TSWUG02/ZJY-2832TSWZG02 */
static const uint8_t u8x8_d_st7571_128x128_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  
  U8X8_C(0xAE), 				// Display OFF
  U8X8_C(0x38), 				// Mode Set  
  U8X8_C(0xB8), 				// FR=1011 (85Hz), BE[1:0]=10, level 3 booster
  
  
  U8X8_C(0xA0), 				// ADC select
  U8X8_C(0xC8), 				// SHL select
  U8X8_CA(0x44, 0x00), 		// COM0 register  
  U8X8_CA(0x40, 0x7f), 		// initial display line  (0x7f... strange but ok... maybe specific for the JLX128128)
  
  U8X8_C(0xAB), 				// OSC ON  
  U8X8_C(0x25), 				// Voltage regulator
  U8X8_CA(0x81, 0x33), 		// Volume
  U8X8_C(0x54), 				// LCD Bias: 0x056=1/11 (1/11 according to JLX128128 datasheet), 0x054=1/9
  U8X8_CA(0x44, 0x7f), 		// Duty 1/128
  
  U8X8_C(0x2C), 				// Power Control, VC: ON, VR: OFF, VF: OFF
  U8X8_DLY(200),
  U8X8_C(0x2E), 				// Power Control, VC: ON, VR: ON, VF: OFF
  U8X8_DLY(200),
  U8X8_C(0x2F), 				// Power Control, VC: ON, VR: ON, VF: ON
  U8X8_DLY(10),

  U8X8_C(0x7B), 				// command set 3
  U8X8_C(0x11), 				// black white mode
  U8X8_C(0x00), 				// exit command set 3


  U8X8_C(0xA6), 				// Display Inverse OFF
  U8X8_C(0xA4), 				// Disable Display All Pixel ON

  //U8X8_C(0xAF), 				// Display on


  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()           			/* end of sequence */
};




static const u8x8_display_info_t u8x8_st7571_128x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* */
  /* sck_pulse_width_ns = */ 40,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* 400KHz */
  /* data_setup_time_ns = */ 15,
  /* write_pulse_width_ns = */ 70,	
  /* tile_width = */ 16,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 128
};

uint8_t u8x8_d_st7571_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
  if ( u8x8_d_st7571_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7571_128x128_init_seq); 
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7571_128x128_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}
