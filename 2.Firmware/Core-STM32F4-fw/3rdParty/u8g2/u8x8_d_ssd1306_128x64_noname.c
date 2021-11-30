/*

  u8x8_d_ssd1306_128x64_noname.c

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



/* more or less generic setup of all these small OLEDs */
static const uint8_t u8x8_d_ssd1306_128x64_noname_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_CA(0x0d5, 0x080),		/* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
  U8X8_CA(0x0a8, 0x03f),		/* multiplex ratio */
  U8X8_CA(0x0d3, 0x000),		/* display offset */
  U8X8_C(0x040),		                /* set display start line to 0 */
  U8X8_CA(0x08d, 0x014),		/* [2] charge pump setting (p62): 0x014 enable, 0x010 disable, SSD1306 only, should be removed for SH1106 */
  U8X8_CA(0x020, 0x000),		/* horizontal addressing mode */
  
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  // Flipmode
  // U8X8_C(0x0a0),				/* segment remap a0/a1*/
  // U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  
  U8X8_CA(0x0da, 0x012),		/* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */

  U8X8_CA(0x081, 0x0cf), 		/* [2] set contrast control */
  U8X8_CA(0x0d9, 0x0f1), 		/* [2] pre-charge period 0x022/f1*/
  U8X8_CA(0x0db, 0x040), 		/* vcomh deselect level */  
  // if vcomh is 0, then this will give the biggest range for contrast control issue #98
  // restored the old values for the noname constructor, because vcomh=0 will not work for all OLEDs, #116
  
  U8X8_C(0x02e),				/* Deactivate scroll */ 
  U8X8_C(0x0a4),				/* output ram to display */
  U8X8_C(0x0a6),				/* none inverted normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

/* this setup maximizes the brightness range, that can be set with setContrast() */
/* Drawback: VCOMH deselect level is set to 0, which das not work so good with all OLEDs, issue #116 */
static const uint8_t u8x8_d_ssd1306_128x64_vcomh0_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_CA(0x0d5, 0x080),		/* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
  U8X8_CA(0x0a8, 0x03f),		/* multiplex ratio */
  U8X8_CA(0x0d3, 0x000),		/* display offset */
  U8X8_C(0x040),		                /* set display start line to 0 */
  U8X8_CA(0x08d, 0x014),		/* [2] charge pump setting (p62): 0x014 enable, 0x010 disable */
  U8X8_CA(0x020, 0x000),		/* horizontal addressing mode */
  
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  // Flipmode
  // U8X8_C(0x0a0),				/* segment remap a0/a1*/
  // U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  
  U8X8_CA(0x0da, 0x012),		/* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
  U8X8_CA(0x081, 0x0ef),		/* [2] set contrast control,  */
  U8X8_CA(0x0d9, 0x0a1),		/* [2] pre-charge period 0x022/f1*/
  U8X8_CA(0x0db, 0x000),		/* vcomh deselect level 0x000 .. 0x070, low nibble always 0 */
  // if vcomh is 0, then this will give the biggest range for contrast control issue #98
  
  U8X8_C(0x02e),				/* Deactivate scroll */ 
  U8X8_C(0x0a4),				/* output ram to display */
  U8X8_C(0x0a6),				/* none inverted normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


/* same as u8x8_d_ssd1306_128x64_noname_init_seq, but 0x0da bit 4 is set to 0 */
/* this will disable the alternative COM configuration */
static const uint8_t u8x8_d_ssd1306_128x64_alt0_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_CA(0x0d5, 0x080),		/* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
  U8X8_CA(0x0a8, 0x03f),		/* multiplex ratio */
  U8X8_CA(0x0d3, 0x000),		/* display offset */
  U8X8_C(0x040),		                /* set display start line to 0 */
  U8X8_CA(0x08d, 0x014),		/* [2] charge pump setting (p62): 0x014 enable, 0x010 disable, SSD1306 only, should be removed for SH1106 */
  U8X8_CA(0x020, 0x000),		/* horizontal addressing mode */
  
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  // Flipmode
  // U8X8_C(0x0a0),				/* segment remap a0/a1*/
  // U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  
  U8X8_CA(0x0da, 0x002),		/* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */

  U8X8_CA(0x081, 0x0cf), 		/* [2] set contrast control */
  U8X8_CA(0x0d9, 0x0f1), 		/* [2] pre-charge period 0x022/f1*/
  U8X8_CA(0x0db, 0x040), 		/* vcomh deselect level */  
  // if vcomh is 0, then this will give the biggest range for contrast control issue #98
  // restored the old values for the noname constructor, because vcomh=0 will not work for all OLEDs, #116
  
  U8X8_C(0x02e),				/* Deactivate scroll */ 
  U8X8_C(0x0a4),				/* output ram to display */
  U8X8_C(0x0a6),				/* none inverted normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};



/* issue 316: a special sh1106 setup, https://www.mikrocontroller.net/topic/431371?goto=5087807#5087807 */
static const uint8_t u8x8_d_sh1106_128x64_winstar_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0xae),                 // Display OFF/ON: off (POR = 0xae)
  U8X8_C(0xa4),                 // Set Entire Display OFF/ON: off (POR = 0xa4)
  U8X8_CA(0xd5, 0x50),       // Divide Ratio/Oscillator FrequencyData Set: divide ratio = 1 (POR = 1), Oscillator Frequency = +/- 0% (POR = +/- 0%)
  U8X8_CA(0xa8, 0x3f),       // Multiplex Ratio Data Set: 64 (POR = 0x3f, 64)
  U8X8_CA(0xd3, 0x00),       // Display OffsetData Set: 0 (POR = 0x00)
  U8X8_C(0x40),                 // Set Display Start Line: 0  
  U8X8_CA(0xad, 0x8b),       // DC-DC ON/OFF Mode Set: Built-in DC-DC is used, Normal Display (POR = 0x8b)
  U8X8_CA(0xd9, 0x22),       // Dis-charge/Pre-charge PeriodData Set: pre-charge 2 DCLKs, dis-charge 2 DCLKs (POR = 0x22, pre-charge 2 DCLKs, dis-charge 2 DCLKs)
  U8X8_CA(0xdb, 0x35),       // VCOM Deselect LevelData Set: 0,770V (POR = 0x35, 0,770 V)
  U8X8_C(0x32), // Set Pump voltage value: 8,0 V (POR = 0x32, 8,0 V)
  U8X8_CA(0x81, 0xff),       // Contrast Data Register Set: 255 (large) (POR = 0x80)
  U8X8_C(0x0a6),			// Set Normal/Reverse Display: normal (POR = 0xa6)
  U8X8_CA(0x0da, 0x012),		// com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) 
      
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const uint8_t u8x8_d_ssd1306_128x64_noname_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x64_noname_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x64_noname_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x64_noname_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),				/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static uint8_t u8x8_d_ssd1306_sh1106_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_128x64_noname_display_info);
      break;
    */
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_noname_init_seq);    
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_noname_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_noname_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_noname_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_noname_flip1_seq);
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
    
      u8x8_cad_SendCmd(u8x8, 0x040 );	/* set line offset to 0 */
    
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      u8x8_cad_SendArg(u8x8, 0x000 | ((x&15)));					/* probably wrong, should be SendCmd */
      u8x8_cad_SendArg(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));	/* probably wrong, should be SendCmd */

    
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
	u8x8_cad_SendData(u8x8, c*8, ptr); 	/* note: SendData can not handle more than 255 bytes */
	/*
	do
	{
	  u8x8_cad_SendData(u8x8, 8, ptr);
	  ptr += 8;
	  c--;
	} while( c > 0 );
	*/
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


static const u8x8_display_info_t u8x8_ssd1306_128x64_noname_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1306: 3 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1306: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1306: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* SSD1306: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 16,
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};

uint8_t u8x8_d_ssd1306_128x64_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
  if ( u8x8_d_ssd1306_sh1106_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_noname_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_128x64_noname_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}

uint8_t u8x8_d_ssd1306_128x64_vcomh0(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
  if ( u8x8_d_ssd1306_sh1106_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_vcomh0_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_128x64_noname_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}

uint8_t u8x8_d_ssd1306_128x64_alt0(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  
  if ( u8x8_d_ssd1306_sh1106_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_alt0_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_128x64_noname_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}


static const u8x8_display_info_t u8x8_sh1106_128x64_noname_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1306: 3 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1306: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1306: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215) */
  /* spi_mode = */ 3,		/* active low (clock is high by default), rising edge, this seems to be a difference to the ssd1306 */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* SSD1306: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 16,
  /* tile_hight = */ 8,
  /* default_x_offset = */ 2,
  /* flipmode_x_offset = */ 2,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};

uint8_t u8x8_d_sh1106_128x64_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if ( u8x8_d_ssd1306_sh1106_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      /* maybe use a better init sequence */
      /* https://www.mikrocontroller.net/topic/431371 */
      /* the new sequence is added in the winstar constructor (see below), this is kept untouched */
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_noname_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_sh1106_128x64_noname_display_info);
      break;
    default:
      return 0;
  }
  return 1;
    
}

uint8_t u8x8_d_sh1106_128x64_vcomh0(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if ( u8x8_d_ssd1306_sh1106_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x64_vcomh0_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_sh1106_128x64_noname_display_info);
      break;
    default:
      return 0;
  }
  return 1;
    
}

uint8_t u8x8_d_sh1106_128x64_winstar(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if ( u8x8_d_ssd1306_sh1106_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1106_128x64_winstar_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_sh1106_128x64_noname_display_info);
      break;
    default:
      return 0;
  }
  return 1;
    
}

