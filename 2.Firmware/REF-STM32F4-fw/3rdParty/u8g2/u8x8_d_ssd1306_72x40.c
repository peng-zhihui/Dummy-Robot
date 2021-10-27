/*

  u8x8_d_ssd1306_72x40.c

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
  
*/


#include "u8x8.h"



/* 
EastRising 0.41 OLED 
https://www.buydisplay.com/default/white-0-42-inch-oled-display-panel-72x40-iic-i2c-serial-spi-ssd1306


    command(0xae);//--turn off oled panel
	
    command(0xd5);//--set display clock divide ratio/oscillator frequency
    command(0x80);//--set divide ratio

    command(0xa8);//--set multiplex ratio
    command(0x27);//--1/40 duty

    command(0xd3);//-set display offset
    command(0x00);//-not offset

    command(0xad);//--Internal IREF Setting	
    command(0x30);//--

    command(0x8d);//--set Charge Pump enable/disable
    command(0x14);//--set(0x10) disable

    command(0x40);//--set start line address

    command(0xa6);//--set normal display

    command(0xa4);//Disable Entire Display On

    command(0xa1);//--set segment re-map 128 to 0

    command(0xC8);//--Set COM Output Scan Direction 64 to 0

    command(0xda);//--set com pins hardware configuration
    command(0x12);

    command(0x81);//--set contrast control register
    command(0xaf);

    command(0xd9);//--set pre-charge period
    command(0x22);

    command(0xdb);//--set vcomh
    command(0x20);

    command(0xaf);//--turn on oled panel


*/
static const uint8_t u8x8_d_ssd1306_72x40_er_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_CA(0x0d5, 0x080),		/* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
  U8X8_CA(0x0a8, 0x027),		/* multiplex ratio, 0.42 OLED: 0x27*/
  U8X8_CA(0x0d3, 0x000),		/* display offset, 0.42 OLED  */
  U8X8_CA(0x0ad, 0x030),		/* Internal IREF Setting for the 0.42 OLED, see also issue https://github.com/olikraus/u8g2/issues/1047 */
  U8X8_CA(0x08d, 0x014),		/* [2] charge pump setting (p62): 0x014 enable, 0x010 disable, 0.66 OLED  0x14*/

  U8X8_C(0x040),		                /* set display start line to 0, 0.66 OLED */
  U8X8_C(0x0a6),				/* none inverted normal display mode */
  U8X8_C(0x0a4),				/* output ram to display */
  
  U8X8_CA(0x020, 0x000),		/* horizontal addressing mode */
  
  U8X8_C(0x0a1),				/* segment remap a0/a1, 0.66 OLED  */
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse, 0.66 OLED  */
  // Flipmode
  // U8X8_C(0x0a0),				/* segment remap a0/a1*/
  // U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  
  U8X8_CA(0x0da, 0x012),		/* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5), 0.66 OLED */
  U8X8_CA(0x081, 0x0af),		/* [2] set contrast control, 0.42 OLED */
  U8X8_CA(0x0d9, 0x022),		/* [2] pre-charge period 0x022/f1, 0.42 OLED datasheet: 0x22 */
  U8X8_CA(0x0db, 0x020),		/* vcomh deselect level, 0.42 OLED datasheet: 0x20 */
  
  U8X8_C(0x02e),				/* Deactivate scroll */ 
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_72x40_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_72x40_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_72x40_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_72x40_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),				/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static uint8_t u8x8_d_ssd1306_72x40_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_72x40_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_72x40_er_init_seq);    
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_72x40_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_72x40_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_72x40_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_72x40_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* ssd1306 has range from 0 to 255 */
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


static const u8x8_display_info_t u8x8_ssd1306_72x40_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1306: 3 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1306: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1306: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* SSD1306: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 9,
  /* tile_hight = */ 5,
  /* default_x_offset = */ 28,
  /* flipmode_x_offset = */ 28,
  /* pixel_width = */ 72,
  /* pixel_height = */ 40
};

/* 
EastRising 0.41 OLED 
https://www.buydisplay.com/default/white-0-42-inch-oled-display-panel-72x40-iic-i2c-serial-spi-ssd1306
*/
uint8_t u8x8_d_ssd1306_72x40_er(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
    {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_72x40_display_info);
      return 1;
    }
    else if ( msg == U8X8_MSG_DISPLAY_INIT )
    {
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_72x40_er_init_seq);    
      return 1;
    }
    return u8x8_d_ssd1306_72x40_generic(u8x8, msg, arg_int, arg_ptr);
}


