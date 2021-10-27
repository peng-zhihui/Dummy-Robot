/*

  u8x8_d_ssd1327.c

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


static const uint8_t u8x8_d_ssd1327_96x96_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1327_96x96_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const uint8_t u8x8_d_ssd1327_seeed_96x96_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0a2, 0x020),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x051),		/* remap configuration */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1327_seeed_96x96_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0a2, 0x060),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x042),		/* remap configuration */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const uint8_t u8x8_d_ssd1327_winstar_96x64_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(0x0a0, 0x042),		/* remap configuration */
  U8X8_CA(0x0a2, 0x000),		/* display offset, shift mapping ram counter */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1327_winstar_96x64_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0a0, 0x051),		/* remap configuration */
  U8X8_CA(0x0a2, 0x040),		/* display offset, shift mapping ram counter */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

/*
  input:
    one tile (8 Bytes)
  output:
    Tile for ssd1327 (32 Bytes)
*/

static uint8_t u8x8_ssd1327_8to32_dest_buf[32];

static uint8_t *u8x8_ssd1327_8to32(U8X8_UNUSED u8x8_t *u8x8, uint8_t *ptr)
{
  uint8_t v;
  uint8_t a,b;
  uint8_t i, j;
  uint8_t *dest;
  
  for( j = 0; j < 4; j++ )
  {
    dest = u8x8_ssd1327_8to32_dest_buf;
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
  
  return u8x8_ssd1327_8to32_dest_buf;
}




static uint8_t u8x8_d_ssd1327_96x96_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, y, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1327_96x96_display_info);
      break;
    */
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_96x96_init_seq);    
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_96x96_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_96x96_powersave1_seq);
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* ssd1327 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      x *= 4;
      x+=u8x8->x_offset/2;
    
      y = (((u8x8_tile_t *)arg_ptr)->y_pos);
      y *= 8;
    
      u8x8_cad_SendCmd(u8x8, 0x075 );	/* set row address, moved out of the loop (issue 302) */
      u8x8_cad_SendArg(u8x8, y);
      u8x8_cad_SendArg(u8x8, y+7);
	  
      
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;

	do
	{
	  u8x8_cad_SendCmd(u8x8, 0x015 );	/* set column address */
	  u8x8_cad_SendArg(u8x8, x );	/* start */
	  u8x8_cad_SendArg(u8x8, x+3 );	/* end */

	  
	  u8x8_cad_SendData(u8x8, 32, u8x8_ssd1327_8to32(u8x8, ptr));
	  ptr += 8;
	  x += 4;
	  c--;
	} while( c > 0 );
	
	//x += 4;
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}

/*=============================================*/
/*
  Winstar WEA009664B 96x64 OLED Display, 1.1 inch OLED
  https://www.winstar.com.tw/products/oled-module/graphic-oled-display/96x64-oled.html

  https://github.com/olikraus/u8g2/issues/1050
*/

static const u8x8_display_info_t u8x8_ssd1327_winstar_96x64_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	
  /* post_reset_wait_ms = */ 100, 		/**/
  /* sda_setup_time_ns = */ 100,		/* */
  /* sck_pulse_width_ns = */ 100,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 1,	/* use 1 instead of 4, because the SSD1327 seems to be very slow */
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 60,	
  /* tile_width = */ 12,
  /* tile_hight = */ 8,
  /* default_x_offset = */ 16,		/* changed to 16, issue 1050 */
  /* flipmode_x_offset = */ 16,		/* changed to 16, issue 1050 */
  /* pixel_width = */ 96,
  /* pixel_height = */ 64
};

/*
	Write_Cmd(0xAE);	//Set Display Off     OK
	Write_Cmd(0x81);	//Contrast Level  OK
  	Write_Cmd(0xdF);	//			VALUE WRONG????
	Write_Cmd(0xD9);	//Pre-charge Period
  	Write_Cmd(0x00);
	Write_Cmd(0xA0);	//Set Re-map		OK
	Write_Cmd(0x42);	//Default Setting	OK
	Write_Cmd(0xA1);	//Set Display Start Line	OK
	Write_Cmd(0x00);						OK
	Write_Cmd(0xA2);	//Set Display Offset		OK
	Write_Cmd(0x00);						OK
	Write_Cmd(0xA4);	//Set Display Mode		OK
	Write_Cmd(0xA8);	//Set Multiplex Ratio     	OK
	Write_Cmd(0x63);	//Multiplex			OK
	Write_Cmd(0xAB);	//Set Function SelectionA OK
	Write_Cmd(0x01);						OK
	Write_Cmd(0xB1);	//Set Phase Length		OK
	Write_Cmd(0x47);						OK
	Write_Cmd(0xB3);	//Set Display Clock Divide Ratio/Oscillator Frequency	OK
	Write_Cmd(0x00);						OK
	Write_Cmd(0xBC);	//Set Prechange Voltage	OK
	Write_Cmd(0x07);						OK
	Write_Cmd(0xBE);	//Set VCOMH Voltage	OK
	Write_Cmd(0x07);						OK
	Write_Cmd(0xB6);	//Set Second Pre-charge period	OK
	Write_Cmd(0x04);								OK
	Write_Cmd(0xD5);	//Set Function selection B		OK
	Write_Cmd(0x62);								OK
	Write_Cmd(0xAF);	//Set Display On

*/

static const uint8_t u8x8_d_ssd1327_winstar_96x64_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_CA(0x0fd, 0x012),		/* unlock display, usually not required because the display is unlocked after reset */
  U8X8_C(0x0ae),		                /* display off */
  
  U8X8_CA(0x0d9, 0x000),		/* Pre-charge Period ??? */
  
  U8X8_CA(0x0a0, 0x042),		/* remap configuration */
  U8X8_CA(0x0a1, 0x000),		/* display start line */  
  U8X8_CA(0x0a2, 0x000),		/* display offset, shift mapping ram counter */
  
  U8X8_CA(0x0a8, 0x063),		/* multiplex ratio: 63* 1/64 duty */ /* changed to hex, issue 1050 */
  
  U8X8_CA(0x0ab, 0x001),		/* Enable internal VDD regulator (RESET) */
  U8X8_CA(0x081, 0x053),		/* contrast, brightness, 0..128 */
  
  U8X8_CA(0x0b1, 0x047),                    /* phase length */  
  //U8X8_CA(0x0b3, 0x001),		/* set display clock divide ratio/oscillator frequency  */			
  U8X8_CA(0x0b3, 0x000),		/* set display clock divide ratio/oscillator frequency  */			
  
  U8X8_C(0x0b9),				/* use linear lookup table */

  U8X8_CA(0x0bc, 0x007),                    /* pre-charge voltage level */
  U8X8_CA(0x0be, 0x007),                     /* VCOMH voltage */
  U8X8_CA(0x0b6, 0x004),		/* second precharge */
  U8X8_CA(0x0d5, 0x062),		/* enable second precharge, internal vsl (bit0 = 0) */
  
  U8X8_C(0x0a4),				/* normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_ssd1327_ws_96x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if ( u8x8_d_ssd1327_96x96_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
  {
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1327_winstar_96x64_display_info);
    return 1;
  }
  else if ( msg == U8X8_MSG_DISPLAY_INIT )
  {
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_winstar_96x64_init_seq);    
    return 1;
  }
  else if  ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
    if ( arg_int == 0 )
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_winstar_96x64_flip0_seq);
      u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_winstar_96x64_flip1_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }
    return 1;
  }
  return 0;
}


/*=============================================*/
/*  Seeedstudio Grove OLED 96x96 */

static const u8x8_display_info_t u8x8_ssd1327_96x96_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	
  /* post_reset_wait_ms = */ 100, 		/**/
  /* sda_setup_time_ns = */ 100,		/* */
  /* sck_pulse_width_ns = */ 100,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 1,	/* use 1 instead of 4, because the SSD1327 seems to be very slow */
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 60,	
  /* tile_width = */ 12,
  /* tile_hight = */ 12,
  /* default_x_offset = */ 16,
  /* flipmode_x_offset = */ 16,		
  /* pixel_width = */ 96,
  /* pixel_height = */ 96
};

/*  https://github.com/SeeedDocument/Grove_OLED_1.12/raw/master/resources/LY120-096096.pdf */
/*  http://www.seeedstudio.com/wiki/index.php?title=Twig_-_OLED_96x96 */
/* values from u8glib */
/*
  Re-map setting in Graphic Display Data RAM, command 0x0a0
    Bit 0: Column Address Re-map
    Bit 1: Nibble Re-map
    Bit 2: Horizontal/Vertical Address Increment
    Bit 3: Not used, must be 0
    
    Bit 4: COM Re-map
    Bit 5: Not used, must be 0
    Bit 6: COM Split Odd Even
    Bit 7: Not used, must be 0
*/


static const uint8_t u8x8_d_ssd1327_96x96_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_CA(0x0fd, 0x012),		/* unlock display, usually not required because the display is unlocked after reset */
  U8X8_C(0x0ae),		                /* display off */
  //U8X8_CA(0x0a8, 0x03f),		/* multiplex ratio: 0x03f * 1/64 duty */
  U8X8_CA(0x0a8, 0x05f),		/* multiplex ratio: 0x05f * 1/64 duty */
  U8X8_CA(0x0a1, 0x000),		/* display start line */
  //U8X8_CA(0x0a2, 0x04c),		/* display offset, shift mapping ram counter */
  
  U8X8_CA(0x0a2, 0x020),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x051),		/* remap configuration */
  
  
  U8X8_CA(0x0ab, 0x001),		/* Enable internal VDD regulator (RESET) */
  //U8X8_CA(0x081, 0x070),		/* contrast, brightness, 0..128 */
  U8X8_CA(0x081, 0x053),		/* contrast, brightness, 0..128 */
  //U8X8_CA(0x0b1, 0x055),                    /* phase length */
  U8X8_CA(0x0b1, 0x051),                    /* phase length */  
  //U8X8_CA(0x0b3, 0x091),		/* set display clock divide ratio/oscillator frequency (set clock as 135 frames/sec) */			
  U8X8_CA(0x0b3, 0x001),		/* set display clock divide ratio/oscillator frequency  */			
  
  //? U8X8_CA(0x0ad, 0x002),		/* master configuration: disable embedded DC-DC, enable internal VCOMH */
  //? U8X8_C(0x086),				/* full current range (0x084, 0x085, 0x086) */
  
  U8X8_C(0x0b9),				/* use linear lookup table */

  //U8X8_CA(0x0bc, 0x010),                    /* pre-charge voltage level */
  U8X8_CA(0x0bc, 0x008),                    /* pre-charge voltage level */
  //U8X8_CA(0x0be, 0x01c),                     /* VCOMH voltage */
  U8X8_CA(0x0be, 0x007),                     /* VCOMH voltage */
  U8X8_CA(0x0b6, 0x001),		/* second precharge */
  U8X8_CA(0x0d5, 0x062),		/* enable second precharge, internal vsl (bit0 = 0) */


  
  U8X8_C(0x0a4),				/* normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};




uint8_t u8x8_d_ssd1327_seeed_96x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if ( u8x8_d_ssd1327_96x96_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
  {
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1327_96x96_display_info);
    return 1;
  }
  else if ( msg == U8X8_MSG_DISPLAY_INIT )
  {
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_96x96_init_seq);    
    return 1;
  }
  else if  ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
    if ( arg_int == 0 )
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_seeed_96x96_flip0_seq);
      u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_seeed_96x96_flip1_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }
    return 1;
  }
  return 0;
}

/*=============================================*/
/*  EA W128128 round OLED 128x128 */
/* issue #641 */
/* https://www.lcd-module.de/fileadmin/eng/pdf/grafik/W128128-XR.pdf */

static const u8x8_display_info_t u8x8_ssd1327_ea_w128128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	
  /* post_reset_wait_ms = */ 100, 		/**/
  /* sda_setup_time_ns = */ 100,		/* */
  /* sck_pulse_width_ns = */ 100,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 1,	/* use 1 instead of 4, because the SSD1327 seems to be very slow */
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 60,	
  /* tile_width = */ 16,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,		
  /* pixel_width = */ 128,
  /* pixel_height = */ 128
};

/* this is a copy of the init sequence for the seeed 96x96 oled */
static const uint8_t u8x8_d_ssd1327_ea_w128128_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_CA(0x0fd, 0x012),		/* unlock display, usually not required because the display is unlocked after reset */
  U8X8_C(0x0ae),		                /* display off */
  //U8X8_CA(0x0a8, 0x03f),		/* multiplex ratio: 0x03f * 1/64 duty */
  U8X8_CA(0x0a8, 0x05f),		/* multiplex ratio: 0x05f * 1/64 duty */
  U8X8_CA(0x0a1, 0x000),		/* display start line */
  //U8X8_CA(0x0a2, 0x04c),		/* display offset, shift mapping ram counter */
  
  U8X8_CA(0x0a2, 0x010),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x051),		/* remap configuration */
  
  
  U8X8_CA(0x0ab, 0x001),		/* Enable internal VDD regulator (RESET) */
  //U8X8_CA(0x081, 0x070),		/* contrast, brightness, 0..128 */
  U8X8_CA(0x081, 0x053),		/* contrast, brightness, 0..128 */
  //U8X8_CA(0x0b1, 0x055),                    /* phase length */
  U8X8_CA(0x0b1, 0x051),                    /* phase length */  
  //U8X8_CA(0x0b3, 0x091),		/* set display clock divide ratio/oscillator frequency (set clock as 135 frames/sec) */			
  U8X8_CA(0x0b3, 0x001),		/* set display clock divide ratio/oscillator frequency  */			
  
  //? U8X8_CA(0x0ad, 0x002),		/* master configuration: disable embedded DC-DC, enable internal VCOMH */
  //? U8X8_C(0x086),				/* full current range (0x084, 0x085, 0x086) */
  
  U8X8_C(0x0b9),				/* use linear lookup table */

  //U8X8_CA(0x0bc, 0x010),                    /* pre-charge voltage level */
  U8X8_CA(0x0bc, 0x008),                    /* pre-charge voltage level */
  //U8X8_CA(0x0be, 0x01c),                     /* VCOMH voltage */
  U8X8_CA(0x0be, 0x007),                     /* VCOMH voltage */
  U8X8_CA(0x0b6, 0x001),		/* second precharge */
  U8X8_CA(0x0d5, 0x062),		/* enable second precharge, internal vsl (bit0 = 0) */
  
  U8X8_C(0x0a4),				/* normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};



static const uint8_t u8x8_d_ssd1327_ea_w128128_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0a2, 0x000),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x051),		/* remap configuration */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1327_ea_w128128_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0a2, 0x000),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x042),		/* remap configuration */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_ssd1327_ea_w128128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if ( u8x8_d_ssd1327_96x96_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
  {
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1327_ea_w128128_display_info);
    return 1;
  }
  else if ( msg == U8X8_MSG_DISPLAY_INIT )
  {
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_ea_w128128_init_seq);    
    return 1;
  }
  else if  ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
    if ( arg_int == 0 )
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_ea_w128128_flip0_seq);
      u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_ea_w128128_flip1_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }
    return 1;
  }
  return 0;
}

/*=============================================*/
/*  MIDAS MCOT128128C1V-YM 128x128 Module */


static const u8x8_display_info_t u8x8_ssd1327_128x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	
  /* post_reset_wait_ms = */ 100, 		/**/
  /* sda_setup_time_ns = */ 100,		/* */
  /* sck_pulse_width_ns = */ 100,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 1,	/* use 1 instead of 4, because the SSD1327 seems to be very slow, Update 9 Aug 2019: The OLED from aliexpress supports 400kHz */
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 60,	
  /* tile_width = */ 16,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,		
  /* pixel_width = */ 128,
  /* pixel_height = */ 128
};

/*  https://github.com/SeeedDocument/Grove_OLED_1.12/raw/master/resources/LY120-096096.pdf */
/*  http://www.seeedstudio.com/wiki/index.php?title=Twig_-_OLED_96x96 */
/* values from u8glib */
/*
  Re-map setting in Graphic Display Data RAM, command 0x0a0
    Bit 0: Column Address Re-map
    Bit 1: Nibble Re-map
    Bit 2: Horizontal/Vertical Address Increment
    Bit 3: Not used, must be 0
    
    Bit 4: COM Re-map
    Bit 5: Not used, must be 0
    Bit 6: COM Split Odd Even
    Bit 7: Not used, must be 0
*/


static const uint8_t u8x8_d_ssd1327_128x128_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

	
  U8X8_CA(0x0fd, 0x012),		/* unlock display, usually not required because the display is unlocked after reset */
  U8X8_C(0x0ae),		                /* display off */
  //U8X8_CA(0x0a8, 0x03f),		/* multiplex ratio: 0x03f * 1/64 duty */
  //U8X8_CA(0x0a8, 0x05f),		/* multiplex ratio: 0x05f * 1/64 duty */
  U8X8_CA(0x0a8, 0x07f),       		 /* multiplex ratio: 0x05f * 1/128duty */
  U8X8_CA(0x0a1, 0x000),		/* display start line */
  //U8X8_CA(0x0a2, 0x04c),		/* display offset, shift mapping ram counter */
  
  U8X8_CA(0x0a2, 0x000),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x051),		/* remap configuration */
  
  
  U8X8_CA(0x0ab, 0x001),		/* Enable internal VDD regulator (RESET) */
  //U8X8_CA(0x081, 0x070),		/* contrast, brightness, 0..128 */
  U8X8_CA(0x081, 0x053),		/* contrast, brightness, 0..128 */
  //U8X8_CA(0x0b1, 0x055),                    /* phase length */
  U8X8_CA(0x0b1, 0x051),                    /* phase length */  
  //U8X8_CA(0x0b3, 0x091),		/* set display clock divide ratio/oscillator frequency (set clock as 135 frames/sec) */			
  U8X8_CA(0x0b3, 0x001),		/* set display clock divide ratio/oscillator frequency  */			
  
  //? U8X8_CA(0x0ad, 0x002),		/* master configuration: disable embedded DC-DC, enable internal VCOMH */
  //? U8X8_C(0x086),				/* full current range (0x084, 0x085, 0x086) */
  
  U8X8_C(0x0b9),				/* use linear lookup table */

  //U8X8_CA(0x0bc, 0x010),                    /* pre-charge voltage level */
  U8X8_CA(0x0bc, 0x008),                    /* pre-charge voltage level */
  //U8X8_CA(0x0be, 0x01c),                     /* VCOMH voltage */
  U8X8_CA(0x0be, 0x007),                     /* VCOMH voltage */
  U8X8_CA(0x0b6, 0x001),		/* second precharge */
  U8X8_CA(0x0d5, 0x062),		/* enable second precharge, internal vsl (bit0 = 0) */


  
  U8X8_C(0x0a4),				/* normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const uint8_t u8x8_d_ssd1327_128x128_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0a2, 0x000),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x051),		/* remap configuration */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1327_128x128_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0a2, 0x000),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x042),		/* remap configuration */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1327_midas_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call the 96x96 procedure at the moment */
  if ( u8x8_d_ssd1327_96x96_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
  {
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1327_128x128_display_info);
    return 1;
  }
  else if ( msg == U8X8_MSG_DISPLAY_INIT )
  {
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_128x128_init_seq); 
    return 1;
  }
  else if  ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
    if ( arg_int == 0 )
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_128x128_flip0_seq);
      u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_128x128_flip1_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }
    return 1;
  }
  return 0;
}


/*=============================================*/
/*  
  Waveshare 128x128 Module 
  https://www.waveshare.com/w/upload/8/80/1.5inch_OLED_Module_User_Manual_EN.pdf
  https://github.com/olikraus/u8g2/issues/880

  This is mostly a takeover of the EA display.
*/

/*  https://github.com/SeeedDocument/Grove_OLED_1.12/raw/master/resources/LY120-096096.pdf */
/*  http://www.seeedstudio.com/wiki/index.php?title=Twig_-_OLED_96x96 */
/* values from u8glib */
/*
  Re-map setting in Graphic Display Data RAM, command 0x0a0
    Bit 0: Column Address Re-map
    Bit 1: Nibble Re-map
    Bit 2: Horizontal/Vertical Address Increment
    Bit 3: Not used, must be 0
    
    Bit 4: COM Re-map
    Bit 5: Not used, must be 0
    Bit 6: COM Split Odd Even
    Bit 7: Not used, must be 0
*/

/* takeover from https://github.com/olikraus/u8g2/issues/880 */
static const uint8_t u8x8_d_ssd1327_ws_128x128_init_seq[] = {
    
    U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

    U8X8_C(0x0ae), //--turn off oled panel
    U8X8_CAA(0x015, 0x000, 0x07f),    //set column address, start column 0, end column 127
    U8X8_CAA(0x075, 0x000, 0x07f),    //set row address, start row 0, end row 127
    U8X8_CA(0x081, 0x080),    //set contrast control
    U8X8_CA(0x0a0, 0x051),    //gment remap, 51
    U8X8_CA(0x0a1, 0x000),    //start line
    U8X8_CA(0x0a2, 0x000),    //display offset
    U8X8_CAA(0x0a4, 0x0a8, 0x07f),    //rmal display, set multiplex ratio
    U8X8_CA(0x0b1, 0x0f1),    //set phase leghth
    U8X8_CA(0x0b3, 0x000),    //set dclk, 80Hz:0xc1 90Hz:0xe1   100Hz:0x00   110Hz:0x30 120Hz:0x50   130Hz:0x70     01
    U8X8_CA(0x0ab, 0x001),    //
    U8X8_CA(0x0b6, 0x00f),    //set phase leghth
    U8X8_CA(0x0be, 0x00f),
    U8X8_CA(0x0bc, 0x008),
    U8X8_CA(0x0d5, 0x062),
    U8X8_CA(0x0fd, 0x012),

    U8X8_END_TRANSFER(),             	/* disable chip */
    U8X8_END()             			/* end of sequence */
  };


uint8_t u8x8_d_ssd1327_ws_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call the 96x96 procedure at the moment */
  if ( u8x8_d_ssd1327_96x96_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
  {
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1327_ea_w128128_display_info);
    return 1;
  }
  else if ( msg == U8X8_MSG_DISPLAY_INIT )
  {
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_ws_128x128_init_seq); 
    return 1;
  }
  else if  ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
    if ( arg_int == 0 )
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_ea_w128128_flip0_seq);
      u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_ea_w128128_flip1_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }
    return 1;
  }
  return 0;
}




/*=============================================*/
/*  
Visonox VGM128096A4W10 128x96 COB 
https://github.com/olikraus/u8g2/files/4052919/M02289_VGM128096A4W10_Y02.pdf
https://github.com/olikraus/u8g2/issues/1090
*/


static const u8x8_display_info_t u8x8_ssd1327_128x96_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	
  /* post_reset_wait_ms = */ 100, 		/**/
  /* sda_setup_time_ns = */ 100,		/* */
  /* sck_pulse_width_ns = */ 100,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 1,	/* use 1 instead of 4, because the SSD1327 seems to be very slow, Update 9 Aug 2019: The OLED from aliexpress supports 400kHz */
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 60,	
  /* tile_width = */ 16,
  /* tile_hight = */ 12,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,		
  /* pixel_width = */ 128,
  /* pixel_height = */ 96
};

/*  https://github.com/SeeedDocument/Grove_OLED_1.12/raw/master/resources/LY120-096096.pdf */
/*  http://www.seeedstudio.com/wiki/index.php?title=Twig_-_OLED_96x96 */
/* values from u8glib */
/*
  Re-map setting in Graphic Display Data RAM, command 0x0a0
    Bit 0: Column Address Re-map
    Bit 1: Nibble Re-map
    Bit 2: Horizontal/Vertical Address Increment
    Bit 3: Not used, must be 0
    
    Bit 4: COM Re-map
    Bit 5: Not used, must be 0
    Bit 6: COM Split Odd Even
    Bit 7: Not used, must be 0
*/

/* init values from the Visionox datasheeet section 10.4 */

static const uint8_t u8x8_d_ssd1327_128x96_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

	
  U8X8_CA(0x0fd, 0x012),		/* unlock display, usually not required because the display is unlocked after reset */
  U8X8_C(0x0ae),		                /* display off */
  //U8X8_CA(0x0a8, 0x03f),		/* multiplex ratio: 0x03f * 1/64 duty */
  U8X8_CA(0x0a8, 0x05f),		/* multiplex ratio: 0x05f * 1/64 duty */
  //U8X8_CA(0x0a8, 0x07f),       		 /* multiplex ratio: 0x05f * 1/128duty */
  U8X8_CA(0x0a1, 0x000),		/* display start line */
  //U8X8_CA(0x0a2, 0x04c),		/* display offset, shift mapping ram counter */
  
  U8X8_CA(0x0a2, 0x020),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x051),		/* remap configuration */
  
  
  U8X8_CA(0x0ab, 0x001),		/* Enable internal VDD regulator (RESET) */
  //U8X8_CA(0x081, 0x070),		/* contrast, brightness, 0..128 */
  U8X8_CA(0x081, 0x0df),		/* contrast, brightness, 0..128 (0xdf as per datasheet) */
  U8X8_CA(0x0b1, 0x022),                    /* phase length */  
  U8X8_CA(0x0b3, 0x050),		/* set display clock divide ratio/oscillator frequency  */			
  
  //? U8X8_CA(0x0ad, 0x002),		/* master configuration: disable embedded DC-DC, enable internal VCOMH */
  //? U8X8_C(0x086),				/* full current range (0x084, 0x085, 0x086) */
  
  U8X8_C(0x0b9),				/* use linear lookup table */

  U8X8_CA(0x0bc, 0x010),                    /* pre-charge voltage level */
  U8X8_CA(0x0be, 0x005),                     /* VCOMH voltage */
  U8X8_CA(0x0b6, 0x00a),		/* second precharge */
  U8X8_CA(0x0d5, 0x062),		/* enable second precharge, internal vsl (bit0 = 0) */


  
  U8X8_C(0x0a4),				/* normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const uint8_t u8x8_d_ssd1327_128x96_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0a2, 0x020),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x051),		/* remap configuration */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1327_128x96_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0a2, 0x060),		/* display offset, shift mapping ram counter */
  U8X8_CA(0x0a0, 0x042),		/* remap configuration */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1327_visionox_128x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call the 96x96 procedure at the moment */
  if ( u8x8_d_ssd1327_96x96_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
  {
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1327_128x96_display_info);
    return 1;
  }
  else if ( msg == U8X8_MSG_DISPLAY_INIT )
  {
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_128x96_init_seq); 
    return 1;
  }
  else if  ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
    if ( arg_int == 0 )
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_128x96_flip0_seq);
      u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1327_128x96_flip1_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }
    return 1;
  }
  return 0;
}

