/*

  u8x8_d_st75320.c

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2019, olikraus@gmail.com
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


  ST75320: 320x240 monochrome LCD
  
  https://github.com/olikraus/u8g2/issues/921

*/


#include "u8x8.h"

static const uint8_t u8x8_d_st75320_jlx320240_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75320_jlx320240_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75320_jlx320240_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0xC4, 0x02), 			/* COM Output Status, Bits 0 & 1 */
  U8X8_C(0xA1), 				/* Column Address Direction: Bit 0 */
  //U8X8_C(0x0a1),				/* segment remap a0/a1*/
  //U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75320_jlx320240_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  //U8X8_C(0x0a0),				/* segment remap a0/a1*/
  //U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  U8X8_CA(0xC4, 0x03), 			/* COM Output Status, Bits 0 & 1 */
  U8X8_C(0xA0), 				/* Column Address Direction: Bit 0 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};




/*===================================================*/

static uint8_t u8x8_d_st75320_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint16_t x;
  uint8_t c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75320_jlx320240_display_info);
      break;
    */
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st75320_jlx320240_init_seq);    
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st75320_jlx320240_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st75320_jlx320240_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st75320_jlx320240_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st75320_jlx320240_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int<<2 );	
      u8x8_cad_SendArg(u8x8, arg_int>>6 );	
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      x *= 8;
      x += u8x8->x_offset;

      u8x8_cad_StartTransfer(u8x8);
    
      u8x8_cad_SendCmd(u8x8, 0x013);
      u8x8_cad_SendArg(u8x8, (x>>8) );
      u8x8_cad_SendArg(u8x8, (x&255) );
      u8x8_cad_SendCmd(u8x8, 0x0b1 ); 
      u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos)); 


      u8x8_cad_SendCmd(u8x8, 0x01d );		// write data 
    
      do
      {
        c = ((u8x8_tile_t *)arg_ptr)->cnt;
        ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
        /* SendData can not handle more than 255 bytes */
        if ( c > 31 )
        {
          u8x8_cad_SendData(u8x8, 248, ptr); 	/* 31*8=248 */
          ptr+=248;
          c -= 31;
        }
        
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
static const uint8_t u8x8_d_st75320_jlx320240_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0xAE), 				// Display OFF
  U8X8_CA(0xEA, 0x00), 			// Power Discharge Control, Discharge OFF
  U8X8_C(0xA8), 				// sleep out
  U8X8_C(0xAB), 				// OSC ON
  U8X8_C(0x69), 				// Temperature Detection ON
  U8X8_C(0x4E), 				// TC Setting
  U8X8_A8(0xff, 0x44, 0x12, 0x11,  0x11, 0x11, 0x22, 0x23),
  U8X8_CAA(0x39, 0x00, 0x00), 	//TC Flag
  
  
  U8X8_CA(0x2B, 0x00), 			// Frame Rate Level
  U8X8_CAA(0x5F, 0x66, 0x66), 	// Set Frame Frequency, fFR=80Hz in all temperature range
  U8X8_CAAA(0xEC, 0x19, 0x64, 0x6e), // FR Compensation Temp. Range, TA = -15 degree, TB = 60 degree, TC = 70 degree
  U8X8_CAA(0xED, 0x04, 0x04), 	// Temp. Hysteresis Value (thermal sensitivity)
  U8X8_C(0xA6), 				// Display Inverse OFF
  U8X8_C(0xA4), 				// Disable Display All Pixel ON

  U8X8_CA(0xC4, 0x02), 			// COM Output Status  
  U8X8_C(0xA1), 				// Column Address Direction: MX=0
  
  U8X8_CAA(0x6D, 0x07, 0x00), 	// Display Area, Duty = 1/240 duty, Start Group = 1
  U8X8_C(0x84), 				// Display Data Input Direction: Column
  U8X8_CA(0x36, 0x1e), 			// Set N-Line
  U8X8_C(0xE4), 				// N-Line On
  U8X8_CA(0xE7, 0x19), 			// LCD Drive Method //NLFR=1//

  U8X8_CAA(0x81, 0x4f, 0x01), 	// OX81: Set EV=64h, 0..255, 0..3
  U8X8_CA(0xA2, 0x0a), 			// BIAS //1/16 BIAS
  U8X8_CA(0x25, 0x020), 		// Power Control //AVDD ON
  U8X8_DLY(10),
  U8X8_CA(0x25, 0x60), 			// Power Control//AVDD, MV3 & NAVDD ON
  U8X8_DLY(10),
  U8X8_CA(0x25, 0x70), 			// Power Control //AVDD, MV3, NAVDD & V3 ON
  U8X8_DLY(10),
  U8X8_CA(0x25, 0x78), 			// Power Control//AVDD, MV3, NAVDD, V3 & VPF ON
  U8X8_DLY(10),
  U8X8_CA(0x25, 0x7c), 			// Power Control//AVDD, MV3, NAVDD, V3, VPF & VNF ON
  U8X8_DLY(10),
  U8X8_CA(0x25, 0x7e), 			// Power Control//VOUT, AVDD, MV3, NAVDD, V3, VPF & VNF ON
  U8X8_DLY(10),
  U8X8_CA(0x25, 0x7f), 			// Power Control/VOUT, AVDD, MV3, NAVDD, V3, VPF & VNF ON
  U8X8_DLY(10),
  //U8X8_C(0xaf); //Display ON  
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()           			/* end of sequence */
};




static const u8x8_display_info_t u8x8_st75320_jlx320240_display_info =
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
  /* tile_width = */ 40,
  /* tile_hight = */ 30,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 320,
  /* pixel_height = */ 240
};

uint8_t u8x8_d_st75320_jlx320240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
  if ( u8x8_d_st75320_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st75320_jlx320240_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75320_jlx320240_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}
