/*

  u8x8_d_ld7032_60x32.c
  Note: Flip Mode is NOT supported

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


/* testboard U8GLIB_LD7032_60x32 u8g(11, 12, 9, 10, 8);	// SPI Com: SCK = 11, MOSI = 12, CS = 9, A0 = 10, RST = 8  (SW SPI Nano Board) */
/* http://www.seeedstudio.com/document/pdf/0.5OLED%20SPEC.pdf */
#ifdef OBSOLETE
static const uint8_t u8x8_d_ld7032_60x32_init_seq_old[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  //U8X8_CA(0x002, 0x001),		/* Dot Matrix Display ON */
  U8X8_CA(0x014, 0x000),		/* Stand-by OFF */
  U8X8_CA(0x01a, 0x004),		/* Dot Matrix Frame Rate,  special value for this OLED from manual*/
  U8X8_CA(0x01d, 0x000),		/* Graphics Memory Writing Direction: reset default (right down, horizontal) */
  U8X8_CA(0x009, 0x000),		/* Display Direction:  reset default (x,y: min --> max) */
  U8X8_CAA(0x030, 0x000, 0x03b),	/* Display Size X, Column Start - End*/
  U8X8_CAA(0x032, 0x000, 0x01f),	/* Display Size Y, Row Start - End*/
  U8X8_CA(0x010, 0x000),		/* Peak Pulse Width Set: 0 SCLK */
  U8X8_CA(0x016, 0x000),		/* Peak Pulse Delay Set: 0 SCLK */
  U8X8_CA(0x012, 0x040),		/* Dot Matrix Current Level Set: 0x050 * 1 uA = 80 uA */
  U8X8_CA(0x018, 0x003),		/* Pre-Charge Pulse Width: 3 SCLK */
  U8X8_CA(0x044, 0x002),		/* Pre-Charge Mode: Every Time */
  U8X8_CA(0x048, 0x003),		/* Row overlap timing: Pre-Charge + Peak Delay + Peak boot Timing */
  U8X8_CA(0x03f, 0x011),		/* VCC_R_SEL: ??? */
  U8X8_CA(0x03d, 0x000),		/* VSS selection: 2.8V */
  //U8X8_CA(0x002, 0x001),		/* Dot Matrix Display ON */
  
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};
#endif

/* new sequence https://github.com/olikraus/u8g2/issues/865 */
static const uint8_t u8x8_d_ld7032_60x32_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  U8X8_CA(0x02, 0x00),		  		/* Dot Matrix Display OFF */
  U8X8_CA(0x14, 0x00),		  		/* Stand-by OFF, OSCA Start */
  U8X8_CA(0x1a, 0x04),		  		/* Dot Matrix Frame Rate,  special value for this OLED from manual 4 => 120Hz*/
  U8X8_CA(0x1d, 0x00),		  		/* Graphics Memory Writing Direction: reset default (right down, horizontal) */
  U8X8_CA(0x09, 0x00),	      		/* Display Direction:  reset default (x,y: min --> max) */
  U8X8_CAA(0x30, 0x00, 0x3B),  		/* Display Size X, Column Start - End 0-0x3b(59)*/
  U8X8_CAA(0x32, 0x00, 0x1F),  		/* Display Size Y, Row Start - End 0-0x1f(31)*/
  U8X8_CA(0x34, 0x00),				/* Data Reading/Writing Box X start */
  U8X8_CA(0x35, 0x07),				/* Data Reading/Writing Box X end */
  U8X8_CA(0x36, 0x00),				/* Data Reading/Writing Box Y start */
  U8X8_CA(0x37, 0x1F),				/* Data Reading/Writing Box Y end */
  U8X8_CA(0x38, 0x00),        		/* Display Start Address X */
  U8X8_CA(0x39, 0x00),        		/* Display Start Address Y */
  U8X8_CA(0x10, 0x00),		  		/* Peak Pulse Width Set: 0 SCLK */
  U8X8_CA(0x16, 0x00),		  		/* Peak Pulse Delay Set: 0 SCLK */
  U8X8_CA(0x12, 0x40),		  		/* 0x32, 0x50 or 0x40 Dot Matrix Current Level Set: 0x050 * 1 uA = 80 uA */
  U8X8_CA(0x18, 0x03),		  		/* Pre-Charge Pulse Width: 3 SCLK */
  U8X8_CA(0x44, 0x02),		  		/* Pre-Charge Mode: Every Time */
  U8X8_CA(0x48, 0x03),		  		/* Row overlap timing: Pre-Charge + Peak Delay + Peak boot Timing */
  U8X8_CA(0x17, 0x00),          	/* Row Scan */
  U8X8_CA(0x13, 0x00),          	/* Row Scan Sequence Setting */
  U8X8_CA(0x1C, 0x00),          	/* Data Reverse */
  U8X8_CA(0x3f, 0x11),		  		/* VCC_R_SEL: Internal Regulator enabled(D4=1) and VCC_R=VCC_C*0.7(D0=1) */
  U8X8_CA(0x3d, 0x00),		  		/* VSS selection: 2.8V */

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ld7032_60x32_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x002, 0x001),		/* Dot Matrix Display ON */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ld7032_60x32_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x002, 0x000),		/* Dot Matrix Display ON */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ld7032_60x32_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x009, 0x000),		/* Display Direction:  reset default (x,y: min --> max) */  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ld7032_60x32_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  //U8X8_CA(0x009, 0x002),		/* Display Direction:  reset default (x,y: min --> max) */  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static uint8_t u8x8_d_ld7032_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ld7032_60x32_display_info);
      break;
    */
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ld7032_60x32_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ld7032_60x32_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ld7032_60x32_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ld7032_60x32_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ld7032_60x32_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x012 );
      if ( arg_int > 0x07f )			/* default is 0x040, limit to 0x07f to be on the safe side (hopefully) */
	arg_int= 0x07f;
      u8x8_cad_SendArg(u8x8, arg_int );	/* values from 0x00 to 0x0ff are allowed, bit will all values be safe??? */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      x += u8x8->x_offset/8;
      u8x8_cad_SendCmd(u8x8, 0x034 );
      u8x8_cad_SendArg(u8x8, x );
      u8x8_cad_SendCmd(u8x8, 0x035 );
      u8x8_cad_SendArg(u8x8, 0x007 );
      u8x8_cad_SendCmd(u8x8, 0x036 );
      u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos)*8 );
      u8x8_cad_SendCmd(u8x8, 0x037 );
      u8x8_cad_SendArg(u8x8, 0x01f );
      u8x8_cad_SendCmd(u8x8, 0x008 );
    
      
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
	u8x8_cad_SendData(u8x8, c*8, ptr); 	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


static const u8x8_display_info_t u8x8_ld7032_60x32_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 15,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 100, 	
  /* post_reset_wait_ms = */ 100, 
  /* sda_setup_time_ns = */ 30,		/* 20ns, but cycle time is 60ns, so use 60/2 */
  /* sck_pulse_width_ns = */ 30,	/* 20ns, but cycle time is 60ns, so use 60/2  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 20,
  /* write_pulse_width_ns = */ 40,	
  /* tile_width = */ 8,
  /* tile_hight = */ 4,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 60,
  /* pixel_height = */ 32
};

uint8_t u8x8_d_ld7032_60x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
    {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ld7032_60x32_display_info);
      return 1;
    }
    return u8x8_d_ld7032_generic(u8x8, msg, arg_int, arg_ptr);
}



/* alternative version, issue #1189 */

/* new sequence https://github.com/olikraus/u8g2/issues/1189 */
static const uint8_t u8x8_d_ld7032_60x32_alt_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  U8X8_CA(0x02, 0x00),		  		
  U8X8_CA(0x14, 0x00),		  		
  U8X8_CA(0x1A, 0x05),		  		
  U8X8_CA(0x1D, 0x00),		  	
  U8X8_CA(0x09, 0x00),	      		
  U8X8_CAA(0x30, 0x00, 0x3F),
  U8X8_CAA(0x32, 0x08, 0x27),
  U8X8_CA(0x34, 0x00),				
  U8X8_CA(0x35, 0x07),		
  U8X8_CA(0x36, 0x08),	
  U8X8_CA(0x37, 0x27),			
  U8X8_CA(0x38, 0x00),        
  U8X8_CA(0x39, 0x20),       
  U8X8_CA(0x10, 0x05),		  	
  U8X8_CA(0x16, 0x00),		  	
  U8X8_CA(0x18, 0x08),		  		
  U8X8_CA(0x12, 0x2F),		  	
  U8X8_CA(0x3D, 0x01),		  	
  U8X8_CA(0x3F, 0x10),		  		
  U8X8_CA(0x44, 0x02),		  		
  U8X8_CA(0x48, 0x03),		  	
  U8X8_CA(0x17, 0x00),         
  U8X8_CA(0x13, 0x01),        
  U8X8_CA(0x3F, 0x11),
  U8X8_CA(0x3D, 0x00),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */};

uint8_t u8x8_d_ld7032_60x32_alt(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
    {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ld7032_60x32_display_info);
      return 1;
    }

    if ( msg ==U8X8_MSG_DISPLAY_INIT )
    {
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ld7032_60x32_alt_init_seq);    
      return 1;
    }
    
    return u8x8_d_ld7032_generic(u8x8, msg, arg_int, arg_ptr);
}

